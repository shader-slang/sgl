// SPDX-License-Identifier: Apache-2.0

#include "command.h"

#include "kali/device/helpers.h"
#include "kali/device/device.h"
#include "kali/device/fence.h"
#include "kali/device/resource.h"
#include "kali/device/query.h"
#include "kali/device/pipeline.h"
#include "kali/device/raytracing.h"
#include "kali/device/shader_object.h"
#include "kali/device/framebuffer.h"
#include "kali/device/native_handle_traits.h"
#include "kali/device/cuda_utils.h"
#include "kali/device/cuda_interop.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/print.h"

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

#if KALI_HAS_CUDA
    if (m_device->supports_cuda_interop()) {
        m_cuda_fence = m_device->create_fence({.shared = true});
        m_cuda_fence->break_strong_reference_to_device();
        m_cuda_semaphore = make_ref<cuda::ExternalSemaphore>(m_cuda_fence);
    }
#endif
}

void CommandQueue::submit(const CommandBuffer* command_buffer)
{
    KALI_CHECK_NOT_NULL(command_buffer);
#if KALI_HAS_CUDA
    if (m_device->supports_cuda_interop())
        handle_copy_from_cuda(command_buffer);
#endif
    m_gfx_command_queue->executeCommandBuffer(command_buffer->gfx_command_buffer());
#if KALI_HAS_CUDA
    if (m_device->supports_cuda_interop())
        handle_copy_to_cuda(command_buffer);
#endif
}

void CommandQueue::submit(std::span<const CommandBuffer*> command_buffers)
{
    KALI_UNUSED(command_buffers);
    KALI_UNIMPLEMENTED();
    // m_gfx_command_queue->executeCommandBuffers(
    //     narrow_cast<gfx::GfxCount>(command_buffers.size()),
    //     reinterpret_cast<gfx::ICommandBuffer*>(command_buffers.data())
    // );
}

void CommandQueue::submit_and_wait(const CommandBuffer* command_buffer)
{
    submit(command_buffer);
    wait();
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

#if KALI_HAS_CUDA
void CommandQueue::wait_for_cuda(void* cuda_stream)
{
    KALI_CHECK(m_device->supports_cuda_interop(), "Device does not support CUDA interop");
    m_cuda_semaphore->wait_for_cuda(this, static_cast<CUstream>(cuda_stream));
}

void CommandQueue::wait_for_device(void* cuda_stream)
{
    KALI_CHECK(m_device->supports_cuda_interop(), "Device does not support CUDA interop");
    m_cuda_semaphore->wait_for_device(this, static_cast<CUstream>(cuda_stream));
}
#endif // KALI_HAS_CUDA


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
        "  device = {},\n",
        "  type = {}\n",
        ")",
        m_device,
        m_desc.type
    );
}

#if KALI_HAS_CUDA
void CommandQueue::handle_copy_from_cuda(const CommandBuffer* command_buffer)
{
    void* cuda_stream = 0; // TODO

    if (command_buffer->m_cuda_interop_buffers.empty())
        return;

    for (const auto& buffer : command_buffer->m_cuda_interop_buffers)
        buffer->copy_from_cuda(cuda_stream);

    wait_for_cuda(cuda_stream);
}

void CommandQueue::handle_copy_to_cuda(const CommandBuffer* command_buffer)
{
    void* cuda_stream = 0; // TODO

    if (command_buffer->m_cuda_interop_buffers.empty())
        return;

    wait_for_device(cuda_stream);

    for (const auto& buffer : command_buffer->m_cuda_interop_buffers)
        if (buffer->is_uav())
            buffer->copy_to_cuda(cuda_stream);
}
#endif

// ----------------------------------------------------------------------------
// ComputeCommandEncoder
// ----------------------------------------------------------------------------

ComputeCommandEncoder::~ComputeCommandEncoder()
{
    end();
}

void ComputeCommandEncoder::end()
{
    if (m_command_buffer) {
        m_command_buffer->end_encoder();
        m_command_buffer = nullptr;
    }
}

