#include "command.h"

#include "kali/device/device.h"
#include "kali/device/native_handle_traits.h"
#include "kali/device/helpers.h"
#include "kali/device/shader.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/reflection.h"
#include "kali/device/pipeline.h"
#include "kali/device/query.h"

#include "kali/core/config.h"
#include "kali/core/error.h"
#include "kali/core/maths.h"
#include "kali/core/type_utils.h"
#include "kali/core/string.h"
#include "kali/core/short_vector.h"

#include "kali/math/vector_math.h"

namespace kali {

// ----------------------------------------------------------------------------
// CommandQueue
// ----------------------------------------------------------------------------

CommandQueue::CommandQueue(ref<Device> device, CommandQueueDesc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    gfx::ICommandQueue::Desc gfx_desc{
        .type = static_cast<gfx::ICommandQueue::QueueType>(m_desc.type),
    };

    SLANG_CALL(m_device->gfx_device()->createCommandQueue(gfx_desc, m_gfx_command_queue.writeRef()));
}

NativeHandle CommandQueue::get_native_handle() const
{
    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_command_queue->getNativeHandle(&handle));
#if KALI_HAS_D3D12
    if (m_device->type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12CommandQueue*>(handle.handleValue));
#endif
#if KALI_HAS_VULKAN
    if (m_device->type() == DeviceType::vulkan)
        return NativeHandle(reinterpret_cast<VkQueue>(handle.handleValue));
#endif
    return {};
}

std::string CommandQueue::to_string() const
{
    return fmt::format(
        "CommandQueue("
        "  device={}\n",
        "  type={}\n",
        ")",
        m_device,
        m_desc.type
    );
}

// ----------------------------------------------------------------------------
// CommandStream
// ----------------------------------------------------------------------------

CommandStream::CommandStream(ref<Device> device, CommandStreamDesc desc)
    : m_device(std::move(device))
    , m_desc(std::move(desc))
{
    m_command_queue = m_device->create_command_queue({.type = m_desc.queue_type});
    m_command_queue->break_strong_reference_to_device();

    // Create per-frame data.
    m_frame_data.resize(m_desc.frame_count);
    for (auto& frame_data : m_frame_data) {
        m_device->gfx_device()->createTransientResourceHeap(
            gfx::ITransientResourceHeap::Desc{
                .flags = gfx::ITransientResourceHeap::Flags::AllowResizing,
                .constantBufferSize = 1024 * 1024 * 4,
                .samplerDescriptorCount = 1024,
                .uavDescriptorCount = 1024,
                .srvDescriptorCount = 1024,
                .constantBufferDescriptorCount = 1024,
                .accelerationStructureDescriptorCount = 1024,
            },
            frame_data.transient_resource_heap.writeRef()
        );
    }
}

void CommandStream::submit()
{
    if (m_command_buffer) {
        if (m_encoder) {
            m_encoder->end();
            m_encoder = nullptr;
        }
        m_command_buffer->gfx_command_buffer()->close();
        m_command_queue->gfx_command_queue()->executeCommandBuffer(m_command_buffer->gfx_command_buffer());
        m_command_buffer = nullptr;
    }
}

// ----------------------------------------------------------------------------
// Synchronization
// ----------------------------------------------------------------------------

uint64_t CommandStream::signal(Fence* fence, uint64_t value)
{
    KALI_CHECK_NOT_NULL(fence);
    uint64_t signal_value = fence->update_signaled_value(value);
    m_command_queue->gfx_command_queue()->executeCommandBuffers(0, nullptr, fence->gfx_fence(), signal_value);
    return signal_value;
}

void CommandStream::wait(const Fence* fence, uint64_t value)
{
    KALI_CHECK_NOT_NULL(fence);
    uint64_t wait_value = value == Fence::AUTO ? fence->signaled_value() : value;
    gfx::IFence* fences[] = {fence->gfx_fence()};
    uint64_t wait_values[] = {wait_value};
    SLANG_CALL(m_command_queue->gfx_command_queue()->waitForFenceValuesOnDevice(1, fences, wait_values));
}

// ----------------------------------------------------------------------------
// Barriers
// ----------------------------------------------------------------------------

