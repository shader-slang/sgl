#include "command.h"

#include "kali/device/device.h"
#include "kali/device/native_handle_traits.h"
#include "kali/device/helpers.h"
#include "kali/device/shader.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/reflection.h"
#include "kali/device/pipeline.h"

#include "kali/core/error.h"
#include "kali/core/maths.h"
#include "kali/core/type_utils.h"
#include "kali/core/string.h"

#include "kali/math/vector_math.h"

namespace kali {

inline gfx::ICommandQueue::QueueType get_gfx_command_queue_type(CommandQueueType type)
{
    static_assert(uint32_t(CommandQueueType::graphics) == uint32_t(gfx::ICommandQueue::QueueType::Graphics));
    KALI_ASSERT(uint32_t(type) <= uint32_t(CommandQueueType::graphics));
    return gfx::ICommandQueue::QueueType(type);
}

CommandQueue::CommandQueue(ref<Device> device, CommandQueueDesc desc)
    : m_device(std::move(device))
    , m_desc(std::move(desc))
{
    gfx::ICommandQueue::Desc gfx_desc{
        .type = get_gfx_command_queue_type(m_desc.type),
    };

    SLANG_CALL(m_device->get_gfx_device()->createCommandQueue(gfx_desc, m_gfx_command_queue.writeRef()));
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

void CommandQueue::break_strong_reference_to_device()
{
    m_device.break_strong_reference();
}

std::string CommandQueue::to_string() const
{
    return fmt::format(
        "CommandQueue("
        "    desc={}\n",
        ")",
        string::indent(m_desc.to_string())
    );
}

// defined in resource.cpp
gfx::ResourceState get_gfx_resource_state(ResourceState resource_state);

CommandStream::CommandStream(ref<Device> device, CommandStreamDesc desc)
    : m_device(std::move(device))
    , m_desc(std::move(desc))
{
    m_command_queue = m_device->create_command_queue({.type = m_desc.queue_type});
    m_command_queue->break_strong_reference_to_device();

    m_compute_pipeline_cache = m_device->create_compute_pipeline_cache();
    m_compute_pipeline_cache->break_strong_reference_to_device();

    // Create per-frame data.
    m_frame_data.resize(m_desc.frame_count);
    for (auto& frame_data : m_frame_data) {
        m_device->get_gfx_device()->createTransientResourceHeap(
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
            m_encoder->endEncoding();
            m_encoder = nullptr;
        }
        m_command_buffer->close();
        m_command_queue->get_gfx_command_queue()->executeCommandBuffer(m_command_buffer);
        m_command_buffer = nullptr;
    }
}

// ----------------------------------------------------------------------------
// Synchronization
// ----------------------------------------------------------------------------

uint64_t CommandStream::signal(Fence* fence, uint64_t value)
{
    KALI_CHECK(fence, "'fence' must not be null");
    uint64_t signal_value = fence->update_signaled_value(value);
    m_command_queue->get_gfx_command_queue()->executeCommandBuffers(0, nullptr, fence->get_gfx_fence(), signal_value);
    return signal_value;
}

void CommandStream::wait(Fence* fence, uint64_t value)
{
    KALI_CHECK(fence, "'fence' must not be null");
    uint64_t wait_value = value == Fence::AUTO ? fence->signaled_value() : value;
    gfx::IFence* fences[] = {fence->get_gfx_fence()};
    uint64_t wait_values[] = {wait_value};
    SLANG_CALL(m_command_queue->get_gfx_command_queue()->waitForFenceValuesOnDevice(1, fences, wait_values));
}

// ----------------------------------------------------------------------------
// Barriers
// ----------------------------------------------------------------------------

bool CommandStream::buffer_barrier(const Buffer* buffer, ResourceState new_state)
{
    KALI_ASSERT(buffer);

    bool recorded = false;
    ResourceState current_state = buffer->global_state();
    if (current_state != new_state) {
        auto encoder = get_resource_command_encoder();
        gfx::IBufferResource* gfx_buffer_resource = buffer->get_gfx_buffer_resource();
        encoder->bufferBarrier(
            1,
            &gfx_buffer_resource,
            get_gfx_resource_state(current_state),
            get_gfx_resource_state(new_state)
        );
        buffer->set_global_state(new_state);
        mark_pending();
        recorded = true;
    }

    return recorded;
}

bool CommandStream::texture_barrier(const Texture* texture, ResourceState new_state)
{
    KALI_ASSERT(texture);

    bool recorded = false;
    ResourceState current_state = texture->global_state();
    if (current_state != new_state) {
        auto encoder = get_resource_command_encoder();
        gfx::ITextureResource* gfx_texture_resource = texture->get_gfx_texture_resource();
        encoder->textureBarrier(
            1,
            &gfx_texture_resource,
            get_gfx_resource_state(current_state),
            get_gfx_resource_state(new_state)
        );
        texture->set_global_state(new_state);
        mark_pending();
        recorded = true;
    }

    return recorded;
}

bool CommandStream::resource_barrier(const Resource* resource, ResourceState new_state)
{
    if (resource->type() == ResourceType::buffer) {
        return buffer_barrier(static_cast<const Buffer*>(resource), new_state);
    } else {
        return texture_barrier(static_cast<const Texture*>(resource), new_state);
    }
}

void CommandStream::uav_barrier(const Resource* resource)
{
    KALI_ASSERT(resource);

    auto encoder = get_resource_command_encoder();
    if (resource->type() == ResourceType::buffer) {
        gfx::IBufferResource* buffer = static_cast<gfx::IBufferResource*>(resource->get_gfx_resource());
        encoder->bufferBarrier(1, &buffer, gfx::ResourceState::UnorderedAccess, gfx::ResourceState::UnorderedAccess);
    } else {
        gfx::ITextureResource* texture = static_cast<gfx::ITextureResource*>(resource->get_gfx_resource());
        encoder->textureBarrier(1, &texture, gfx::ResourceState::UnorderedAccess, gfx::ResourceState::UnorderedAccess);
    }
    mark_pending();
}

// ----------------------------------------------------------------------------
// Resource copying
// ----------------------------------------------------------------------------

#if 0
void CommandStream::copy_resource(const Resource* dst, const Resource* src)
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
            dst_buffer->get_gfx_buffer_resource(),
            dst_buffer->get_gpu_address_offset(),
            src_buffer->get_gfx_buffer_resource(),
            src_buffer->get_gpu_address_offset(),
            src_buffer->get_size()
        );
    } else {
        const Texture* dst_texture = static_cast<const Texture*>(dst);
        const Texture* src_texture = static_cast<const Texture*>(src);
        gfx::SubresourceRange sr_range = {};

        encoder->copyTexture(
            dst_texture->get_gfx_texture_resource(),
            gfx::ResourceState::CopyDestination,
            sr_range,
            gfx::ITextureResource::Offset3D(0, 0, 0),
            src_texture->get_gfx_texture_resource(),
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
    const Buffer* dst,
    DeviceOffset dst_offset,
    const Buffer* src,
    DeviceOffset src_offset,
    DeviceSize size
)
{
    // TODO(@skallweit): check for overlapping copies within same buffer?
    KALI_ASSERT(dst);
    KALI_ASSERT(src);
    KALI_ASSERT_LE(dst_offset + size, dst->size());
    KALI_ASSERT_LE(src_offset + size, src->size());

    buffer_barrier(dst, ResourceState::copy_destination);
    buffer_barrier(src, ResourceState::copy_source);

    auto encoder = get_resource_command_encoder();
    encoder->copyBuffer(dst->get_gfx_buffer_resource(), dst_offset, src->get_gfx_buffer_resource(), src_offset, size);
    mark_pending();
}

#if 0
void CommandStream::copy_texture_region(
    const Texture* dst,
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

void CommandStream::upload_buffer_data(const Buffer* buffer, size_t offset, size_t size, const void* data)
{
    KALI_ASSERT(buffer);
    KALI_ASSERT_LE(offset + size, buffer->size());
    KALI_ASSERT(data);

    buffer_barrier(buffer, ResourceState::copy_destination);

    auto encoder = get_resource_command_encoder();
    encoder->uploadBufferData(buffer->get_gfx_buffer_resource(), offset, size, const_cast<void*>(data));
    mark_pending();
}

void CommandStream::upload_texture_data(const Texture* texture, const void* data)
{
    KALI_ASSERT(texture);
    KALI_ASSERT(data);

    upload_texture_subresource_data(texture, 0, texture->subresource_count(), data);
}

void CommandStream::upload_texture_subresource_data(
    const Texture* texture,
    uint32_t subresource_index,
    uint32_t subresource_count,
    const void* data,
    uint3 offset,
    uint3 size
)
{
    KALI_ASSERT(texture);
    KALI_ASSERT_LT(subresource_index, texture->subresource_count());
    KALI_ASSERT_LE(subresource_index + subresource_count, texture->subresource_count());
    KALI_ASSERT_GE(subresource_count, 1);
    KALI_ASSERT(data);

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
    SLANG_CALL(gfx::gfxGetFormatInfo(get_gfx_format(texture->format()), &format_info));
    for (uint32_t i = subresource_index; i < subresource_index + subresource_count; i++) {
        gfx::SubresourceRange sr_range = {};
        // sr_range.baseArrayLayer = narrow_cast<gfx::GfxIndex>(pTexture->getSubresourceArraySlice(i));
        // sr_range.mipLevel = narrow_cast<gfx::GfxIndex>(pTexture->getSubresourceMipLevel(i));
        sr_range.layerCount = 1;
        sr_range.mipLevelCount = 1;
        if (!copy_region) {
            gfx_size.width
                = align_to(format_info.blockWidth, narrow_cast<gfx::GfxCount>(texture->get_width(sr_range.mipLevel)));
            gfx_size.height
                = align_to(format_info.blockHeight, narrow_cast<gfx::GfxCount>(texture->get_height(sr_range.mipLevel)));
            gfx_size.depth = narrow_cast<gfx::GfxCount>(texture->get_depth(sr_range.mipLevel));
        }
        gfx::ITextureResource::SubresourceData sr_data = {};
        sr_data.data = data_ptr;
        sr_data.strideY = narrow_cast<int64_t>(gfx_size.width) / format_info.blockWidth * format_info.blockSizeInBytes;
        sr_data.strideZ = sr_data.strideY * (gfx_size.height / format_info.blockHeight);
        data_ptr += sr_data.strideZ * gfx_size.depth;
        encoder->uploadTextureData(texture->get_gfx_texture_resource(), sr_range, gfx_offset, gfx_size, &sr_data, 1);
    }

    mark_pending();
}

// ----------------------------------------------------------------------------
// Compute
// ----------------------------------------------------------------------------

ref<ShaderObject> CommandStream::bind_compute_pipeline(const ComputePipelineState* pipeline)
{
    gfx::IShaderObject* shader_object = get_compute_command_encoder()->bindPipeline(pipeline->get_gfx_pipeline_state());
    return make_ref<TransientShaderObject>(shader_object, this);
}

void CommandStream::bind_compute_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object)
{
    KALI_UNUSED(pipeline);
    KALI_UNUSED(shader_object);
    KALI_UNIMPLEMENTED();
}

void CommandStream::dispatch_compute(uint3 thread_group_count)
{
    SLANG_CALL(
        get_compute_command_encoder()->dispatchCompute(thread_group_count.x, thread_group_count.y, thread_group_count.z)
    );
}

void CommandStream::dispatch_compute_indirect(Buffer* cmd_buffer, DeviceOffset offset)
{
    SLANG_CALL(get_compute_command_encoder()->dispatchComputeIndirect(cmd_buffer->get_gfx_buffer_resource(), offset));
}

// ----------------------------------------------------------------------------
// Debug events
// ----------------------------------------------------------------------------

void CommandStream::begin_debug_event(const char* name, float3 color)
{
    get_resource_command_encoder()->beginDebugEvent(name, &color.x);
}

void CommandStream::end_debug_event()
{
    get_resource_command_encoder()->endDebugEvent();
}

void CommandStream::mark_pending() { }

void CommandStream::break_strong_reference_to_device()
{
    m_device.break_strong_reference();
}

gfx::IResourceCommandEncoder* CommandStream::get_resource_command_encoder()
{
    return reinterpret_cast<gfx::IResourceCommandEncoder*>(request_encoder(EncoderType::resource));
}

gfx::IComputeCommandEncoder* CommandStream::get_compute_command_encoder()
{
    return reinterpret_cast<gfx::IComputeCommandEncoder*>(request_encoder(EncoderType::compute));
}

gfx::IRenderCommandEncoder* CommandStream::get_render_command_encoder()
{
    return reinterpret_cast<gfx::IRenderCommandEncoder*>(request_encoder(EncoderType::render));
}

gfx::IRayTracingCommandEncoder* CommandStream::get_raytracing_command_encoder()
{
    return reinterpret_cast<gfx::IRayTracingCommandEncoder*>(request_encoder(EncoderType::raytracing));
}

gfx::ICommandEncoder* CommandStream::request_encoder(EncoderType type)
{
    // Return open encoder if type matches (all encoders inherit the resource encoder).
    if (m_encoder && (type == m_encoder_type || type == EncoderType::resource))
        return m_encoder;

    // Close the current encoder.
    if (m_encoder) {
        m_encoder->endEncoding();
        m_encoder = nullptr;
    }

    // Create new command buffer if needed.
    if (!m_command_buffer) {
        SLANG_CALL(get_current_transient_resource_heap()->createCommandBuffer(m_command_buffer.writeRef()));
    }

    // Open a new encoder.
    switch (type) {
    case EncoderType::none:
        KALI_ASSERT(m_encoder == nullptr);
        break;
    case EncoderType::resource:
        m_encoder = m_command_buffer->encodeResourceCommands();
        break;
    case EncoderType::compute:
        m_encoder = m_command_buffer->encodeComputeCommands();
        break;
    case EncoderType::render:
        KALI_UNIMPLEMENTED();
        // m_encoder = m_command_buffer->encodeRenderCommands()
        break;
    case EncoderType::raytracing:
        m_encoder = m_command_buffer->encodeRayTracingCommands();
        break;
    }

    m_encoder_type = type;
    return m_encoder;
}

} // namespace kali