ref<TransientShaderObject> ComputeCommandEncoder::bind_pipeline(const ComputePipeline* pipeline)
{
    KALI_CHECK_NOT_NULL(pipeline);

    m_bound_pipeline = pipeline;
    gfx::IShaderObject* gfx_shader_object;
    SLANG_CALL(m_gfx_compute_command_encoder->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object));
    ref<TransientShaderObject> transient_shader_object
        = make_ref<TransientShaderObject>(ref<Device>(m_command_buffer->device()), gfx_shader_object, m_command_buffer);
    if (m_command_buffer->device()->debug_printer())
        m_command_buffer->device()->debug_printer()->bind(ShaderCursor(transient_shader_object));
    m_bound_shader_object = transient_shader_object;
    return transient_shader_object;
}

void ComputeCommandEncoder::bind_pipeline(const ComputePipeline* pipeline, const ShaderObject* shader_object)
{
    KALI_CHECK_NOT_NULL(pipeline);
    KALI_CHECK_NOT_NULL(shader_object);

    m_bound_pipeline = pipeline;
    // TODO we should probably take shader_object by ref<const ShaderObject>
    // alternatively we could process CUDA buffers at bind time
    m_bound_shader_object = ref<const ShaderObject>(shader_object);
    SLANG_CALL(m_gfx_compute_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void ComputeCommandEncoder::dispatch(uint3 thread_count)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

    uint3 thread_group_size = m_bound_pipeline->thread_group_size();
    uint3 thread_group_count{
        div_round_up(thread_count.x, thread_group_size.x),
        div_round_up(thread_count.y, thread_group_size.y),
        div_round_up(thread_count.z, thread_group_size.z)};
    dispatch_thread_groups(thread_group_count);
}

void ComputeCommandEncoder::dispatch_thread_groups(uint3 thread_group_count)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

#if KALI_HAS_CUDA
    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);
#endif

    SLANG_CALL(
        m_gfx_compute_command_encoder->dispatchCompute(thread_group_count.x, thread_group_count.y, thread_group_count.z)
    );
}

void ComputeCommandEncoder::dispatch_thread_groups_indirect(const Buffer* cmd_buffer, DeviceOffset offset)
{
    KALI_CHECK_NOT_NULL(cmd_buffer);
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

    SLANG_CALL(m_gfx_compute_command_encoder->dispatchComputeIndirect(cmd_buffer->gfx_buffer_resource(), offset));
}

// ----------------------------------------------------------------------------
// RenderCommandEncoder
// ----------------------------------------------------------------------------

RenderCommandEncoder::~RenderCommandEncoder()
{
    end();
}

void RenderCommandEncoder::end()
{
    if (m_command_buffer) {
        m_command_buffer->end_encoder();
        m_command_buffer = nullptr;
    }
}

ref<TransientShaderObject> RenderCommandEncoder::bind_pipeline(const GraphicsPipeline* pipeline)
{
    KALI_CHECK_NOT_NULL(pipeline);

    m_bound_pipeline = pipeline;
    gfx::IShaderObject* gfx_shader_object;
    SLANG_CALL(m_gfx_render_command_encoder->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object));
    ref<TransientShaderObject> transient_shader_object
        = make_ref<TransientShaderObject>(ref<Device>(m_command_buffer->device()), gfx_shader_object, m_command_buffer);
    if (m_command_buffer->device()->debug_printer())
        m_command_buffer->device()->debug_printer()->bind(ShaderCursor(transient_shader_object));
    m_bound_shader_object = transient_shader_object;
    return transient_shader_object;
}

