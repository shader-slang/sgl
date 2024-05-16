// SPDX-License-Identifier: Apache-2.0

#include "command.h"

#include "sgl/device/helpers.h"
#include "sgl/device/device.h"
#include "sgl/device/fence.h"
#include "sgl/device/resource.h"
#include "sgl/device/query.h"
#include "sgl/device/pipeline.h"
#include "sgl/device/raytracing.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/framebuffer.h"
#include "sgl/device/native_handle_traits.h"
#include "sgl/device/cuda_utils.h"
#include "sgl/device/cuda_interop.h"
#include "sgl/device/shader_cursor.h"
#include "sgl/device/print.h"
#include "sgl/device/blit.h"

#include "sgl/core/short_vector.h"
#include "sgl/core/maths.h"
#include "sgl/core/type_utils.h"

#include "sgl/math/vector.h"

namespace sgl {

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
    SGL_CHECK_NOT_NULL(pipeline);

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
    SGL_CHECK_NOT_NULL(pipeline);
    SGL_CHECK_NOT_NULL(shader_object);

    m_bound_pipeline = pipeline;
    // TODO we should probably take shader_object by ref<const ShaderObject>
    // alternatively we could process CUDA buffers at bind time
    m_bound_shader_object = ref<const ShaderObject>(shader_object);
    static_cast<const MutableShaderObject*>(shader_object)->set_resource_states(m_command_buffer);
    SLANG_CALL(m_gfx_compute_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void ComputeCommandEncoder::dispatch(uint3 thread_count)
{
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    uint3 thread_group_size = m_bound_pipeline->thread_group_size();
    uint3 thread_group_count{
        div_round_up(thread_count.x, thread_group_size.x),
        div_round_up(thread_count.y, thread_group_size.y),
        div_round_up(thread_count.z, thread_group_size.z)};
    dispatch_thread_groups(thread_group_count);
}

void ComputeCommandEncoder::dispatch_thread_groups(uint3 thread_group_count)
{
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

    SLANG_CALL(
        m_gfx_compute_command_encoder->dispatchCompute(thread_group_count.x, thread_group_count.y, thread_group_count.z)
    );
}

void ComputeCommandEncoder::dispatch_thread_groups_indirect(const Buffer* cmd_buffer, DeviceOffset offset)
{
    SGL_CHECK_NOT_NULL(cmd_buffer);
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

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
    SGL_CHECK_NOT_NULL(pipeline);

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
    SGL_CHECK_NOT_NULL(pipeline);
    SGL_CHECK_NOT_NULL(shader_object);

    m_bound_pipeline = pipeline;
    m_bound_shader_object = ref<const ShaderObject>(shader_object);
    static_cast<const MutableShaderObject*>(shader_object)->set_resource_states(m_command_buffer);
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
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

    SLANG_CALL(m_gfx_render_command_encoder->draw(vertex_count, start_vertex));
}

void RenderCommandEncoder::draw_indexed(uint32_t index_count, uint32_t start_index, uint32_t base_vertex)
{
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

    SLANG_CALL(m_gfx_render_command_encoder->drawIndexed(index_count, start_index, base_vertex));
}

void RenderCommandEncoder::draw_instanced(
    uint32_t vertex_count,
    uint32_t instance_count,
    uint32_t start_vertex,
    uint32_t start_instance
)
{
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

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
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

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
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

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
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

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
    SGL_CHECK_NOT_NULL(pipeline);

    m_bound_pipeline = pipeline;
    gfx::IShaderObject* gfx_shader_object;
    SLANG_CALL(m_gfx_ray_tracing_command_encoder->bindPipeline(pipeline->gfx_pipeline_state(), &gfx_shader_object));
    ref<TransientShaderObject> transient_shader_object
        = make_ref<TransientShaderObject>(ref<Device>(m_command_buffer->device()), gfx_shader_object, m_command_buffer);
    if (m_command_buffer->device()->debug_printer())
        m_command_buffer->device()->debug_printer()->bind(ShaderCursor(transient_shader_object));
    m_bound_shader_object = transient_shader_object;
    return transient_shader_object;
}

void RayTracingCommandEncoder::bind_pipeline(const RayTracingPipeline* pipeline, const ShaderObject* shader_object)
{
    SGL_CHECK_NOT_NULL(pipeline);
    SGL_CHECK_NOT_NULL(shader_object);

    m_bound_pipeline = pipeline;
    m_bound_shader_object = ref<const ShaderObject>(shader_object);
    static_cast<const MutableShaderObject*>(shader_object)->set_resource_states(m_command_buffer);
    SLANG_CALL(m_gfx_ray_tracing_command_encoder
                   ->bindPipelineWithRootObject(pipeline->gfx_pipeline_state(), shader_object->gfx_shader_object()));
}

void RayTracingCommandEncoder::dispatch_rays(
    uint32_t ray_gen_shader_index,
    const ShaderTable* shader_table,
    uint3 dimensions
)
{
    SGL_CHECK_NOT_NULL(shader_table);
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

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
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);

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

CommandBuffer::CommandBuffer(ref<Device> device)
    : DeviceResource(std::move(device))
{
    open();
}

CommandBuffer::~CommandBuffer()
{
    close();
}

void CommandBuffer::open()
{
    if (m_open)
        return;

    m_device->_set_open_command_buffer(this);

    m_gfx_transient_resource_heap = m_device->_get_or_create_transient_resource_heap();
    SLANG_CALL(m_gfx_transient_resource_heap->createCommandBuffer(m_gfx_command_buffer.writeRef()));
    m_open = true;
}

void CommandBuffer::close()
{
    if (!m_open)
        return;

    if (m_encoder_open)
        SGL_THROW("Cannot close command buffer with an open encoder.");

    m_device->_set_open_command_buffer(nullptr);

    end_current_gfx_encoder();
    m_gfx_command_buffer->close();
    m_open = false;
}

uint64_t CommandBuffer::submit(CommandQueueType queue)
{
    close();
    return m_device->submit_command_buffer(this, queue);
}

void CommandBuffer::write_timestamp(QueryPool* query_pool, uint32_t index)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(query_pool);
    SGL_CHECK_LE(index, query_pool->desc().count);

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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(query_pool);
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK_LE(index + count, query_pool->desc().count);
    SGL_CHECK_LE(offset + count * sizeof(uint64_t), buffer->size());

    set_buffer_state(buffer, ResourceState::resolve_destination);

    get_gfx_resource_command_encoder()
        ->resolveQuery(query_pool->gfx_query_pool(), index, count, buffer->gfx_buffer_resource(), offset);
}

bool CommandBuffer::set_resource_state(const Resource* resource, ResourceState new_state)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(resource);