bool CommandStream::buffer_barrier(const Buffer* buffer, ResourceState new_state)
{
    KALI_CHECK_NOT_NULL(buffer);

    ResourceState current_state = buffer->state_tracker().global_state();
    if (new_state == current_state)
        return false;

    get_resource_command_encoder()->buffer_barrier(buffer, current_state, new_state);
    buffer->state_tracker().set_global_state(new_state);
    return true;
}

bool CommandStream::texture_barrier(const Texture* texture, ResourceState new_state)
{
    KALI_CHECK_NOT_NULL(texture);

    ResourceState current_state = texture->state_tracker().global_state();
    if (new_state == current_state)
        return false;

    get_resource_command_encoder()->texture_barrier(texture, current_state, new_state);
    texture->state_tracker().set_global_state(new_state);
    return true;
}

bool CommandStream::texture_subresource_barrier(
    const Texture* texture,
    uint32_t array_slice,
    uint32_t mip_level,
    ResourceState new_state
)
{
    KALI_CHECK_NOT_NULL(texture);
    KALI_CHECK_LT(array_slice, texture->array_size());
    KALI_CHECK_LT(mip_level, texture->mip_count());

    uint32_t subresource_index = texture->get_subresource_index(mip_level, array_slice);
    ResourceState current_state = texture->state_tracker().subresource_state(subresource_index);
    if (new_state == current_state)
        return false;

    get_resource_command_encoder()->texture_subresource_barrier(
        texture,
        SubresourceRange{
            .texture_aspect = TextureAspect::color,
            .mip_level = mip_level,
            .mip_count = 1,
            .base_array_layer = array_slice,
            .layer_count = 1,
        },
        current_state,
        new_state
    );
    texture->state_tracker().set_subresource_state(subresource_index, new_state);
    return true;
}

bool CommandStream::resource_barrier(const Resource* resource, ResourceState new_state)
{
    KALI_CHECK_NOT_NULL(resource);

    if (resource->type() == ResourceType::buffer) {
        return buffer_barrier(static_cast<const Buffer*>(resource), new_state);
    } else {
        return texture_barrier(static_cast<const Texture*>(resource), new_state);
    }
}

void CommandStream::uav_barrier(const Resource* resource)
{
    KALI_CHECK_NOT_NULL(resource);

    if (resource->type() == ResourceType::buffer) {
        get_resource_command_encoder()->buffer_barrier(
            static_cast<const Buffer*>(resource),
            ResourceState::unordered_access,
            ResourceState::unordered_access
        );
    } else {
        get_resource_command_encoder()->texture_barrier(
            static_cast<const Texture*>(resource),
            ResourceState::unordered_access,
            ResourceState::unordered_access
        );
    }
}

// ----------------------------------------------------------------------------
// Resource copying
// ----------------------------------------------------------------------------

#if 0
void CommandStream::copy_resource(Resource* dst, Resource* src)
{
    // Copy from texture to texture or from buffer to buffer.
    KALI_ASSERT(dst->get_type() == src->get_type());

    resource_barrier(dst, ResourceState::copy_destination);
    resource_barrier(src, ResourceState::copy_source);

    auto encoder = get_resource_command_encoder();

    if (dst->get_type() == ResourceType::buffer) {
        const Buffer* dst_buffer = static_cast<const Buffer*>(dst);
        const Buffer* src_buffer = static_cast<const Buffer*>(src);

        KALI_ASSERT(src_buffer->get_size() <= dst_buffer->get_size());

        encoder->copyBuffer(
            dst_buffer->gfx_buffer_resource(),
            dst_buffer->get_gpu_address_offset(),
            src_buffer->gfx_buffer_resource(),
            src_buffer->get_gpu_address_offset(),
            src_buffer->get_size()
        );
    } else {
        const Texture* dst_texture = static_cast<const Texture*>(dst);
        const Texture* src_texture = static_cast<const Texture*>(src);
        gfx::SubresourceRange sr_range = {};

        encoder->copyTexture(
            dst_texture->gfx_texture_resource(),
            gfx::ResourceState::CopyDestination,
            sr_range,
            gfx::ITextureResource::Offset3D(0, 0, 0),
            src_texture->gfx_texture_resource(),
            gfx::ResourceState::CopySource,
            sr_range,
            gfx::ITextureResource::Offset3D(0, 0, 0),
            gfx::ITextureResource::Extents{0, 0, 0}
        );
    }

    mark_pending();
}
#endif

