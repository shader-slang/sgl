#include "command.h"

#include "kali/device/helpers.h"
#include "kali/device/device.h"
#include "kali/device/fence.h"
#include "kali/device/resource.h"
#include "kali/device/pipeline.h"
#include "kali/device/raytracing.h"
#include "kali/device/shader_object.h"

#include "kali/core/short_vector.h"

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

    gfx::IShaderObject* gfx_shader_object;
    SLANG_CALL(m_gfx_compute_command_encoder->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object));
    return make_ref<TransientShaderObject>(gfx_shader_object, m_command_stream);
}

void ComputePassEncoder::bind_pipeline(const ComputePipelineState* pipeline, const ShaderObject* shader_object)
{
    KALI_CHECK_NOT_NULL(pipeline);
    KALI_CHECK_NOT_NULL(shader_object);

    SLANG_CALL(m_gfx_compute_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
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

void CommandStream::upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data)
{
    KALI_ASSERT(buffer);
    KALI_ASSERT_LE(offset + size, buffer->size());
    KALI_ASSERT(data);

    get_gfx_resource_command_encoder()
        ->uploadBufferData(buffer->gfx_buffer_resource(), offset, size, const_cast<void*>(data));
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