    if (resource->type() == ResourceType::buffer) {
        return set_buffer_state(resource->as_buffer(), new_state);
    } else {
        return set_texture_state(resource->as_texture(), new_state);
    }
}

bool CommandBuffer::set_resource_state(const ResourceView* resource_view, ResourceState new_state)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(resource_view);

    if (resource_view->resource()->type() == ResourceType::buffer) {
        return set_buffer_state(resource_view->resource()->as_buffer(), new_state);
    } else {
        if (resource_view->all_subresources())
            return set_texture_state(resource_view->resource()->as_texture(), new_state);
        else
            return set_texture_subresource_state(
                resource_view->resource()->as_texture(),
                resource_view->desc().subresource_range,
                new_state
            );
    }
}

bool CommandBuffer::set_buffer_state(const Buffer* buffer, ResourceState new_state)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(buffer);

    ResourceStateTracker& state_tracker = buffer->state_tracker();
    SGL_ASSERT(state_tracker.has_global_state());
    ResourceState current_state = state_tracker.global_state();
    if (new_state == current_state)
        return false;

    state_tracker.set_global_state(new_state);

    get_gfx_resource_command_encoder()->bufferBarrier(
        buffer->gfx_buffer_resource(),
        static_cast<gfx::ResourceState>(current_state),
        static_cast<gfx::ResourceState>(new_state)
    );

    return true;
}

bool CommandBuffer::set_texture_state(const Texture* texture, ResourceState new_state)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(texture);

    ResourceStateTracker& state_tracker = texture->state_tracker();
    if (state_tracker.has_global_state()) {
        ResourceState current_state = state_tracker.global_state();
        if (new_state == current_state)
            return false;

        state_tracker.set_global_state(new_state);

        get_gfx_resource_command_encoder()->textureBarrier(
            texture->gfx_texture_resource(),
            static_cast<gfx::ResourceState>(current_state),
            static_cast<gfx::ResourceState>(new_state)
        );

        return true;
    } else {
        bool changed = false;
        for (uint32_t a = 0; a < texture->array_size(); ++a) {
            for (uint32_t m = 0; m < texture->mip_count(); ++m) {
                uint32_t subresource = texture->get_subresource_index(m, a);
                ResourceState old_state = state_tracker.subresource_state(subresource);
                if (old_state != new_state) {
                    get_gfx_resource_command_encoder()->textureSubresourceBarrier(
                        texture->gfx_texture_resource(),
                        {
                            .mipLevel = narrow_cast<gfx::GfxIndex>(m),
                            .mipLevelCount = 1,
                            .baseArrayLayer = narrow_cast<gfx::GfxIndex>(a),
                            .layerCount = 1,
                        },
                        static_cast<gfx::ResourceState>(old_state),
                        static_cast<gfx::ResourceState>(new_state)
                    );
                    changed = true;
                }
            }
        }
        state_tracker.set_global_state(new_state);
        return changed;
    }
}

