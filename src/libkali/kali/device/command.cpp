#include "command.h"

#include "kali/device/helpers.h"
#include "kali/device/device.h"
#include "kali/device/fence.h"
#include "kali/device/resource.h"
#include "kali/device/pipeline.h"
#include "kali/device/raytracing.h"
#include "kali/device/shader_object.h"

#include "kali/core/short_vector.h"
#include "kali/core/maths.h"
#include "kali/core/type_utils.h"

#include "kali/math/vector.h"

namespace kali {

namespace detail {
    gfx::SubresourceRange gfx_subresource_range(const Texture* texture, uint32_t subresource_index)
    {
        return gfx::SubresourceRange{
            .mipLevel = narrow_cast<gfx::GfxIndex>(texture->get_subresource_mip_level(subresource_index)),
            .mipLevelCount = 1,
            .baseArrayLayer = narrow_cast<gfx::GfxIndex>(texture->get_subresource_array_slice(subresource_index)),
            .layerCount = 1,
        };
    }
} // namespace detail

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

void CommandQueue::submit(const CommandBuffer* command_buffer)
{
    KALI_CHECK_NOT_NULL(command_buffer);
    m_gfx_command_queue->executeCommandBuffer(command_buffer->gfx_command_buffer());
}

void CommandQueue::wait()
{
    m_gfx_command_queue->waitOnHost();
}

uint64_t CommandQueue::signal(Fence* fence, uint64_t value)
{
    KALI_CHECK_NOT_NULL(fence);

    uint64_t signal_value = fence->update_signaled_value(value);
    m_gfx_command_queue->executeCommandBuffers(0, nullptr, fence->gfx_fence(), signal_value);
    return signal_value;
}

void CommandQueue::wait(const Fence* fence, uint64_t value)
{
    KALI_CHECK_NOT_NULL(fence);

    uint64_t wait_value = value == Fence::AUTO ? fence->signaled_value() : value;
    gfx::IFence* fences[] = {fence->gfx_fence()};
    uint64_t wait_values[] = {wait_value};
    SLANG_CALL(m_gfx_command_queue->waitForFenceValuesOnDevice(1, fences, wait_values));
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
// CommandBuffer
// ----------------------------------------------------------------------------

CommandBuffer::CommandBuffer(ref<Device> device, Slang::ComPtr<gfx::ICommandBuffer> gfx_command_buffer)
    : DeviceResource(std::move(device))
    , m_gfx_command_buffer(std::move(gfx_command_buffer))
{
}

std::string CommandBuffer::to_string() const
{
    return fmt::format(
        "CommandBuffer(\n"
        "  device={}\n"
        ")",
        m_device
    );
}

#if 0
// ----------------------------------------------------------------------------
// PassEncoder
// ----------------------------------------------------------------------------

PassEncoder::~PassEncoder()
{
    if (m_command_stream)
        m_command_stream->end_pass();
}
#endif

// ----------------------------------------------------------------------------
// ComputePassEncoder
// ----------------------------------------------------------------------------

ComputePassEncoder::~ComputePassEncoder()
{
    end();
}

void ComputePassEncoder::end()
{
    if (m_command_stream) {
        m_command_stream->end_pass();
        m_command_stream = nullptr;
    }
}

ref<TransientShaderObject> ComputePassEncoder::bind_pipeline(const ComputePipelineState* pipeline)
{
    KALI_CHECK_NOT_NULL(pipeline);

    m_bound_pipeline = pipeline;
    gfx::IShaderObject* gfx_shader_object;
    SLANG_CALL(m_gfx_compute_command_encoder->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object));
    return make_ref<TransientShaderObject>(gfx_shader_object, m_command_stream);
}

void ComputePassEncoder::bind_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object)
{
    KALI_CHECK_NOT_NULL(pipeline);
    KALI_CHECK_NOT_NULL(shader_object);

    m_bound_pipeline = pipeline;
    SLANG_CALL(m_gfx_compute_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void ComputePassEncoder::dispatch(uint3 thread_count)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

    uint3 thread_group_size = m_bound_pipeline->thread_group_size();
    uint3 thread_group_count{
        div_round_up(thread_count.x, thread_group_size.x),
        div_round_up(thread_count.y, thread_group_size.y),
        div_round_up(thread_count.z, thread_group_size.z)};
    dispatch_thread_groups(thread_group_count);
}

void ComputePassEncoder::dispatch_thread_groups(uint3 thread_group_count)
{
    SLANG_CALL(
        m_gfx_compute_command_encoder->dispatchCompute(thread_group_count.x, thread_group_count.y, thread_group_count.z)
    );
}

void ComputePassEncoder::dispatch_thread_groups_indirect(const Buffer* cmd_buffer, DeviceOffset offset)
{
    KALI_CHECK_NOT_NULL(cmd_buffer);

    SLANG_CALL(m_gfx_compute_command_encoder->dispatchComputeIndirect(cmd_buffer->gfx_buffer_resource(), offset));
}

#if 0
// ----------------------------------------------------------------------------
// RenderPassEncoder
// ----------------------------------------------------------------------------

ref<TransientShaderObject> RenderPassEncoder::bind_pipeline(const GraphicsPipelineState* pipeline)
{
    KALI_CHECK_NOT_NULL(pipeline);

    gfx::IShaderObject* gfx_shader_object;
    SLANG_CALL(m_gfx_render_command_encoder->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object));
    return make_ref<TransientShaderObject>(gfx_shader_object, m_command_stream);
}