void CommandStream::copy_buffer_region(
    Buffer* dst,
    DeviceOffset dst_offset,
    const Buffer* src,
    DeviceOffset src_offset,
    DeviceSize size
)
{
    KALI_CHECK_NOT_NULL(dst);
    KALI_CHECK_NOT_NULL(src);
    // TODO(@skallweit): check for overlapping copies within same buffer?
    KALI_ASSERT_LE(dst_offset + size, dst->size());
    KALI_ASSERT_LE(src_offset + size, src->size());

    buffer_barrier(dst, ResourceState::copy_destination);
    buffer_barrier(src, ResourceState::copy_source);

    get_resource_command_encoder()->copy_buffer(dst, dst_offset, src, src_offset, size);
}

#if 0
void CommandStream::copy_texture_region(
    Texture* dst,
    uint32_t dst_sub_index,
    const Texture* src,
    uint32_t src_sub_index,
    uint3 dstOffset,
    uint3 srcOffset,
    uint3 size
)
{
    texture_barrier(dst, ResourceState::copy_destination);
    texture_barrier(src, ResourceState::copy_source);

    gfx::SubresourceRange dstSubresource = {};
    dstSubresource.baseArrayLayer = dst->getSubresourceArraySlice(dstSubresourceIdx);
    dstSubresource.layerCount = 1;
    dstSubresource.mipLevel = dst->getSubresourceMipLevel(dstSubresourceIdx);
    dstSubresource.mipLevelCount = 1;

    gfx::SubresourceRange srcSubresource = {};
    srcSubresource.baseArrayLayer = src->getSubresourceArraySlice(srcSubresourceIdx);
    srcSubresource.layerCount = 1;
    srcSubresource.mipLevel = src->getSubresourceMipLevel(srcSubresourceIdx);
    srcSubresource.mipLevelCount = 1;

    gfx::ITextureResource::Extents copySize = {(int)size.x, (int)size.y, (int)size.z};

    if (size.x == glm::uint(-1)) {
        copySize.width = src->getWidth(srcSubresource.mipLevel) - srcOffset.x;
        copySize.height = src->getHeight(srcSubresource.mipLevel) - srcOffset.y;
        copySize.depth = src->getDepth(srcSubresource.mipLevel) - srcOffset.z;
    }

    auto resourceEncoder = getLowLevelData()->getResourceCommandEncoder();
    resourceEncoder->copyTexture(
        dst->getGfxTextureResource(),
        gfx::ResourceState::CopyDestination,
        dstSubresource,
        gfx::ITextureResource::Offset3D(dstOffset.x, dstOffset.y, dstOffset.z),
        src->getGfxTextureResource(),
        gfx::ResourceState::CopySource,
        srcSubresource,
        gfx::ITextureResource::Offset3D(srcOffset.x, srcOffset.y, srcOffset.z),
        copySize
    );
    mCommandsPending = true;
}
#endif

void CommandStream::upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data)
{
    KALI_CHECK_NOT_NULL(buffer);
    KALI_CHECK_NOT_NULL(data);
    KALI_CHECK(offset + size <= buffer->size(), "data does not fit into the buffer");

    buffer_barrier(buffer, ResourceState::copy_destination);

    get_resource_command_encoder()->upload_buffer_data(buffer, offset, size, data);
}

void CommandStream::upload_texture_data(Texture* texture, const void* data)
{
    KALI_CHECK_NOT_NULL(texture);
    KALI_CHECK_NOT_NULL(data);

    upload_texture_subresource_data(texture, 0, texture->subresource_count(), data);
}