void RenderCommandEncoder::bind_pipeline(const GraphicsPipeline* pipeline, const ShaderObject* shader_object)
{
    KALI_CHECK_NOT_NULL(pipeline);
    KALI_CHECK_NOT_NULL(shader_object);

    m_bound_pipeline = pipeline;
    m_bound_shader_object = ref<const ShaderObject>(shader_object);
    SLANG_CALL(m_gfx_render_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void RenderCommandEncoder::set_viewports(std::span<Viewport> viewports)
{
    m_gfx_render_command_encoder->setViewports(
        narrow_cast<gfx::GfxCount>(viewports.size()),
        reinterpret_cast<const gfx::Viewport*>(viewports.data())
    );
}

void RenderCommandEncoder::set_scissor_rects(std::span<ScissorRect> scissor_rects)
{
    m_gfx_render_command_encoder->setScissorRects(
        narrow_cast<gfx::GfxCount>(scissor_rects.size()),
        reinterpret_cast<const gfx::ScissorRect*>(scissor_rects.data())
    );
}

void RenderCommandEncoder::set_viewport_and_scissor_rect(const Viewport& viewport)
{
    m_gfx_render_command_encoder->setViewportAndScissor(reinterpret_cast<const gfx::Viewport&>(viewport));
}

void RenderCommandEncoder::set_primitive_topology(PrimitiveTopology topology)
{
    m_gfx_render_command_encoder->setPrimitiveTopology(static_cast<gfx::PrimitiveTopology>(topology));
}

void RenderCommandEncoder::set_stencil_reference(uint32_t reference_value)
{
    m_gfx_render_command_encoder->setStencilReference(reference_value);
}

void RenderCommandEncoder::set_vertex_buffers(uint32_t start_slot, std::span<Slot> slots)
{
    short_vector<gfx::IBufferResource*, 16> gfx_buffers(slots.size(), nullptr);
    short_vector<gfx::Offset, 16> gfx_offsets(slots.size(), 0);
    for (size_t i = 0; i < slots.size(); i++) {
        gfx_buffers[i] = slots[i].buffer->gfx_buffer_resource();
        gfx_offsets[i] = slots[i].offset;
    }
    m_gfx_render_command_encoder->setVertexBuffers(
        start_slot,
        narrow_cast<gfx::GfxCount>(slots.size()),
        gfx_buffers.data(),
        gfx_offsets.data()
    );
}

void RenderCommandEncoder::set_vertex_buffer(uint32_t slot, const Buffer* buffer, DeviceOffset offset)
{
    gfx::IBufferResource* gfx_buffer = buffer->gfx_buffer_resource();
    m_gfx_render_command_encoder->setVertexBuffer(slot, gfx_buffer, offset);
}

void RenderCommandEncoder::set_index_buffer(const Buffer* buffer, Format index_format, DeviceOffset offset)
{
    m_gfx_render_command_encoder
        ->setIndexBuffer(buffer->gfx_buffer_resource(), static_cast<gfx::Format>(index_format), offset);
}

void RenderCommandEncoder::draw(uint32_t vertex_count, uint32_t start_vertex)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

#if KALI_HAS_CUDA
    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);
#endif

    SLANG_CALL(m_gfx_render_command_encoder->draw(vertex_count, start_vertex));
}

void RenderCommandEncoder::draw_indexed(uint32_t index_count, uint32_t start_index, uint32_t base_vertex)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

#if KALI_HAS_CUDA
    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);
#endif

    SLANG_CALL(m_gfx_render_command_encoder->drawIndexed(index_count, start_index, base_vertex));
}

void RenderCommandEncoder::draw_instanced(
    uint32_t vertex_count,
    uint32_t instance_count,
    uint32_t start_vertex,
    uint32_t start_instance
)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

#if KALI_HAS_CUDA
    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);
#endif

    SLANG_CALL(m_gfx_render_command_encoder->drawInstanced(vertex_count, instance_count, start_vertex, start_instance));
}

void RenderCommandEncoder::draw_indexed_instanced(
    uint32_t index_count,
    uint32_t instance_count,
    uint32_t start_index,
    uint32_t base_vertex,
    uint32_t start_instance
)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

#if KALI_HAS_CUDA
    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);
#endif

    SLANG_CALL(m_gfx_render_command_encoder
                   ->drawIndexedInstanced(index_count, instance_count, start_index, base_vertex, start_instance));
}

void RenderCommandEncoder::draw_indirect(
    uint32_t max_draw_count,
    const Buffer* arg_buffer,
    DeviceOffset arg_offset,
    const Buffer* count_buffer,
    DeviceOffset count_offset
)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

#if KALI_HAS_CUDA
    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);
#endif

    SLANG_CALL(m_gfx_render_command_encoder->drawIndirect(
        max_draw_count,
        arg_buffer->gfx_buffer_resource(),
        arg_offset,
        count_buffer ? count_buffer->gfx_buffer_resource() : nullptr,
        count_offset
    ));
}

void RenderCommandEncoder::draw_indexed_indirect(
    uint32_t max_draw_count,
    const Buffer* arg_buffer,
    DeviceOffset arg_offset,
    const Buffer* count_buffer,
    DeviceOffset count_offset
)
{
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

#if KALI_HAS_CUDA
    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);
#endif

    SLANG_CALL(m_gfx_render_command_encoder->drawIndexedIndirect(
        max_draw_count,
        arg_buffer->gfx_buffer_resource(),
        arg_offset,
        count_buffer ? count_buffer->gfx_buffer_resource() : nullptr,
        count_offset
    ));
}