bool CommandBuffer::set_texture_subresource_state(
    const Texture* texture,
    SubresourceRange range,
    ResourceState new_state
)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(texture);

    uint32_t mip_count = range.mip_count;
    if (mip_count == SubresourceRange::ALL)
        mip_count = texture->mip_count() - range.mip_level;
    uint32_t layer_count = range.layer_count;
    if (layer_count == SubresourceRange::ALL)
        layer_count = texture->array_size() - range.base_array_layer;

    // Set state on the entire resource if possible.
    bool entire_resource = (range.mip_level == 0) && (mip_count == texture->mip_count())
        && (range.base_array_layer == 0) && (layer_count == texture->array_size());
    if (entire_resource)
        return set_texture_state(texture, new_state);

    SGL_CHECK(range.mip_level + mip_count <= texture->mip_count(), "Invalid mip level range");
    SGL_CHECK(range.base_array_layer + layer_count <= texture->array_size(), "Invalid array layer range");

    // Change state on each subresource.
    ResourceStateTracker& state_tracker = texture->state_tracker();
    bool changed = false;
    for (uint32_t m = range.mip_level; m < range.mip_level + mip_count; ++m) {
        for (uint32_t a = range.base_array_layer; a < range.base_array_layer + layer_count; ++a) {
            uint32_t subresource = texture->get_subresource_index(m, a);
            ResourceState old_state = state_tracker.subresource_state(subresource);
            if (old_state != new_state) {
                state_tracker.set_subresource_state(subresource, new_state);
                get_gfx_resource_command_encoder()->textureSubresourceBarrier(
                    texture->gfx_texture_resource(),
                    {
                        .mipLevel = narrow_cast<gfx::GfxIndex>(m),
                        .mipLevelCount = 1,
                        .baseArrayLayer = narrow_cast<gfx::GfxIndex>(a),
                        .layerCount = 1,
                    },
                    static_cast<gfx::ResourceState>(old_state),
                    static_cast<gfx::ResourceState>(new_state)
                );
                changed = true;
            }
        }
    }
    return changed;
}

void CommandBuffer::uav_barrier(const Resource* resource)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(resource);

    if (set_resource_state(resource, ResourceState::unordered_access))
        return;

    if (resource->type() == ResourceType::buffer) {
        get_gfx_resource_command_encoder()->bufferBarrier(
            resource->as_buffer()->gfx_buffer_resource(),
            gfx::ResourceState::UnorderedAccess,
            gfx::ResourceState::UnorderedAccess
        );
    } else {
        get_gfx_resource_command_encoder()->textureBarrier(
            resource->as_texture()->gfx_texture_resource(),
            gfx::ResourceState::UnorderedAccess,
            gfx::ResourceState::UnorderedAccess
        );
    }
}