void CommandStream::upload_texture_subresource_data(
    Texture* texture,
    uint32_t subresource_index,
    uint32_t subresource_count,
    const void* data,
    uint3 offset,
    uint3 size
)
{
    KALI_CHECK_NOT_NULL(texture);
    KALI_CHECK_NOT_NULL(data);
    KALI_ASSERT_LT(subresource_index, texture->subresource_count());
    KALI_ASSERT_LE(subresource_index + subresource_count, texture->subresource_count());
    KALI_ASSERT_GE(subresource_count, 1);

    bool copy_region = (any(offset != uint3{0})) || any((size != uint3{0xffffffff}));
    KALI_ASSERT(subresource_count == 1 || (copy_region == false));

    texture_barrier(texture, ResourceState::copy_destination);

    const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(data);
    auto encoder = get_resource_command_encoder();
    gfx::ITextureResource::Offset3D gfx_offset = {
        narrow_cast<gfx::GfxIndex>(offset.x),
        narrow_cast<gfx::GfxIndex>(offset.y),
        narrow_cast<gfx::GfxIndex>(offset.z),
    };
    gfx::ITextureResource::Extents gfx_size = {
        narrow_cast<gfx::GfxCount>(size.x),
        narrow_cast<gfx::GfxCount>(size.y),
        narrow_cast<gfx::GfxCount>(size.z),
    };
    gfx::FormatInfo format_info = {};
    SLANG_CALL(gfx::gfxGetFormatInfo(static_cast<gfx::Format>(texture->format()), &format_info));
    for (uint32_t i = subresource_index; i < subresource_index + subresource_count; i++) {
        gfx::SubresourceRange sr_range = {};
        // sr_range.baseArrayLayer = narrow_cast<gfx::GfxIndex>(pTexture->getSubresourceArraySlice(i));
        // sr_range.mipLevel = narrow_cast<gfx::GfxIndex>(pTexture->getSubresourceMipLevel(i));
        sr_range.layerCount = 1;
        sr_range.mipLevelCount = 1;
        if (!copy_region) {
            gfx_size.width = align_to(
                format_info.blockWidth,
                narrow_cast<gfx::GfxCount>(texture->get_mip_width(sr_range.mipLevel))
            );
            gfx_size.height = align_to(
                format_info.blockHeight,
                narrow_cast<gfx::GfxCount>(texture->get_mip_height(sr_range.mipLevel))
            );
            gfx_size.depth = narrow_cast<gfx::GfxCount>(texture->get_mip_depth(sr_range.mipLevel));
        }
        gfx::ITextureResource::SubresourceData sr_data = {};
        sr_data.data = data_ptr;
        sr_data.strideY = narrow_cast<int64_t>(gfx_size.width) / format_info.blockWidth * format_info.blockSizeInBytes;
        sr_data.strideZ = sr_data.strideY * (gfx_size.height / format_info.blockHeight);
        data_ptr += sr_data.strideZ * gfx_size.depth;
        // encoder->uploadTextureData(texture->gfx_texture_resource(), sr_range, gfx_offset, gfx_size, &sr_data, 1);
    }
}

// ----------------------------------------------------------------------------
// Compute
// ----------------------------------------------------------------------------

ref<TransientShaderObject> CommandStream::bind_compute_pipeline(const ComputePipelineState* pipeline)
{
    // TODO pass object for setting barriers
    ref<TransientShaderObject> shader_object = get_compute_command_encoder()->bind_pipeline(pipeline);
    shader_object->m_stream = this;
    return shader_object;
}

void CommandStream::bind_compute_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object)
{
    get_compute_command_encoder()->bind_pipeline(pipeline, shader_object);
}

void CommandStream::dispatch_compute(uint3 thread_group_count)
{
    get_compute_command_encoder()->dispatch_compute(thread_group_count);
}

void CommandStream::dispatch_compute_indirect(const Buffer* cmd_buffer, DeviceOffset offset)
{
    get_compute_command_encoder()->dispatch_compute_indirect(cmd_buffer, offset);
}

// ----------------------------------------------------------------------------
// Debug events
// ----------------------------------------------------------------------------

void CommandStream::begin_debug_event(const char* name, float3 color)
{
    get_resource_command_encoder()->begin_debug_event(name, color);
}

void CommandStream::end_debug_event()
{
    get_resource_command_encoder()->end_debug_event();
}