void RenderPassEncoder::bind_pipeline(const GraphicsPipelineState* pipeline, const ShaderObject* shader_object)
{
    KALI_CHECK_NOT_NULL(pipeline);
    KALI_CHECK_NOT_NULL(shader_object);

    SLANG_CALL(m_gfx_render_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

// ----------------------------------------------------------------------------
// RayTracingPassEncoder
// ----------------------------------------------------------------------------

ref<TransientShaderObject> RayTracingPassEncoder::bind_pipeline(const RayTracingPipelineState* pipeline)
{
    KALI_CHECK_NOT_NULL(pipeline);

    gfx::IShaderObject* gfx_shader_object;
    // TODO gfx bindPipeline should return Result (fix in slang/gfx)
    m_gfx_ray_tracing_command_encoder->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object);
    return make_ref<TransientShaderObject>(gfx_shader_object, m_command_stream);
}

void RayTracingPassEncoder::bind_pipeline(const RayTracingPipelineState* pipeline, const ShaderObject* shader_object)
{
    KALI_CHECK_NOT_NULL(pipeline);
    KALI_CHECK_NOT_NULL(shader_object);

    SLANG_CALL(m_gfx_ray_tracing_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void RayTracingPassEncoder::dispatch_rays(
    uint32_t ray_gen_shader_index,
    const ShaderTable* shader_table,
    uint3 dimensions
)
{
    KALI_CHECK_NOT_NULL(shader_table);

    SLANG_CALL(m_gfx_ray_tracing_command_encoder->dispatchRays(
        narrow_cast<gfx::GfxIndex>(ray_gen_shader_index),
        shader_table->gfx_shader_table(),
        narrow_cast<gfx::GfxCount>(dimensions.x),
        narrow_cast<gfx::GfxCount>(dimensions.y),
        narrow_cast<gfx::GfxCount>(dimensions.z)
    ));
}
#endif

// ----------------------------------------------------------------------------
// CommandStream
// ----------------------------------------------------------------------------

CommandStream::CommandStream(ref<Device> device, ref<CommandQueue> command_queue)
    : DeviceResource(std::move(device))
    , m_command_queue(std::move(command_queue))
{
}

void CommandStream::submit()
{
    if (m_command_buffer) {
        end_current_encoder();
        m_command_buffer->gfx_command_buffer()->close();
        m_command_queue->submit(m_command_buffer);
        m_command_buffer = nullptr;
    }
}

uint64_t CommandStream::signal(Fence* fence, uint64_t value)
{
    return m_command_queue->signal(fence, value);
}

void CommandStream::wait(const Fence* fence, uint64_t value)
{
    return m_command_queue->wait(fence, value);
}

bool CommandStream::buffer_barrier(const Buffer* buffer, ResourceState new_state)
{
    KALI_CHECK_NOT_NULL(buffer);

    ResourceState current_state = buffer->state_tracker().global_state();
    if (new_state == current_state)
        return false;

    buffer_barrier(buffer, current_state, new_state);
    buffer->state_tracker().set_global_state(new_state);
    return true;
}

bool CommandStream::texture_barrier(const Texture* texture, ResourceState new_state)
{
    KALI_CHECK_NOT_NULL(texture);

    ResourceState current_state = texture->state_tracker().global_state();
    if (new_state == current_state)
        return false;

    texture_barrier(texture, current_state, new_state);
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

    texture_subresource_barrier(
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
        buffer_barrier(
            static_cast<const Buffer*>(resource),
            ResourceState::unordered_access,
            ResourceState::unordered_access
        );
    } else {
        texture_barrier(
            static_cast<const Texture*>(resource),
            ResourceState::unordered_access,
            ResourceState::unordered_access
        );
    }
}

void CommandStream::buffer_barrier(std::span<const Buffer*> buffers, ResourceState old_state, ResourceState new_state)
{
    ShortVector<gfx::IBufferResource*, 16> gfx_buffers(buffers.size(), nullptr);
    for (size_t i = 0; i < buffers.size(); i++)
        gfx_buffers[i] = buffers[i]->gfx_buffer_resource();
    get_gfx_resource_command_encoder()->bufferBarrier(
        narrow_cast<gfx::GfxCount>(buffers.size()),
        gfx_buffers.data(),
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void CommandStream::buffer_barrier(const Buffer* buffer, ResourceState old_state, ResourceState new_state)
{
    KALI_ASSERT(buffer != nullptr);

    gfx::IBufferResource* gfx_buffer_resource = buffer->gfx_buffer_resource();
    get_gfx_resource_command_encoder()->bufferBarrier(
        1,
        &gfx_buffer_resource,
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void CommandStream::texture_barrier(
    std::span<const Texture*> textures,
    ResourceState old_state,
    ResourceState new_state
)
{
    ShortVector<gfx::ITextureResource*, 16> gfx_textures(textures.size(), nullptr);
    for (size_t i = 0; i < textures.size(); i++)
        gfx_textures[i] = textures[i]->gfx_texture_resource();
    get_gfx_resource_command_encoder()->textureBarrier(
        narrow_cast<gfx::GfxCount>(textures.size()),
        gfx_textures.data(),
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void CommandStream::texture_barrier(const Texture* texture, ResourceState old_state, ResourceState new_state)
{
    KALI_ASSERT(texture != nullptr);

    gfx::ITextureResource* gfx_texture_resource = texture->gfx_texture_resource();
    get_gfx_resource_command_encoder()->textureBarrier(
        1,
        &gfx_texture_resource,
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void CommandStream::texture_subresource_barrier(
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

    get_gfx_resource_command_encoder()->textureSubresourceBarrier(
        texture->gfx_texture_resource(),
        gfx_subresource_range,
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void CommandStream::copy_resource(Resource* dst, const Resource* src)
{
    KALI_CHECK_NOT_NULL(dst);
    KALI_CHECK_NOT_NULL(src);
    KALI_CHECK(dst->type() == src->type(), "Resources must be of the same type");

    resource_barrier(dst, ResourceState::copy_destination);
    resource_barrier(src, ResourceState::copy_source);

    if (dst->type() == ResourceType::buffer) {
        const Buffer* dst_buffer = static_cast<const Buffer*>(dst);
        const Buffer* src_buffer = static_cast<const Buffer*>(src);

        KALI_CHECK(src_buffer->size() <= dst_buffer->size(), "Source buffer is larger than destination buffer");

        get_gfx_resource_command_encoder()->copyBuffer(
            dst_buffer->gfx_buffer_resource(),
            0,
            src_buffer->gfx_buffer_resource(),
            0,
            src_buffer->size()
        );
    } else {
        const Texture* dst_texture = static_cast<const Texture*>(dst);
        const Texture* src_texture = static_cast<const Texture*>(src);
        gfx::SubresourceRange sr_range = {};

        get_gfx_resource_command_encoder()->copyTexture(
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
}

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
    KALI_CHECK_LE(dst_offset + size, dst->size());
    KALI_CHECK_LE(src_offset + size, src->size());

    buffer_barrier(dst, ResourceState::copy_destination);
    buffer_barrier(src, ResourceState::copy_source);

    get_gfx_resource_command_encoder()
        ->copyBuffer(dst->gfx_buffer_resource(), dst_offset, src->gfx_buffer_resource(), src_offset, size);
}

void CommandStream::copy_texture_region(
    Texture* dst,
    uint32_t dst_subresource,
    const Texture* src,
    uint32_t src_subresource,
    uint3 dst_offset,
    uint3 src_offset,
    uint3 extent
)
{
    KALI_CHECK_NOT_NULL(dst);
    KALI_CHECK_NOT_NULL(src);
    KALI_CHECK_LT(dst_subresource, dst->subresource_count());
    KALI_CHECK_LT(src_subresource, src->subresource_count());

    texture_barrier(dst, ResourceState::copy_destination);
    texture_barrier(src, ResourceState::copy_source);

    gfx::SubresourceRange dst_sr = detail::gfx_subresource_range(dst, dst_subresource);
    gfx::SubresourceRange src_sr = detail::gfx_subresource_range(src, src_subresource);

    if (all(extent == uint3(-1)))
        extent = src->get_mip_dimensions(src_sr.mipLevel) - src_offset;

    gfx::ITextureResource::Extents gfx_extent
        = {narrow_cast<gfx::GfxCount>(extent.x),
           narrow_cast<gfx::GfxCount>(extent.y),
           narrow_cast<gfx::GfxCount>(extent.z)};

    get_gfx_resource_command_encoder()->copyTexture(
        dst->gfx_texture_resource(),
        gfx::ResourceState::CopyDestination,
        dst_sr,
        gfx::ITextureResource::Offset3D(dst_offset.x, dst_offset.y, dst_offset.z),
        src->gfx_texture_resource(),
        gfx::ResourceState::CopySource,
        src_sr,
        gfx::ITextureResource::Offset3D(src_offset.x, src_offset.y, src_offset.z),
        gfx_extent
    );
}

void CommandStream::copy_texture_to_buffer(
    Buffer* dst,
    DeviceOffset dst_offset,
    DeviceSize dst_size,
    DeviceSize dst_row_stride,
    const Texture* src,
    uint32_t src_subresource,
    uint3 src_offset,
    uint3 extent
)
{
    KALI_CHECK_NOT_NULL(dst);
    KALI_CHECK(dst_offset + dst_size <= dst->size(), "Destination buffer is too small");
    KALI_CHECK_NOT_NULL(src);
    KALI_CHECK_LT(src_subresource, src->subresource_count());

    texture_barrier(src, ResourceState::copy_source);
    buffer_barrier(dst, ResourceState::copy_destination);

    gfx::SubresourceRange src_sr = detail::gfx_subresource_range(src, src_subresource);
    get_gfx_resource_command_encoder()->copyTextureToBuffer(
        dst->gfx_buffer_resource(),
        dst_offset,
        dst_size,
        dst_row_stride,
        src->gfx_texture_resource(),
        gfx::ResourceState::CopySource,
        src_sr,
        gfx::ITextureResource::Offset3D(src_offset.x, src_offset.y, src_offset.z),
        gfx::ITextureResource::Extents{
            narrow_cast<gfx::GfxCount>(extent.x),
            narrow_cast<gfx::GfxCount>(extent.y),
            narrow_cast<gfx::GfxCount>(extent.z)}
    );
}

void CommandStream::upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data)
{
    KALI_ASSERT(buffer);
    KALI_ASSERT_LE(offset + size, buffer->size());
    KALI_ASSERT(data);

    get_gfx_resource_command_encoder()
        ->uploadBufferData(buffer->gfx_buffer_resource(), offset, size, const_cast<void*>(data));
}

void CommandStream::resolve_texture(Texture* dst, const Texture* src)
{
    KALI_CHECK_NOT_NULL(dst);
    KALI_CHECK_NOT_NULL(src);
    KALI_CHECK(dst->desc().sample_count == 1, "Destination texture must not be multi-sampled.");
    KALI_CHECK(src->desc().sample_count > 1, "Source texture must be multi-sampled.");
    KALI_CHECK(dst->desc().format == src->desc().format, "Source and destination textures must have the same format.");
    KALI_CHECK(
        dst->desc().width == src->desc().width && dst->desc().height == src->desc().height,
        "Source and destination textures must have the same dimensions."
    );
    KALI_CHECK(
        dst->desc().array_size == src->desc().array_size,
        "Source and destination textures must have the same array size."
    );
    KALI_CHECK(
        dst->desc().mip_count == src->desc().mip_count,
        "Source and destination textures must have the same mip count."
    );

    texture_barrier(dst, ResourceState::resolve_destination);
    texture_barrier(src, ResourceState::resolve_source);

    gfx::SubresourceRange dst_sr = {};
    dst_sr.layerCount = dst->array_size();
    dst_sr.mipLevelCount = dst->mip_count();
    gfx::SubresourceRange src_sr = {};
    src_sr.layerCount = src->array_size();
    src_sr.mipLevelCount = src->mip_count();

    get_gfx_resource_command_encoder()->resolveResource(
        src->gfx_texture_resource(),
        gfx::ResourceState::ResolveSource,
        src_sr,
        dst->gfx_texture_resource(),
        gfx::ResourceState::ResolveDestination,
        dst_sr
    );
}

void CommandStream::resolve_subresource(
    Texture* dst,
    uint32_t dst_subresource,
    const Texture* src,
    uint32_t src_subresource
)
{
    KALI_CHECK_NOT_NULL(dst);
    KALI_CHECK_NOT_NULL(src);
    KALI_CHECK_LT(dst_subresource, dst->subresource_count());
    KALI_CHECK_LT(src_subresource, src->subresource_count());
    KALI_CHECK(dst->desc().sample_count == 1, "Destination texture must not be multi-sampled.");
    KALI_CHECK(src->desc().sample_count > 1, "Source texture must be multi-sampled.");
    KALI_CHECK(dst->desc().format == src->desc().format, "Source and destination textures must have the same format.");
    uint32_t dst_mip_level = dst->get_subresource_mip_level(dst_subresource);
    uint32_t dst_array_slice = dst->get_subresource_array_slice(dst_subresource);
    uint32_t src_mip_level = src->get_subresource_mip_level(src_subresource);
    uint32_t src_array_slice = src->get_subresource_array_slice(src_subresource);
    KALI_CHECK(all(dst->get_mip_dimensions(dst_mip_level) == src->get_mip_dimensions(src_mip_level)), "Source and destination textures must have the same dimensions.");

    // TODO it would be better to just use barriers on the subresources.
    texture_barrier(dst, ResourceState::resolve_destination);
    texture_barrier(src, ResourceState::resolve_source);

    gfx::SubresourceRange dst_sr = {
        .mipLevel = narrow_cast<gfx::GfxIndex>(dst_mip_level),
        .mipLevelCount = 1,
        .baseArrayLayer = narrow_cast<gfx::GfxIndex>(dst_array_slice),
        .layerCount = 1,
    };
    gfx::SubresourceRange src_sr = {
        .mipLevel = narrow_cast<gfx::GfxIndex>(src_mip_level),
        .mipLevelCount = 1,
        .baseArrayLayer = narrow_cast<gfx::GfxIndex>(src_array_slice),
        .layerCount = 1,
    };

    get_gfx_resource_command_encoder()->resolveResource(
        src->gfx_texture_resource(),
        gfx::ResourceState::ResolveSource,
        src_sr,
        dst->gfx_texture_resource(),
        gfx::ResourceState::ResolveDestination,
        dst_sr
    );
}

ComputePassEncoder CommandStream::begin_compute_pass()
{
    KALI_CHECK(!m_pass_open, "CommandStream already has an active pass encoder");

    create_command_buffer();

    if (m_active_encoder != EncoderType::compute) {
        end_current_encoder();
        m_gfx_command_encoder = m_command_buffer->gfx_command_buffer()->encodeComputeCommands();
        m_active_encoder = EncoderType::compute;
    }

    m_pass_open = true;
    return ComputePassEncoder(this, (static_cast<gfx::IComputeCommandEncoder*>(m_gfx_command_encoder.get())));
}

void CommandStream::begin_debug_event(const char* name, float3 color)
{
    get_gfx_resource_command_encoder()->beginDebugEvent(name, &color.x);
}

void CommandStream::end_debug_event()
{
    get_gfx_resource_command_encoder()->endDebugEvent();
}

std::string CommandStream::to_string() const
{
    return fmt::format(
        "CommandStream(\n"
        "  device={}\n"
        "  command_queue={}\n"
        ")",
        m_device,
        m_command_queue
    );
}

void CommandStream::end_pass()
{
    KALI_ASSERT(m_pass_open);
    m_pass_open = false;
    end_current_encoder();
}

void CommandStream::create_command_buffer()
{
    if (!m_command_buffer) {
        m_command_buffer = m_device->create_command_buffer();
        m_command_buffer->break_strong_reference_to_device();
    }
}

gfx::IResourceCommandEncoder* CommandStream::get_gfx_resource_command_encoder()
{
    create_command_buffer();

    if (m_active_encoder == EncoderType::none) {
        m_gfx_command_encoder = m_command_buffer->gfx_command_buffer()->encodeResourceCommands();
        m_active_encoder = EncoderType::resource;
    }
    return static_cast<gfx::IResourceCommandEncoder*>(m_gfx_command_encoder.get());
}

void CommandStream::end_current_encoder()
{
    if (m_active_encoder != EncoderType::none) {
        m_gfx_command_encoder->endEncoding();
        m_active_encoder = EncoderType::none;
    }
}


} // namespace kali