void CommandBuffer::clear_resource_view(ResourceView* resource_view, float4 clear_value)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(resource_view);

    switch (resource_view->type()) {
    case ResourceViewType::render_target:
        set_resource_state(resource_view, ResourceState::render_target);
        break;
    case ResourceViewType::shader_resource:
        set_resource_state(resource_view, ResourceState::shader_resource);
        break;
    case ResourceViewType::unordered_access:
        set_resource_state(resource_view, ResourceState::unordered_access);
        break;
    default:
        SGL_THROW("Invalid resource view type");
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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(resource_view);

    switch (resource_view->type()) {
    case ResourceViewType::render_target:
        set_resource_state(resource_view, ResourceState::render_target);
        break;
    case ResourceViewType::shader_resource:
        set_resource_state(resource_view, ResourceState::shader_resource);
        break;
    case ResourceViewType::unordered_access:
        set_resource_state(resource_view, ResourceState::unordered_access);
        break;
    default:
        SGL_THROW("Invalid resource view type");
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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(resource_view);

    switch (resource_view->type()) {
    case ResourceViewType::depth_stencil:
        set_resource_state(resource_view, ResourceState::depth_write);
        break;
    default:
        SGL_THROW("Invalid resource view type");
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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(texture);
    FormatType format_type = get_format_info(texture->format()).type;
    SGL_CHECK(
        format_type != FormatType::sint && format_type != FormatType::uint && format_type != FormatType::typeless,
        "Texture format must be floating point compatible"
    );

    if (is_set(texture->desc().usage, ResourceUsage::unordered_access)) {
        clear_resource_view(texture->get_uav(), clear_value);
    } else if (is_set(texture->desc().usage, ResourceUsage::render_target)) {
        clear_resource_view(texture->get_rtv(), clear_value);
    } else {
        SGL_THROW("Texture must be either unordered access or render target");
    }
}

void CommandBuffer::clear_texture(Texture* texture, uint4 clear_value)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(texture);
    FormatType format_type = get_format_info(texture->format()).type;
    SGL_CHECK(
        format_type == FormatType::sint || format_type == FormatType::uint || format_type == FormatType::typeless,
        "Texture format must be integer compatible"
    );

    if (is_set(texture->desc().usage, ResourceUsage::unordered_access)) {
        clear_resource_view(texture->get_uav(), clear_value);
    } else if (is_set(texture->desc().usage, ResourceUsage::render_target)) {
        clear_resource_view(texture->get_rtv(), clear_value);
    } else {
        SGL_THROW("Texture must be either unordered access or render target");
    }
}

void CommandBuffer::copy_resource(Resource* dst, const Resource* src)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK(dst->type() == src->type(), "Resources must be of the same type");

    set_resource_state(dst, ResourceState::copy_destination);
    set_resource_state(src, ResourceState::copy_source);

    if (dst->type() == ResourceType::buffer) {
        const Buffer* dst_buffer = dst->as_buffer();
        const Buffer* src_buffer = src->as_buffer();

        SGL_CHECK(src_buffer->size() <= dst_buffer->size(), "Source buffer is larger than destination buffer");

        get_gfx_resource_command_encoder()->copyBuffer(
            dst_buffer->gfx_buffer_resource(),
            0,
            src_buffer->gfx_buffer_resource(),
            0,
            src_buffer->size()
        );
    } else {
        const Texture* dst_texture = dst->as_texture();
        const Texture* src_texture = src->as_texture();
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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK_LE(dst_offset + size, dst->size());
    SGL_CHECK_LE(src_offset + size, src->size());

    set_buffer_state(dst, ResourceState::copy_destination);
    set_buffer_state(src, ResourceState::copy_source);

    get_gfx_resource_command_encoder()
        ->copyBuffer(dst->gfx_buffer_resource(), dst_offset, src->gfx_buffer_resource(), src_offset, size);
}

void CommandBuffer::copy_texture_region(
    Texture* dst,
    uint32_t dst_subresource,
    uint3 dst_offset,
    const Texture* src,
    uint32_t src_subresource,
    uint3 src_offset,
    uint3 extent
)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK_LT(dst_subresource, dst->subresource_count());
    SGL_CHECK_LT(src_subresource, src->subresource_count());

    // TODO: set subresource state instead
    set_texture_state(dst, ResourceState::copy_destination);
    set_texture_state(src, ResourceState::copy_source);

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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK(dst_offset + dst_size <= dst->size(), "Destination buffer is too small");
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK_LT(src_subresource, src->subresource_count());

    // TODO: set subresource state instead
    set_texture_state(src, ResourceState::copy_source);
    set_buffer_state(dst, ResourceState::copy_destination);

    if (all(extent == uint3(-1)))
        extent = src->get_mip_dimensions(src->get_subresource_mip_level(src_subresource)) - src_offset;

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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK(offset + size <= buffer->size(), "Buffer upload is out of bounds");
    SGL_CHECK_NOT_NULL(data);

    set_buffer_state(buffer, ResourceState::copy_destination);

    get_gfx_resource_command_encoder()
        ->uploadBufferData(buffer->gfx_buffer_resource(), offset, size, const_cast<void*>(data));
}