void CommandStream::break_strong_reference_to_device()
{
    m_device.break_strong_reference();
}

ResourceCommandEncoder* CommandStream::get_resource_command_encoder()
{
    return reinterpret_cast<ResourceCommandEncoder*>(request_encoder(EncoderType::resource));
}

ComputeCommandEncoder* CommandStream::get_compute_command_encoder()
{
    return reinterpret_cast<ComputeCommandEncoder*>(request_encoder(EncoderType::compute));
}

RenderCommandEncoder* CommandStream::get_render_command_encoder()
{
    return reinterpret_cast<RenderCommandEncoder*>(request_encoder(EncoderType::render));
}

RayTracingCommandEncoder* CommandStream::get_raytracing_command_encoder()
{
    return reinterpret_cast<RayTracingCommandEncoder*>(request_encoder(EncoderType::raytracing));
}

CommandEncoder* CommandStream::request_encoder(EncoderType type)
{
    // Return open encoder if type matches (all encoders inherit the resource encoder).
    if (m_encoder && (type == m_encoder_type || type == EncoderType::resource))
        return m_encoder;

    // Close the current encoder.
    if (m_encoder) {
        m_encoder->end();
        m_encoder = nullptr;
    }

    // Create new command buffer if needed.
    if (!m_command_buffer) {
        Slang::ComPtr<gfx::ICommandBuffer> gfx_command_buffer;
        SLANG_CALL(get_current_transient_resource_heap()->createCommandBuffer(gfx_command_buffer.writeRef()));
        m_command_buffer = make_ref<CommandBuffer>(gfx_command_buffer);
    }

    // Open a new encoder.
    switch (type) {
    case EncoderType::none:
        KALI_ASSERT(m_encoder == nullptr);
        break;
    case EncoderType::resource:
        m_encoder = m_command_buffer->encode_resource_commands();
        break;
    case EncoderType::compute:
        m_encoder = m_command_buffer->encode_compute_commands();
        break;
    case EncoderType::render:
        KALI_UNIMPLEMENTED();
        // m_encoder = m_command_buffer->gfx_command_buffer()->encodeRenderCommands()
        break;
    case EncoderType::raytracing:
        m_encoder = m_command_buffer->encode_ray_tracing_commands();
        break;
    }

    m_encoder_type = type;
    return m_encoder;
}

// ----------------------------------------------------------------------------
// CommandEncoder
// ----------------------------------------------------------------------------

CommandEncoder::CommandEncoder(CommandBuffer* command_buffer)
    : m_command_buffer(command_buffer)
{
}

void CommandEncoder::begin(gfx::ICommandEncoder* gfx_command_encoder)
{
    m_gfx_command_encoder = gfx_command_encoder;
}

void CommandEncoder::end()
{
    m_gfx_command_encoder->endEncoding();
    m_gfx_command_encoder = nullptr;
    m_command_buffer->m_current_encoder = nullptr;
}

void CommandEncoder::write_timestamp(QueryPool* query_pool, uint32_t index)
{
    KALI_ASSERT(m_gfx_command_encoder);
    KALI_ASSERT(query_pool != nullptr);
    m_gfx_command_encoder->writeTimestamp(query_pool->gfx_query_pool(), index);
}

// ----------------------------------------------------------------------------
// ResourceCommandEncoder
// ----------------------------------------------------------------------------

ResourceCommandEncoder::ResourceCommandEncoder(CommandBuffer* command_buffer)
    : CommandEncoder(command_buffer)
{
}