// ----------------------------------------------------------------------------
// RayTracingCommandEncoder
// ----------------------------------------------------------------------------

RayTracingCommandEncoder::~RayTracingCommandEncoder()
{
    end();
}

void RayTracingCommandEncoder::end()
{
    if (m_command_buffer) {
        m_command_buffer->end_encoder();
        m_command_buffer = nullptr;
    }
}

ref<TransientShaderObject> RayTracingCommandEncoder::bind_pipeline(const RayTracingPipeline* pipeline)
{
    KALI_CHECK_NOT_NULL(pipeline);

    m_bound_pipeline = pipeline;
    gfx::IShaderObject* gfx_shader_object;
    // TODO gfx bindPipeline should return Result (fix in slang/gfx)
    m_gfx_ray_tracing_command_encoder->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object);
    ref<TransientShaderObject> transient_shader_object
        = make_ref<TransientShaderObject>(ref<Device>(m_command_buffer->device()), gfx_shader_object, m_command_buffer);
    if (m_command_buffer->device()->debug_printer())
        m_command_buffer->device()->debug_printer()->bind(ShaderCursor(transient_shader_object));
    m_bound_shader_object = transient_shader_object;
    return transient_shader_object;
}

void RayTracingCommandEncoder::bind_pipeline(const RayTracingPipeline* pipeline, const ShaderObject* shader_object)
{
    KALI_CHECK_NOT_NULL(pipeline);
    KALI_CHECK_NOT_NULL(shader_object);

    m_bound_pipeline = pipeline;
    m_bound_shader_object = ref<const ShaderObject>(shader_object);
    SLANG_CALL(m_gfx_ray_tracing_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void RayTracingCommandEncoder::dispatch_rays(
    uint32_t ray_gen_shader_index,
    const ShaderTable* shader_table,
    uint3 dimensions
)
{
    KALI_CHECK_NOT_NULL(shader_table);
    KALI_CHECK(m_bound_pipeline, "No pipeline bound");

#if KALI_HAS_CUDA
    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);
#endif

    SLANG_CALL(m_gfx_ray_tracing_command_encoder->dispatchRays(
        narrow_cast<gfx::GfxIndex>(ray_gen_shader_index),
        shader_table->gfx_shader_table(),
        narrow_cast<gfx::GfxCount>(dimensions.x),
        narrow_cast<gfx::GfxCount>(dimensions.y),
        narrow_cast<gfx::GfxCount>(dimensions.z)
    ));
}

void RayTracingCommandEncoder::build_acceleration_structure(
    const AccelerationStructureBuildDesc& desc,
    std::span<AccelerationStructureQueryDesc> queries
)
{
    gfx::IAccelerationStructure::BuildDesc gfx_build_desc{
        .inputs = reinterpret_cast<const gfx::IAccelerationStructure::BuildInputs&>(desc.inputs),
        .source = desc.src ? desc.src->gfx_acceleration_structure() : nullptr,
        .dest = desc.dst ? desc.dst->gfx_acceleration_structure() : nullptr,
        .scratchData = desc.scratch_data,
    };

    short_vector<gfx::AccelerationStructureQueryDesc, 16> gfx_query_descs(queries.size(), {});
    for (size_t i = 0; i < queries.size(); i++) {
        gfx_query_descs[i] = {
            .queryType = static_cast<gfx::QueryType>(queries[i].query_type),
            .queryPool = queries[i].query_pool->gfx_query_pool(),
            .firstQueryIndex = narrow_cast<gfx::GfxIndex>(queries[i].first_query_index),
        };
    }

    m_gfx_ray_tracing_command_encoder->buildAccelerationStructure(
        gfx_build_desc,
        narrow_cast<gfx::GfxCount>(gfx_query_descs.size()),
        gfx_query_descs.data()
    );
}

void RayTracingCommandEncoder::copy_acceleration_structure(
    AccelerationStructure* dst,
    const AccelerationStructure* src,
    AccelerationStructureCopyMode mode
)
{
    KALI_CHECK_NOT_NULL(dst);
    KALI_CHECK_NOT_NULL(src);

    m_gfx_ray_tracing_command_encoder->copyAccelerationStructure(
        dst->gfx_acceleration_structure(),
        src->gfx_acceleration_structure(),
        static_cast<gfx::AccelerationStructureCopyMode>(mode)
    );
}

void RayTracingCommandEncoder::query_acceleration_structure_properties(
    std::span<const AccelerationStructure*> acceleration_structures,
    std::span<AccelerationStructureQueryDesc> queries
)
{
    short_vector<gfx::IAccelerationStructure*, 16> gfx_acceleration_structures(acceleration_structures.size(), nullptr);
    for (size_t i = 0; i < acceleration_structures.size(); i++)
        gfx_acceleration_structures[i] = acceleration_structures[i]->gfx_acceleration_structure();

    short_vector<gfx::AccelerationStructureQueryDesc, 16> gfx_queries(queries.size(), {});
    for (size_t i = 0; i < queries.size(); i++) {
        gfx_queries[i] = {
            .queryType = static_cast<gfx::QueryType>(queries[i].query_type),
            .queryPool = queries[i].query_pool->gfx_query_pool(),
            .firstQueryIndex = narrow_cast<gfx::GfxIndex>(queries[i].first_query_index),
        };
    }

    m_gfx_ray_tracing_command_encoder->queryAccelerationStructureProperties(
        narrow_cast<gfx::GfxCount>(gfx_acceleration_structures.size()),
        gfx_acceleration_structures.data(),
        narrow_cast<gfx::GfxCount>(gfx_queries.size()),
        gfx_queries.data()
    );
}

void RayTracingCommandEncoder::serialize_acceleration_structure(DeviceAddress dst, const AccelerationStructure* src)
{
    m_gfx_ray_tracing_command_encoder->serializeAccelerationStructure(dst, src->gfx_acceleration_structure());
}

void RayTracingCommandEncoder::deserialize_acceleration_structure(AccelerationStructure* dst, DeviceAddress src)
{
    m_gfx_ray_tracing_command_encoder->deserializeAccelerationStructure(dst->gfx_acceleration_structure(), src);
}

// ----------------------------------------------------------------------------
// CommandBuffer
// ----------------------------------------------------------------------------

CommandBuffer::CommandBuffer(ref<Device> device, Slang::ComPtr<gfx::ICommandBuffer> gfx_command_buffer)
    : DeviceResource(std::move(device))
    , m_gfx_command_buffer(std::move(gfx_command_buffer))
{
}

void CommandBuffer::close()
{
    if (!m_open)
        return;

    if (m_encoder_open)
        KALI_THROW("Cannot close command buffer with an open encoder.");

    end_current_gfx_encoder();
    m_gfx_command_buffer->close();
    m_open = false;
}

void CommandBuffer::submit()
{
    close();
    m_device->graphics_queue()->submit(this);
}

void CommandBuffer::write_timestamp(QueryPool* query_pool, uint32_t index)
{
    KALI_CHECK(m_open, "Command buffer is closed");
    KALI_CHECK_NOT_NULL(query_pool);
    KALI_CHECK_LE(index, query_pool->desc().count);

    get_gfx_resource_command_encoder()->writeTimestamp(query_pool->gfx_query_pool(), index);
}

void CommandBuffer::resolve_query(
    QueryPool* query_pool,
    uint32_t index,
    uint32_t count,
    Buffer* buffer,
    DeviceOffset offset
)
{
    KALI_CHECK_NOT_NULL(query_pool);
    KALI_CHECK_NOT_NULL(buffer);
    KALI_CHECK_LE(index + count, query_pool->desc().count);
    KALI_CHECK_LE(offset + count * sizeof(uint64_t), buffer->size());

    buffer_barrier(buffer, ResourceState::resolve_destination);

    get_gfx_resource_command_encoder()
        ->resolveQuery(query_pool->gfx_query_pool(), index, count, buffer->gfx_buffer_resource(), offset);
}

bool CommandBuffer::buffer_barrier(const Buffer* buffer, ResourceState new_state)
{
    KALI_CHECK_NOT_NULL(buffer);

    ResourceState current_state = buffer->state_tracker().global_state();
    if (new_state == current_state)
        return false;

    buffer_barrier(buffer, current_state, new_state);
    buffer->state_tracker().set_global_state(new_state);
    return true;
}

bool CommandBuffer::texture_barrier(const Texture* texture, ResourceState new_state)
{
    KALI_CHECK_NOT_NULL(texture);

    ResourceState current_state = texture->state_tracker().global_state();
    if (new_state == current_state)
        return false;

    texture_barrier(texture, current_state, new_state);
    texture->state_tracker().set_global_state(new_state);
    return true;
}

bool CommandBuffer::texture_subresource_barrier(
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

bool CommandBuffer::resource_barrier(const Resource* resource, ResourceState new_state)
{
    KALI_CHECK_NOT_NULL(resource);

    if (resource->type() == ResourceType::buffer) {
        return buffer_barrier(static_cast<const Buffer*>(resource), new_state);
    } else {
        return texture_barrier(static_cast<const Texture*>(resource), new_state);
    }
}

void CommandBuffer::uav_barrier(const Resource* resource)
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

void CommandBuffer::buffer_barrier(std::span<const Buffer*> buffers, ResourceState old_state, ResourceState new_state)
{
    short_vector<gfx::IBufferResource*, 16> gfx_buffers(buffers.size(), nullptr);
    for (size_t i = 0; i < buffers.size(); i++)
        gfx_buffers[i] = buffers[i]->gfx_buffer_resource();
    get_gfx_resource_command_encoder()->bufferBarrier(
        narrow_cast<gfx::GfxCount>(buffers.size()),
        gfx_buffers.data(),
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void CommandBuffer::buffer_barrier(const Buffer* buffer, ResourceState old_state, ResourceState new_state)
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

void CommandBuffer::texture_barrier(
    std::span<const Texture*> textures,
    ResourceState old_state,
    ResourceState new_state
)
{
    short_vector<gfx::ITextureResource*, 16> gfx_textures(textures.size(), nullptr);
    for (size_t i = 0; i < textures.size(); i++)
        gfx_textures[i] = textures[i]->gfx_texture_resource();
    get_gfx_resource_command_encoder()->textureBarrier(
        narrow_cast<gfx::GfxCount>(textures.size()),
        gfx_textures.data(),
        static_cast<gfx::ResourceState>(old_state),
        static_cast<gfx::ResourceState>(new_state)
    );
}

void CommandBuffer::texture_barrier(const Texture* texture, ResourceState old_state, ResourceState new_state)
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

void CommandBuffer::texture_subresource_barrier(
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

void CommandBuffer::clear_resource_view(ResourceView* resource_view, float4 clear_value)
{
    KALI_CHECK_NOT_NULL(resource_view);

    switch (resource_view->type()) {
    case ResourceViewType::render_target:
        resource_barrier(resource_view->resource(), ResourceState::render_target);
        break;
    case ResourceViewType::shader_resource:
        resource_barrier(resource_view->resource(), ResourceState::shader_resource);
        break;
    case ResourceViewType::unordered_access:
        resource_barrier(resource_view->resource(), ResourceState::unordered_access);
        break;
    default:
        KALI_THROW("Invalid resource view type");
    }

    gfx::ClearValue gfx_clear_value = {};
    gfx_clear_value.color.floatValues[0] = clear_value[0];
    gfx_clear_value.color.floatValues[1] = clear_value[1];
    gfx_clear_value.color.floatValues[2] = clear_value[2];
    gfx_clear_value.color.floatValues[3] = clear_value[3];
    get_gfx_resource_command_encoder()->clearResourceView(
        resource_view->gfx_resource_view(),
        &gfx_clear_value,
        gfx::ClearResourceViewFlags::FloatClearValues
    );
}

void CommandBuffer::clear_resource_view(ResourceView* resource_view, uint4 clear_value)
{
    KALI_CHECK_NOT_NULL(resource_view);

    switch (resource_view->type()) {
    case ResourceViewType::render_target:
        resource_barrier(resource_view->resource(), ResourceState::render_target);
        break;
    case ResourceViewType::shader_resource:
        resource_barrier(resource_view->resource(), ResourceState::shader_resource);
        break;
    case ResourceViewType::unordered_access:
        resource_barrier(resource_view->resource(), ResourceState::unordered_access);
        break;
    default:
        KALI_THROW("Invalid resource view type");
    }

    gfx::ClearValue gfx_clear_value = {};
    gfx_clear_value.color.uintValues[0] = clear_value[0];
    gfx_clear_value.color.uintValues[1] = clear_value[1];
    gfx_clear_value.color.uintValues[2] = clear_value[2];
    gfx_clear_value.color.uintValues[3] = clear_value[3];
    get_gfx_resource_command_encoder()
        ->clearResourceView(resource_view->gfx_resource_view(), &gfx_clear_value, gfx::ClearResourceViewFlags::None);
}

void CommandBuffer::clear_resource_view(
    ResourceView* resource_view,
    float depth_value,
    uint32_t stencil_value,
    bool clear_depth,
    bool clear_stencil
)
{
    KALI_CHECK_NOT_NULL(resource_view);

    switch (resource_view->type()) {
    case ResourceViewType::depth_stencil:
        resource_barrier(resource_view->resource(), ResourceState::depth_write);
        break;
    default:
        KALI_THROW("Invalid resource view type");
    }

    gfx::ClearValue gfx_clear_value = {};
    gfx_clear_value.depthStencil.depth = depth_value;
    gfx_clear_value.depthStencil.stencil = stencil_value;
    uint32_t gfx_flags = gfx::ClearResourceViewFlags::None;
    if (clear_depth)
        gfx_flags |= gfx::ClearResourceViewFlags::ClearDepth;
    if (clear_stencil)
        gfx_flags |= gfx::ClearResourceViewFlags::ClearStencil;
    get_gfx_resource_command_encoder()->clearResourceView(
        resource_view->gfx_resource_view(),
        &gfx_clear_value,
        gfx::ClearResourceViewFlags::Enum(gfx_flags)
    );
}

void CommandBuffer::clear_texture(Texture* texture, float4 clear_value)
{
    KALI_CHECK_NOT_NULL(texture);
    FormatType format_type = get_format_info(texture->format()).type;
    KALI_CHECK(
        format_type != FormatType::sint && format_type != FormatType::uint && format_type != FormatType::typeless,
        "Texture format must be floating point compatible"
    );

    if (is_set(texture->desc().usage, ResourceUsage::unordered_access)) {
        clear_resource_view(texture->get_uav(0), clear_value);
    } else if (is_set(texture->desc().usage, ResourceUsage::render_target)) {
        clear_resource_view(texture->get_rtv(0), clear_value);
    } else {
        KALI_THROW("Texture must be either unordered access or render target");
    }
}

void CommandBuffer::clear_texture(Texture* texture, uint4 clear_value)
{
    KALI_CHECK_NOT_NULL(texture);
    FormatType format_type = get_format_info(texture->format()).type;
    KALI_CHECK(
        format_type == FormatType::sint || format_type == FormatType::uint || format_type == FormatType::typeless,
        "Texture format must be integer compatible"
    );

    if (is_set(texture->desc().usage, ResourceUsage::unordered_access)) {
        clear_resource_view(texture->get_uav(0), clear_value);
    } else if (is_set(texture->desc().usage, ResourceUsage::render_target)) {
        clear_resource_view(texture->get_rtv(0), clear_value);
    } else {
        KALI_THROW("Texture must be either unordered access or render target");
    }
}

void CommandBuffer::copy_resource(Resource* dst, const Resource* src)
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

void CommandBuffer::copy_buffer_region(
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

void CommandBuffer::copy_texture_region(
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

void CommandBuffer::copy_texture_to_buffer(
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

void CommandBuffer::upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data)
{
    KALI_CHECK_NOT_NULL(buffer);
    KALI_CHECK(offset + size <= buffer->size(), "Buffer upload is out of bounds");
    KALI_CHECK_NOT_NULL(data);

    buffer_barrier(buffer, ResourceState::copy_destination);

    get_gfx_resource_command_encoder()
        ->uploadBufferData(buffer->gfx_buffer_resource(), offset, size, const_cast<void*>(data));
}

void CommandBuffer::upload_texture_data(Texture* texture, uint32_t subresource, const void* data)
{
    KALI_CHECK_NOT_NULL(texture);
    KALI_CHECK_LT(subresource, texture->subresource_count());
    KALI_CHECK_NOT_NULL(data);

    texture_barrier(texture, ResourceState::copy_destination);

    uint3 dimensions = texture->get_mip_dimensions(texture->get_subresource_mip_level(subresource));
    SubresourceLayout layout = texture->get_subresource_layout(subresource);

    gfx::ITextureResource::SubresourceData gfx_subresource_data = {
        .data = const_cast<void*>(data),
        .strideY = layout.row_size_aligned,
        .strideZ = layout.row_size_aligned * layout.row_count,
    };

    gfx::SubresourceRange sr = detail::gfx_subresource_range(texture, subresource);
    get_gfx_resource_command_encoder()->uploadTextureData(
        texture->gfx_texture_resource(),
        sr,
        gfx::ITextureResource::Offset3D(0, 0, 0),
        gfx::ITextureResource::Extents{
            narrow_cast<gfx::GfxCount>(dimensions.x),
            narrow_cast<gfx::GfxCount>(dimensions.y),
            narrow_cast<gfx::GfxCount>(dimensions.z),
        },
        &gfx_subresource_data,
        1
    );
}

void CommandBuffer::resolve_texture(Texture* dst, const Texture* src)
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

void CommandBuffer::resolve_subresource(
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
    KALI_CHECK(
        all(dst->get_mip_dimensions(dst_mip_level) == src->get_mip_dimensions(src_mip_level)),
        "Source and destination textures must have the same dimensions."
    );

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

ComputeCommandEncoder CommandBuffer::encode_compute_commands()
{
    KALI_CHECK(!m_encoder_open, "CommandBuffer already has an active encoder");

    if (m_active_gfx_encoder != EncoderType::compute) {
        end_current_gfx_encoder();
        m_gfx_command_encoder = m_gfx_command_buffer->encodeComputeCommands();
        m_active_gfx_encoder = EncoderType::compute;
    }

    m_encoder_open = true;
    return ComputeCommandEncoder(this, (static_cast<gfx::IComputeCommandEncoder*>(m_gfx_command_encoder.get())));
}

RenderCommandEncoder CommandBuffer::encode_render_commands(Framebuffer* framebuffer)
{
    KALI_CHECK(!m_encoder_open, "CommandBuffer already has an active encoder");
    KALI_CHECK_NOT_NULL(framebuffer);

    // Set the render target and depth-stencil barriers.
    for (const auto& render_target : framebuffer->desc().render_targets)
        texture_barrier(render_target.texture, ResourceState::render_target);
    if (framebuffer->desc().depth_stencil)
        texture_barrier(framebuffer->desc().depth_stencil->texture, ResourceState::depth_write);

    if (m_active_gfx_encoder != EncoderType::render) {
        end_current_gfx_encoder();
        m_gfx_command_encoder = m_gfx_command_buffer->encodeRenderCommands(
            framebuffer->gfx_render_pass_layout(),
            framebuffer->gfx_framebuffer()
        );
        m_active_gfx_encoder = EncoderType::render;
    }

    m_encoder_open = true;
    return RenderCommandEncoder(this, (static_cast<gfx::IRenderCommandEncoder*>(m_gfx_command_encoder.get())));
}

RayTracingCommandEncoder CommandBuffer::encode_ray_tracing_commands()
{
    KALI_CHECK(!m_encoder_open, "CommandBuffer already has an active encoder");

    if (m_active_gfx_encoder != EncoderType::raytracing) {
        end_current_gfx_encoder();
        m_gfx_command_encoder = m_gfx_command_buffer->encodeRayTracingCommands();
        m_active_gfx_encoder = EncoderType::raytracing;
    }

    m_encoder_open = true;
    return RayTracingCommandEncoder(this, (static_cast<gfx::IRayTracingCommandEncoder*>(m_gfx_command_encoder.get())));
}

void CommandBuffer::begin_debug_event(const char* name, float3 color)
{
    get_gfx_resource_command_encoder()->beginDebugEvent(name, &color.x);
}

void CommandBuffer::end_debug_event()
{
    get_gfx_resource_command_encoder()->endDebugEvent();
}

std::string CommandBuffer::to_string() const
{
    return fmt::format(
        "CommandBuffer(\n"
        "  device = {}\n"
        ")",
        m_device
    );
}

void CommandBuffer::end_encoder()
{
    KALI_ASSERT(m_encoder_open);
    m_encoder_open = false;
    end_current_gfx_encoder();
}

gfx::IResourceCommandEncoder* CommandBuffer::get_gfx_resource_command_encoder()
{
    if (m_active_gfx_encoder == EncoderType::none) {
        m_gfx_command_encoder = m_gfx_command_buffer->encodeResourceCommands();
        m_active_gfx_encoder = EncoderType::resource;
    }
    return static_cast<gfx::IResourceCommandEncoder*>(m_gfx_command_encoder.get());
}

void CommandBuffer::end_current_gfx_encoder()
{
    if (m_active_gfx_encoder != EncoderType::none) {
        m_gfx_command_encoder->endEncoding();
        m_gfx_command_encoder = nullptr;
        m_active_gfx_encoder = EncoderType::none;
    }
}

} // namespace kali