void CommandBuffer::upload_texture_data(Texture* texture, uint32_t subresource, SubresourceData subresource_data)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(texture);
    SGL_CHECK_LT(subresource, texture->subresource_count());

    set_texture_subresource_state(
        texture,
        {
            .mip_level = texture->get_subresource_mip_level(subresource),
            .mip_count = 1,
            .base_array_layer = texture->get_subresource_array_slice(subresource),
            .layer_count = 1,
        },
        ResourceState::copy_destination
    );

    gfx::ITextureResource::SubresourceData gfx_subresource_data = {
        .data = const_cast<void*>(subresource_data.data),
        .strideY = subresource_data.row_pitch,
        .strideZ = subresource_data.slice_pitch,
    };

    gfx::SubresourceRange sr = detail::gfx_subresource_range(texture, subresource);
    get_gfx_resource_command_encoder()->uploadTextureData(
        texture->gfx_texture_resource(),
        sr,
        gfx::ITextureResource::Offset3D(0, 0, 0),
        gfx::ITextureResource::Extents{-1, -1, -1},
        &gfx_subresource_data,
        1
    );
}

void CommandBuffer::resolve_texture(Texture* dst, const Texture* src)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK(dst->desc().sample_count == 1, "Destination texture must not be multi-sampled.");
    SGL_CHECK(src->desc().sample_count > 1, "Source texture must be multi-sampled.");
    SGL_CHECK(dst->desc().format == src->desc().format, "Source and destination textures must have the same format.");
    SGL_CHECK(
        dst->desc().width == src->desc().width && dst->desc().height == src->desc().height,
        "Source and destination textures must have the same dimensions."
    );
    SGL_CHECK(
        dst->desc().array_size == src->desc().array_size,
        "Source and destination textures must have the same array size."
    );
    SGL_CHECK(
        dst->desc().mip_count == src->desc().mip_count,
        "Source and destination textures must have the same mip count."
    );

    set_texture_state(dst, ResourceState::resolve_destination);
    set_texture_state(src, ResourceState::resolve_source);

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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK_LT(dst_subresource, dst->subresource_count());
    SGL_CHECK_LT(src_subresource, src->subresource_count());
    SGL_CHECK(dst->desc().sample_count == 1, "Destination texture must not be multi-sampled.");
    SGL_CHECK(src->desc().sample_count > 1, "Source texture must be multi-sampled.");
    SGL_CHECK(dst->desc().format == src->desc().format, "Source and destination textures must have the same format.");
    uint32_t dst_mip_level = dst->get_subresource_mip_level(dst_subresource);
    uint32_t dst_array_slice = dst->get_subresource_array_slice(dst_subresource);
    uint32_t src_mip_level = src->get_subresource_mip_level(src_subresource);
    uint32_t src_array_slice = src->get_subresource_array_slice(src_subresource);
    SGL_CHECK(
        all(dst->get_mip_dimensions(dst_mip_level) == src->get_mip_dimensions(src_mip_level)),
        "Source and destination textures must have the same dimensions."
    );

    // TODO: set subresource state instead
    set_texture_state(dst, ResourceState::resolve_destination);
    set_texture_state(src, ResourceState::resolve_source);

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

void CommandBuffer::blit(ResourceView* dst, ResourceView* src, TextureFilteringMode filter)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    m_device->_blitter()->blit(this, dst, src, filter);
}

void CommandBuffer::blit(Texture* dst, Texture* src, TextureFilteringMode filter)
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    m_device->_blitter()->blit(this, dst->get_rtv(), src->get_srv(), filter);
}

ComputeCommandEncoder CommandBuffer::encode_compute_commands()
{
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK(!m_encoder_open, "CommandBuffer already has an active encoder");

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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK(!m_encoder_open, "CommandBuffer already has an active encoder");
    SGL_CHECK_NOT_NULL(framebuffer);

    // Set the render target and depth-stencil barriers.
    for (const auto& render_target : framebuffer->desc().render_targets) {
        set_texture_subresource_state(
            render_target->resource()->as_texture(),
            render_target->subresource_range(),
            ResourceState::render_target
        );
    }
    if (const auto& depth_stencil = framebuffer->desc().depth_stencil) {
        set_texture_subresource_state(
            depth_stencil->resource()->as_texture(),
            depth_stencil->subresource_range(),
            ResourceState::depth_write
        );
    }

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
    SGL_CHECK(m_open, "Command buffer is closed");
    SGL_CHECK(!m_encoder_open, "CommandBuffer already has an active encoder");

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
    SGL_CHECK(m_open, "Command buffer is closed");
    get_gfx_resource_command_encoder()->beginDebugEvent(name, &color.x);
}

void CommandBuffer::end_debug_event()
{
    SGL_CHECK(m_open, "Command buffer is closed");
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
    SGL_ASSERT(m_encoder_open);
    m_encoder_open = false;
    end_current_gfx_encoder();
}

gfx::IResourceCommandEncoder* CommandBuffer::get_gfx_resource_command_encoder()
{
    SGL_ASSERT(m_open);
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

} // namespace sgl