void ResourceCommandEncoder::buffer_barrier(
    std::span<const Buffer*> buffers,
    ResourceState old_state,
    ResourceState new_state
)
{
    ShortVector<gfx::IBufferResource*, 16> gfx_buffers(buffers.size(), nullptr);
    for (size_t i = 0; i < buffers.size(); i++)
        gfx_buffers[i] = buffers[i]->gfx_buffer_resource();
    gfx_resource_command_encoder()->bufferBarrier(
        narrow_cast<gfx::GfxCount>(buffers.size()),
        gfx_buffers.data(),
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void ResourceCommandEncoder::buffer_barrier(const Buffer* buffer, ResourceState old_state, ResourceState new_state)
{
    KALI_ASSERT(buffer != nullptr);

    gfx::IBufferResource* gfx_buffer_resource = buffer->gfx_buffer_resource();
    gfx_resource_command_encoder()->bufferBarrier(
        1,
        &gfx_buffer_resource,
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void ResourceCommandEncoder::texture_barrier(
    std::span<const Texture*> textures,
    ResourceState old_state,
    ResourceState new_state
)
{
    ShortVector<gfx::ITextureResource*, 16> gfx_textures(textures.size(), nullptr);
    for (size_t i = 0; i < textures.size(); i++)
        gfx_textures[i] = textures[i]->gfx_texture_resource();
    gfx_resource_command_encoder()->textureBarrier(
        narrow_cast<gfx::GfxCount>(textures.size()),
        gfx_textures.data(),
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void ResourceCommandEncoder::texture_barrier(const Texture* texture, ResourceState old_state, ResourceState new_state)
{
    KALI_ASSERT(texture != nullptr);

    gfx::ITextureResource* gfx_texture_resource = texture->gfx_texture_resource();
    gfx_resource_command_encoder()->textureBarrier(
        1,
        &gfx_texture_resource,
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void ResourceCommandEncoder::texture_subresource_barrier(
    const Texture* texture,
    SubresourceRange subresource_range,
    ResourceState old_state,
    ResourceState new_state
)
{
    KALI_ASSERT(texture != nullptr);
    KALI_ASSERT(subresource_range.mip_level < texture->mip_count());
    KALI_ASSERT(subresource_range.mip_level + subresource_range.mip_count <= texture->mip_count());
    KALI_ASSERT(subresource_range.base_array_layer < texture->array_size());
    KALI_ASSERT(subresource_range.base_array_layer + subresource_range.layer_count <= texture->array_size());

    gfx::SubresourceRange gfx_subresource_range = {
        .aspectMask = static_cast<gfx::TextureAspect>(subresource_range.texture_aspect),
        .mipLevel = narrow_cast<gfx::GfxIndex>(subresource_range.mip_level),
        .mipLevelCount = narrow_cast<gfx::GfxCount>(subresource_range.mip_count),
        .baseArrayLayer = narrow_cast<gfx::GfxIndex>(subresource_range.base_array_layer),
        .layerCount = narrow_cast<gfx::GfxCount>(subresource_range.layer_count),
    };

    gfx_resource_command_encoder()->textureSubresourceBarrier(
        texture->gfx_texture_resource(),
        gfx_subresource_range,
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void ResourceCommandEncoder::copy_buffer(
    Buffer* dst,
    DeviceOffset dst_offset,
    const Buffer* src,
    DeviceOffset src_offset,
    DeviceSize size
)
{
    KALI_ASSERT(dst != nullptr);
    KALI_ASSERT_LE(dst_offset + size, dst->size());
    KALI_ASSERT(src != nullptr);
    KALI_ASSERT_LE(src_offset + size, src->size());

    gfx_resource_command_encoder()
        ->copyBuffer(dst->gfx_buffer_resource(), dst_offset, src->gfx_buffer_resource(), src_offset, size);
}

void ResourceCommandEncoder::upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data)
{
    KALI_ASSERT(buffer);
    KALI_ASSERT_LE(offset + size, buffer->size());
    KALI_ASSERT(data);

    gfx_resource_command_encoder()
        ->uploadBufferData(buffer->gfx_buffer_resource(), offset, size, const_cast<void*>(data));
}

void ResourceCommandEncoder::begin_debug_event(const char* name, float3 color)
{
    KALI_ASSERT(name != nullptr);

    gfx_resource_command_encoder()->beginDebugEvent(name, &color.x);
}

void ResourceCommandEncoder::end_debug_event()
{
    gfx_resource_command_encoder()->endDebugEvent();
}

// ----------------------------------------------------------------------------
// ComputeCommandEncoder
// ----------------------------------------------------------------------------

ComputeCommandEncoder::ComputeCommandEncoder(CommandBuffer* command_buffer)
    : ResourceCommandEncoder(command_buffer)
{
}

ref<TransientShaderObject> ComputeCommandEncoder::bind_pipeline(const ComputePipelineState* pipeline)
{
    KALI_ASSERT(pipeline != nullptr);

    gfx::IShaderObject* gfx_shader_object;
    SLANG_CALL(gfx_compute_command_encoder()->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object));
    return make_ref<TransientShaderObject>(gfx_shader_object, nullptr);
}

void ComputeCommandEncoder::bind_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object)
{
    KALI_ASSERT(pipeline != nullptr);
    KALI_ASSERT(shader_object != nullptr);

    SLANG_CALL(gfx_compute_command_encoder()
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void ComputeCommandEncoder::dispatch_compute(uint3 thread_group_count)
{
    gfx_compute_command_encoder()->dispatchCompute(thread_group_count.x, thread_group_count.y, thread_group_count.z);
}

void ComputeCommandEncoder::dispatch_compute_indirect(const Buffer* cmd_buffer, DeviceOffset offset)
{
    KALI_ASSERT(cmd_buffer != nullptr);

    gfx_compute_command_encoder()->dispatchComputeIndirect(cmd_buffer->gfx_buffer_resource(), offset);
}

// ----------------------------------------------------------------------------
// RenderCommandEncoder
// ----------------------------------------------------------------------------

RenderCommandEncoder::RenderCommandEncoder(CommandBuffer* command_buffer)
    : ResourceCommandEncoder(command_buffer)
{
}

ref<TransientShaderObject> RenderCommandEncoder::bind_pipeline(const GraphicsPipelineState* pipeline)
{
    KALI_ASSERT(pipeline != nullptr);

    gfx::IShaderObject* gfx_shader_object;
    SLANG_CALL(gfx_render_command_encoder()->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object));
    return make_ref<TransientShaderObject>(gfx_shader_object, nullptr);
}

void RenderCommandEncoder::bind_pipeline(const GraphicsPipelineState* pipeline, const ShaderObject* shader_object)
{
    KALI_ASSERT(pipeline != nullptr);
    KALI_ASSERT(shader_object != nullptr);

    SLANG_CALL(gfx_render_command_encoder()
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void RenderCommandEncoder::set_viewports(std::span<Viewport> viewports)
{
    gfx_render_command_encoder()->setViewports(
        narrow_cast<gfx::GfxCount>(viewports.size()),
        reinterpret_cast<const gfx::Viewport*>(viewports.data())
    );
}

void RenderCommandEncoder::set_scissor_rects(std::span<ScissorRect> scissor_rects)
{
    gfx_render_command_encoder()->setScissorRects(
        narrow_cast<gfx::GfxCount>(scissor_rects.size()),
        reinterpret_cast<const gfx::ScissorRect*>(scissor_rects.data())
    );
}

void RenderCommandEncoder::set_viewport_and_scissor_rect(const Viewport& viewport)
{
    gfx_render_command_encoder()->setViewportAndScissor(reinterpret_cast<const gfx::Viewport&>(viewport));
}

// void set_primitive_topology(PrimitiveTopology topology);

void RenderCommandEncoder::set_stencil_reference(uint32_t reference_value)
{
    gfx_render_command_encoder()->setStencilReference(reference_value);
}

void RenderCommandEncoder::set_vertex_buffers(uint32_t start_slot, std::span<Slot> slots)
{
    ShortVector<gfx::IBufferResource*, 16> gfx_buffers(slots.size(), nullptr);
    ShortVector<gfx::Offset, 16> gfx_offsets(slots.size(), 0);
    for (size_t i = 0; i < slots.size(); i++) {
        gfx_buffers[i] = slots[i].buffer->gfx_buffer_resource();
        gfx_offsets[i] = slots[i].offset;
    }
    gfx_render_command_encoder()->setVertexBuffers(
        start_slot,
        narrow_cast<gfx::GfxCount>(slots.size()),
        gfx_buffers.data(),
        gfx_offsets.data()
    );
}

void RenderCommandEncoder::set_vertex_buffer(uint32_t slot, const Buffer* buffer, DeviceOffset offset)
{
    gfx::IBufferResource* gfx_buffer = buffer->gfx_buffer_resource();
    gfx_render_command_encoder()->setVertexBuffer(slot, gfx_buffer, offset);
}

void RenderCommandEncoder::set_index_buffer(const Buffer* buffer, Format index_format, DeviceOffset offset)
{
    gfx_render_command_encoder()
        ->setIndexBuffer(buffer->gfx_buffer_resource(), static_cast<gfx::Format>(index_format), offset);
}

void RenderCommandEncoder::draw(uint32_t vertex_count, uint32_t start_vertex)
{
    SLANG_CALL(gfx_render_command_encoder()->draw(vertex_count, start_vertex));
}

// ----------------------------------------------------------------------------
// RayTracingCommandEncoder
// ----------------------------------------------------------------------------

RayTracingCommandEncoder::RayTracingCommandEncoder(CommandBuffer* command_buffer)
    : ResourceCommandEncoder(command_buffer)
{
}

ref<TransientShaderObject> RayTracingCommandEncoder::bind_pipeline(const RayTracingPipelineState* pipeline)
{
    gfx::IShaderObject* gfx_shader_object;
    // TODO: fix slang-gfx to return Result
    gfx_ray_tracing_command_encoder()->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object);
    return make_ref<TransientShaderObject>(gfx_shader_object, nullptr);
}

void RayTracingCommandEncoder::bind_pipeline(const RayTracingPipelineState* pipeline, const ShaderObject* shader_object)
{
    SLANG_CALL(gfx_ray_tracing_command_encoder()
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void RayTracingCommandEncoder::dispatch_rays(
    uint32_t ray_gen_shader_index,
    const ShaderTable* shader_table,
    uint3 dimensions
)
{
    SLANG_CALL(gfx_ray_tracing_command_encoder()->dispatchRays(
        ray_gen_shader_index,
        shader_table->gfx_shader_table(),
        dimensions.x,
        dimensions.y,
        dimensions.z
    ));
}

void RayTracingCommandEncoder::serialize_acceleration_structure(DeviceAddress dst, const AccelerationStructure* src)
{
    gfx_ray_tracing_command_encoder()->serializeAccelerationStructure(dst, src->gfx_acceleration_structure());
}

void RayTracingCommandEncoder::deserialize_acceleration_structure(AccelerationStructure* dst, DeviceAddress src)
{
    gfx_ray_tracing_command_encoder()->deserializeAccelerationStructure(dst->gfx_acceleration_structure(), src);
}

// ----------------------------------------------------------------------------
// CommandBuffer
// ----------------------------------------------------------------------------

CommandBuffer::CommandBuffer(Slang::ComPtr<gfx::ICommandBuffer> gfx_command_buffer)
    : m_gfx_command_buffer(std::move(gfx_command_buffer))
    , m_resource_command_encoder(this)
    , m_compute_command_encoder(this)
    , m_graphics_command_encoder(this)
    , m_ray_tracing_command_encoder(this)
{
}

ResourceCommandEncoder* CommandBuffer::encode_resource_commands()
{
    KALI_CHECK(m_current_encoder == nullptr, "Another encoder is already active.");
    m_resource_command_encoder.begin(m_gfx_command_buffer->encodeResourceCommands());
    m_current_encoder = &m_resource_command_encoder;
    return &m_resource_command_encoder;
}

ComputeCommandEncoder* CommandBuffer::encode_compute_commands()
{
    KALI_CHECK(m_current_encoder == nullptr, "Another encoder is already active.");
    m_compute_command_encoder.begin(m_gfx_command_buffer->encodeComputeCommands());
    m_current_encoder = &m_compute_command_encoder;
    return &m_compute_command_encoder;
}

RayTracingCommandEncoder* CommandBuffer::encode_ray_tracing_commands()
{
    KALI_CHECK(m_current_encoder == nullptr, "Another encoder is already active.");
    m_ray_tracing_command_encoder.begin(m_gfx_command_buffer->encodeRayTracingCommands());
    m_current_encoder = &m_ray_tracing_command_encoder;
    return &m_ray_tracing_command_encoder;
}

} // namespace kali
