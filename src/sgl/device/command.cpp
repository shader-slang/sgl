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
    rhi::SubresourceRange rhi_subresource_range(const Texture* texture, uint32_t subresource_index)
    {
        return rhi::SubresourceRange{
            .mipLevel = texture->get_subresource_mip_level(subresource_index),
            .mipLevelCount = 1,
            .baseArrayLayer = texture->get_subresource_array_slice(subresource_index),
            .layerCount = 1,
        };
    }

    rhi::BufferWithOffset to_rhi(const BufferWithOffset& buffer_with_offset)
    {
        return rhi::BufferWithOffset(
            buffer_with_offset.buffer ? buffer_with_offset.buffer->rhi_buffer() : nullptr,
            buffer_with_offset.offset
        );
    }

    rhi::DrawArguments to_rhi(const DrawArguments& draw_args)
    {
        return rhi::DrawArguments{
            .vertexCount = draw_args.vertex_count,
            .instanceCount = draw_args.instance_count,
            .startVertexLocation = draw_args.start_vertex_location,
            .startInstanceLocation = draw_args.start_instance_location,
            .startIndexLocation = draw_args.start_index_location,
        };
    }
} // namespace detail

// ----------------------------------------------------------------------------
// PassEncoder
// ----------------------------------------------------------------------------

void PassEncoder::push_debug_group(const char* name, float3 color)
{
    m_rhi_pass_encoder->pushDebugGroup(name, &color[0]);
}

void PassEncoder::pop_debug_group()
{
    m_rhi_pass_encoder->popDebugGroup();
}

void PassEncoder::insert_debug_marker(const char* name, float3 color)
{
    m_rhi_pass_encoder->insertDebugMarker(name, &color[0]);
}

void PassEncoder::end()
{
    SGL_CHECK(m_rhi_pass_encoder, "Pass encoder already ended");

    m_rhi_pass_encoder->end();
    m_rhi_pass_encoder = nullptr;
}

// ----------------------------------------------------------------------------
// RenderPassEncoder
// ----------------------------------------------------------------------------

ShaderObject* RenderPassEncoder::bind_pipeline(RenderPipeline* pipeline)
{
    SGL_CHECK_NOT_NULL(pipeline);

    rhi::IShaderObject* rhi_shader_object = m_rhi_render_pass_encoder->bindPipeline(pipeline->rhi_pipeline());
    return m_command_encoder->_get_root_object(rhi_shader_object);
}

void RenderPassEncoder::bind_pipeline(RenderPipeline* pipeline, ShaderObject* root_object)
{
    SGL_CHECK_NOT_NULL(pipeline);
    SGL_CHECK_NOT_NULL(root_object);

    m_rhi_render_pass_encoder->bindPipeline(pipeline->rhi_pipeline(), root_object->rhi_shader_object());
}

void RenderPassEncoder::set_render_state(const RenderState& state)
{
    rhi::RenderState rhi_state = {};
    rhi_state.stencilRef = state.stencil_ref;
    for (size_t i = 0; i < state.viewports.size(); ++i)
        rhi_state.viewports[i] = stdx::bit_cast<rhi::Viewport>(state.viewports[i]);
    rhi_state.viewportCount = narrow_cast<uint32_t>(state.viewports.size());
    for (size_t i = 0; i < state.scissor_rects.size(); ++i)
        rhi_state.scissorRects[i] = stdx::bit_cast<rhi::ScissorRect>(state.scissor_rects[i]);
    rhi_state.scissorRectCount = narrow_cast<uint32_t>(state.scissor_rects.size());
    for (size_t i = 0; i < state.vertex_buffers.size(); i++)
        rhi_state.vertexBuffers[i] = detail::to_rhi(state.vertex_buffers[i]);
    rhi_state.vertexBufferCount = narrow_cast<uint32_t>(state.vertex_buffers.size());
    rhi_state.indexBuffer = detail::to_rhi(state.index_buffer);
    rhi_state.indexFormat = static_cast<rhi::IndexFormat>(state.index_format);

    m_rhi_render_pass_encoder->setRenderState(rhi_state);
}

void RenderPassEncoder::draw(const DrawArguments& args)
{
    m_rhi_render_pass_encoder->draw(detail::to_rhi(args));
}

void RenderPassEncoder::draw_indexed(const DrawArguments& args)
{
    m_rhi_render_pass_encoder->drawIndexed(detail::to_rhi(args));
}

void RenderPassEncoder::draw_indirect(
    uint32_t max_draw_count,
    BufferWithOffset arg_buffer,
    BufferWithOffset count_buffer
)
{
    // TODO(slang-rhi)
    SGL_UNUSED(max_draw_count, arg_buffer, count_buffer);
    SGL_UNIMPLEMENTED();
    // m_rhi_render_pass_encoder->drawIndirect(max_draw_count, detail::to_rhi(arg_buffer),
    // detail::to_rhi(count_buffer));
}

void RenderPassEncoder::draw_indexed_indirect(
    uint32_t max_draw_count,
    BufferWithOffset arg_buffer,
    BufferWithOffset count_buffer
)
{
    // TODO(slang-rhi)
    SGL_UNUSED(max_draw_count, arg_buffer, count_buffer);
    SGL_UNIMPLEMENTED();
    // m_rhi_render_pass_encoder->drawIndexedIndirect(max_draw_count, detail::to_rhi(arg_buffer),
    // detail::to_rhi(count_buffer));
}

void RenderPassEncoder::draw_mesh_tasks(uint3 dimensions)
{
    m_rhi_render_pass_encoder->drawMeshTasks(dimensions.x, dimensions.y, dimensions.z);
}

void RenderPassEncoder::end()
{
    PassEncoder::end();
    m_rhi_render_pass_encoder = nullptr;
}

// ----------------------------------------------------------------------------
// ComputePassEncoder
// ----------------------------------------------------------------------------

ShaderObject* ComputePassEncoder::bind_pipeline(ComputePipeline* pipeline)
{
    SGL_CHECK_NOT_NULL(pipeline);

    rhi::IShaderObject* rhi_shader_object = m_rhi_compute_pass_encoder->bindPipeline(pipeline->rhi_pipeline());
    m_thread_group_size = pipeline->thread_group_size();
    return m_command_encoder->_get_root_object(rhi_shader_object);
}

void ComputePassEncoder::bind_pipeline(ComputePipeline* pipeline, ShaderObject* root_object)
{
    SGL_CHECK_NOT_NULL(pipeline);
    SGL_CHECK_NOT_NULL(root_object);

    m_rhi_compute_pass_encoder->bindPipeline(pipeline->rhi_pipeline(), root_object->rhi_shader_object());
    m_thread_group_size = pipeline->thread_group_size();
}

void ComputePassEncoder::dispatch(uint3 thread_count)
{
    uint3 thread_group_count{
        div_round_up(thread_count.x, m_thread_group_size.x),
        div_round_up(thread_count.y, m_thread_group_size.y),
        div_round_up(thread_count.z, m_thread_group_size.z)};
    dispatch_thread_groups(thread_group_count);
}

void ComputePassEncoder::dispatch_thread_groups(uint3 thread_group_count)
{
    m_rhi_compute_pass_encoder->dispatchCompute(thread_group_count.x, thread_group_count.y, thread_group_count.z);
}

void ComputePassEncoder::dispatch_thread_groups_indirect(BufferWithOffset arg_buffer)
{
    m_rhi_compute_pass_encoder->dispatchComputeIndirect(arg_buffer.buffer->rhi_buffer(), arg_buffer.offset);
}

void ComputePassEncoder::end()
{
    PassEncoder::end();
    m_rhi_compute_pass_encoder = nullptr;
}

// ----------------------------------------------------------------------------
// RayTracingPassEncoder
// ----------------------------------------------------------------------------

ShaderObject* RayTracingPassEncoder::bind_pipeline(RayTracingPipeline* pipeline, ShaderTable* shader_table)
{
    SGL_CHECK_NOT_NULL(pipeline);

    rhi::IShaderObject* rhi_shader_object
        = m_rhi_ray_tracing_pass_encoder->bindPipeline(pipeline->rhi_pipeline(), shader_table->rhi_shader_table());
    return m_command_encoder->_get_root_object(rhi_shader_object);
}

void RayTracingPassEncoder::bind_pipeline(
    RayTracingPipeline* pipeline,
    ShaderTable* shader_table,
    ShaderObject* root_object
)
{
    SGL_CHECK_NOT_NULL(pipeline);
    SGL_CHECK_NOT_NULL(shader_table);
    SGL_CHECK_NOT_NULL(root_object);

    m_rhi_ray_tracing_pass_encoder
        ->bindPipeline(pipeline->rhi_pipeline(), shader_table->rhi_shader_table(), root_object->rhi_shader_object());
}

void RayTracingPassEncoder::dispatch_rays(uint32_t ray_gen_shader_index, uint3 dimensions)
{
    m_rhi_ray_tracing_pass_encoder->dispatchRays(ray_gen_shader_index, dimensions.x, dimensions.y, dimensions.z);
}

void RayTracingPassEncoder::end()
{
    PassEncoder::end();
    m_rhi_ray_tracing_pass_encoder = nullptr;
}

// ----------------------------------------------------------------------------
// CommandEncoder
// ----------------------------------------------------------------------------

CommandEncoder::CommandEncoder(ref<Device> device, Slang::ComPtr<rhi::ICommandEncoder> rhi_command_encoder)
    : DeviceResource(std::move(device))
    , m_rhi_command_encoder(std::move(rhi_command_encoder))
    , m_open(true)
{
}

ref<RenderPassEncoder> CommandEncoder::begin_render_pass(const RenderPassDesc& desc)
{
    rhi::RenderPassDesc rhi_desc = {};

    short_vector<rhi::RenderPassColorAttachment, 16> rhi_color_attachments;
    for (const auto& ca : desc.color_attachments) {
        rhi_color_attachments.push_back({
            .view = ca.view ? ca.view->rhi_texture_view() : nullptr,
            .resolveTarget = ca.resolve_target ? ca.resolve_target->rhi_texture_view() : nullptr,
            .loadOp = static_cast<rhi::LoadOp>(ca.load_op),
            .storeOp = static_cast<rhi::StoreOp>(ca.store_op),
            .clearValue = {ca.clear_value[0], ca.clear_value[1], ca.clear_value[2], ca.clear_value[3]},
        });
    }
    rhi_desc.colorAttachments = rhi_color_attachments.data();
    rhi_desc.colorAttachmentCount = narrow_cast<uint32_t>(rhi_color_attachments.size());

    rhi::RenderPassDepthStencilAttachment rhi_depth_stencil_attachment = {};
    if (desc.depth_stencil_attachment) {
        const auto& dsa = *desc.depth_stencil_attachment;
        rhi_depth_stencil_attachment = {
            .view = dsa.view ? dsa.view->rhi_texture_view() : nullptr,
            .depthLoadOp = static_cast<rhi::LoadOp>(dsa.depth_load_op),
            .depthStoreOp = static_cast<rhi::StoreOp>(dsa.depth_store_op),
            .depthClearValue = dsa.depth_clear_value,
            .depthReadOnly = dsa.depth_read_only,
            .stencilLoadOp = static_cast<rhi::LoadOp>(dsa.stencil_load_op),
            .stencilStoreOp = static_cast<rhi::StoreOp>(dsa.stencil_store_op),
            .stencilClearValue = dsa.stencil_clear_value,
            .stencilReadOnly = dsa.stencil_read_only,
        };
        rhi_desc.depthStencilAttachment = &rhi_depth_stencil_attachment;
    }

    if (!m_render_pass_encoder) {
        m_render_pass_encoder = make_ref<RenderPassEncoder>();
    }

    m_render_pass_encoder->m_command_encoder = this;
    m_render_pass_encoder->m_rhi_render_pass_encoder = m_rhi_command_encoder->beginRenderPass(rhi_desc);
    m_render_pass_encoder->m_rhi_pass_encoder = m_render_pass_encoder->m_rhi_render_pass_encoder;
    return m_render_pass_encoder;
}

ref<ComputePassEncoder> CommandEncoder::begin_compute_pass()
{
    if (!m_compute_pass_encoder) {
        m_compute_pass_encoder = make_ref<ComputePassEncoder>();
    }

    m_compute_pass_encoder->m_command_encoder = this;
    m_compute_pass_encoder->m_rhi_compute_pass_encoder = m_rhi_command_encoder->beginComputePass();
    m_compute_pass_encoder->m_rhi_pass_encoder = m_compute_pass_encoder->m_rhi_compute_pass_encoder;
    return m_compute_pass_encoder;
}

ref<RayTracingPassEncoder> CommandEncoder::begin_ray_tracing_pass()
{
    if (!m_ray_tracing_pass_encoder) {
        m_ray_tracing_pass_encoder = make_ref<RayTracingPassEncoder>();
    }

    m_ray_tracing_pass_encoder->m_command_encoder = this;
    m_ray_tracing_pass_encoder->m_rhi_ray_tracing_pass_encoder = m_rhi_command_encoder->beginRayTracingPass();
    m_ray_tracing_pass_encoder->m_rhi_pass_encoder = m_ray_tracing_pass_encoder->m_rhi_ray_tracing_pass_encoder;
    return m_ray_tracing_pass_encoder;
}

ShaderObject* CommandEncoder::_get_root_object(rhi::IShaderObject* rhi_shader_object)
{
    m_root_object = make_ref<ShaderObject>(m_device, rhi_shader_object, false);
    return m_root_object.get();
}

void CommandEncoder::copy_buffer(
    Buffer* dst,
    DeviceOffset dst_offset,
    const Buffer* src,
    DeviceOffset src_offset,
    DeviceSize size
)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK_LE(dst_offset + size, dst->size());
    SGL_CHECK_LE(src_offset + size, src->size());

    m_rhi_command_encoder->copyBuffer(dst->rhi_buffer(), dst_offset, src->rhi_buffer(), src_offset, size);
}

void CommandEncoder::copy_texture(
    Texture* dst,
    uint32_t dst_subresource,
    uint3 dst_offset,
    const Texture* src,
    uint32_t src_subresource,
    uint3 src_offset,
    uint3 extent
)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK_LT(dst_subresource, dst->subresource_count());
    SGL_CHECK_LT(src_subresource, src->subresource_count());

    rhi::SubresourceRange dst_sr = detail::rhi_subresource_range(dst, dst_subresource);
    rhi::SubresourceRange src_sr = detail::rhi_subresource_range(src, src_subresource);

    if (all(extent == uint3(-1)))
        extent = src->get_mip_dimensions(src_sr.mipLevel) - src_offset;

    rhi::Extents rhi_extent
        = {narrow_cast<int32_t>(extent.x), narrow_cast<int32_t>(extent.y), narrow_cast<int32_t>(extent.z)};

    m_rhi_command_encoder->copyTexture(
        dst->rhi_texture(),
        dst_sr,
        rhi::Offset3D(dst_offset.x, dst_offset.y, dst_offset.z),
        src->rhi_texture(),
        src_sr,
        rhi::Offset3D(src_offset.x, src_offset.y, src_offset.z),
        rhi_extent
    );
}

void CommandEncoder::copy_texture_to_buffer(
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
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK(dst_offset + dst_size <= dst->size(), "Destination buffer is too small");
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK_LT(src_subresource, src->subresource_count());

    const FormatInfo& info = get_format_info(src->format());
    SGL_CHECK(
        (src_offset.x % info.block_width == 0) && (src_offset.y % info.block_height == 0),
        "Source offset ({},{}) must be a multiple of the block size ({}x{})",
        src_offset.x,
        src_offset.y,
        info.block_width,
        info.block_height
    );

    if (all(extent == uint3(-1))) {
        extent = src->get_mip_dimensions(src->get_subresource_mip_level(src_subresource)) - src_offset;
    }

    // TODO: in D3D12, the extent must be a multiple of the block size (this should be fixed in gfx instead)
    if (m_device->type() == DeviceType::d3d12) {
        extent.x = align_to(info.block_width, extent.x);
        extent.y = align_to(info.block_height, extent.y);
    }

    rhi::SubresourceRange src_sr = detail::rhi_subresource_range(src, src_subresource);
    m_rhi_command_encoder->copyTextureToBuffer(
        dst->rhi_buffer(),
        dst_offset,
        dst_size,
        dst_row_stride,
        src->rhi_texture(),
        src_sr,
        rhi::Offset3D(src_offset.x, src_offset.y, src_offset.z),
        rhi::Extents{narrow_cast<int32_t>(extent.x), narrow_cast<int32_t>(extent.y), narrow_cast<int32_t>(extent.z)}
    );
}

void CommandEncoder::upload_buffer_data(Buffer* buffer, size_t offset, size_t size, const void* data)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK(offset + size <= buffer->size(), "Buffer upload is out of bounds");
    SGL_CHECK_NOT_NULL(data);

    set_buffer_state(buffer, ResourceState::copy_destination);

    m_rhi_command_encoder->uploadBufferData(buffer->rhi_buffer(), offset, size, const_cast<void*>(data));
}

void CommandEncoder::upload_texture_data(Texture* texture, uint32_t subresource, SubresourceData subresource_data)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(texture);
    SGL_CHECK_LT(subresource, texture->subresource_count());

    rhi::SubresourceData rhi_subresource_data = {
        .data = const_cast<void*>(subresource_data.data),
        .strideY = subresource_data.row_pitch,
        .strideZ = subresource_data.slice_pitch,
    };

    rhi::SubresourceRange sr = detail::rhi_subresource_range(texture, subresource);
    m_rhi_command_encoder->uploadTextureData(
        texture->rhi_texture(),
        sr,
        rhi::Offset3D(0, 0, 0),
        rhi::Extents{-1, -1, -1},
        &rhi_subresource_data,
        1
    );
}

void CommandEncoder::clear_buffer(Buffer* buffer, BufferRange range)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(buffer);

    m_rhi_command_encoder->clearBuffer(buffer->rhi_buffer(), range.offset, range.size);
}

void CommandEncoder::clear_texture(
    Texture* texture,
    float4 clear_value,
    SubresourceRange range,
    bool clear_depth,
    bool clear_stencil
)
{
    // TODO(slang-rhi)
    SGL_UNUSED(texture, clear_value, range, clear_depth, clear_stencil);
    SGL_UNIMPLEMENTED();
}

void CommandEncoder::clear_texture(
    Texture* texture,
    uint4 clear_value,
    SubresourceRange range,
    bool clear_depth,
    bool clear_stencil
)
{
    // TODO(slang-rhi)
    SGL_UNUSED(texture, clear_value, range, clear_depth, clear_stencil);
    SGL_UNIMPLEMENTED();
}

void CommandEncoder::blit(TextureView* dst, TextureView* src, TextureFilteringMode filter)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    m_device->_blitter()->blit(this, dst, src, filter);
}

void CommandEncoder::blit(Texture* dst, Texture* src, TextureFilteringMode filter)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    // TODO(slang-rhi)
    SGL_UNUSED(dst, src, filter);
    SGL_UNIMPLEMENTED();
}

void CommandEncoder::resolve_query(
    QueryPool* query_pool,
    uint32_t index,
    uint32_t count,
    Buffer* buffer,
    DeviceOffset offset
)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(query_pool);
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK_LE(index + count, query_pool->desc().count);
    SGL_CHECK_LE(offset + count * sizeof(uint64_t), buffer->size());

    m_rhi_command_encoder->resolveQuery(query_pool->rhi_query_pool(), index, count, buffer->rhi_buffer(), offset);
}

#if 0
void CommandEncoder::build_acceleration_structure(
    const AccelerationStructureBuildDesc& desc,
    std::span<AccelerationStructureQueryDesc> queries
)
{
    rhi::IAccelerationStructure::BuildDesc rhi_build_desc{
        .inputs = reinterpret_cast<const rhi::IAccelerationStructure::BuildInputs&>(desc.inputs),
        .source = desc.src ? desc.src->rhi_acceleration_structure() : nullptr,
        .dest = desc.dst ? desc.dst->rhi_acceleration_structure() : nullptr,
        .scratchData = desc.scratch_data,
    };

    short_vector<rhi::AccelerationStructureQueryDesc, 16> rhi_query_descs(queries.size(), {});
    for (size_t i = 0; i < queries.size(); i++) {
        rhi_query_descs[i] = {
            .queryType = static_cast<rhi::QueryType>(queries[i].query_type),
            .queryPool = queries[i].query_pool->rhi_query_pool(),
            .firstQueryIndex = narrow_cast<rhi::GfxIndex>(queries[i].first_query_index),
        };
    }

    m_rhi_ray_tracing_command_encoder->buildAccelerationStructure(
        rhi_build_desc,
        narrow_cast<rhi::GfxCount>(rhi_query_descs.size()),
        rhi_query_descs.data()
    );

    m_rhi_command_encoder->buildAccelerationStructure()
}
#endif

void CommandEncoder::copy_acceleration_structure(
    AccelerationStructure* dst,
    AccelerationStructure* src,
    AccelerationStructureCopyMode mode
)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst);

    m_rhi_command_encoder->copyAccelerationStructure(
        dst->rhi_acceleration_structure(),
        src->rhi_acceleration_structure(),
        static_cast<rhi::AccelerationStructureCopyMode>(mode)
    );
}

void CommandEncoder::query_acceleration_structure_properties(
    std::span<AccelerationStructure*> acceleration_structures,
    std::span<AccelerationStructureQueryDesc> queries
)
{
    SGL_CHECK(m_open, "Command encoder is finished");

    short_vector<rhi::IAccelerationStructure*, 16> rhi_acceleration_structures(acceleration_structures.size(), nullptr);
    for (size_t i = 0; i < acceleration_structures.size(); i++)
        rhi_acceleration_structures[i] = acceleration_structures[i]->rhi_acceleration_structure();

    short_vector<rhi::AccelerationStructureQueryDesc, 16> rhi_queries(queries.size(), {});
    for (size_t i = 0; i < queries.size(); i++) {
        rhi_queries[i] = {
            .queryType = static_cast<rhi::QueryType>(queries[i].query_type),
            .queryPool = queries[i].query_pool->rhi_query_pool(),
            .firstQueryIndex = queries[i].first_query_index,
        };
    }

    m_rhi_command_encoder->queryAccelerationStructureProperties(
        narrow_cast<uint32_t>(rhi_acceleration_structures.size()),
        rhi_acceleration_structures.data(),
        narrow_cast<uint32_t>(rhi_queries.size()),
        rhi_queries.data()
    );
}

void CommandEncoder::serialize_acceleration_structure(BufferWithOffset dst, AccelerationStructure* src)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst.buffer);
    SGL_CHECK_NOT_NULL(src);

    m_rhi_command_encoder->serializeAccelerationStructure(detail::to_rhi(dst), src->rhi_acceleration_structure());
}

void CommandEncoder::deserialize_acceleration_structure(AccelerationStructure* dst, BufferWithOffset src)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src.buffer);

    m_rhi_command_encoder->deserializeAccelerationStructure(dst->rhi_acceleration_structure(), detail::to_rhi(src));
}

void CommandEncoder::set_buffer_state(Buffer* buffer, ResourceState state)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(buffer);

    m_rhi_command_encoder->setBufferState(buffer->rhi_buffer(), static_cast<rhi::ResourceState>(state));
}

void CommandEncoder::set_texture_state(Texture* texture, ResourceState state)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(texture);

    m_rhi_command_encoder->setTextureState(texture->rhi_texture(), static_cast<rhi::ResourceState>(state));
}

void CommandEncoder::set_texture_state(Texture* texture, SubresourceRange range, ResourceState state)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(texture);

    m_rhi_command_encoder->setTextureState(
        texture->rhi_texture(),
        {
            .mipLevel = range.mip_level,
            .mipLevelCount = range.mip_count,
            .baseArrayLayer = range.base_array_layer,
            .layerCount = range.layer_count,
        },
        static_cast<rhi::ResourceState>(state)
    );
}

void CommandEncoder::push_debug_group(const char* name, float3 color)
{
    SGL_CHECK(m_open, "Command encoder is finished");

    m_rhi_command_encoder->pushDebugGroup(name, &color[0]);
}

void CommandEncoder::pop_debug_group()
{
    SGL_CHECK(m_open, "Command encoder is finished");

    m_rhi_command_encoder->popDebugGroup();
}

void CommandEncoder::insert_debug_marker(const char* name, float3 color)
{
    SGL_CHECK(m_open, "Command encoder is finished");

    m_rhi_command_encoder->insertDebugMarker(name, &color[0]);
}

void CommandEncoder::write_timestamp(QueryPool* query_pool, uint32_t index)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(query_pool);
    SGL_CHECK_LE(index, query_pool->desc().count);

    m_rhi_command_encoder->writeTimestamp(query_pool->rhi_query_pool(), index);
}

ref<CommandBuffer> CommandEncoder::finish()
{
    SGL_CHECK(m_open, "Command encoder is finished");
    Slang::ComPtr<rhi::ICommandBuffer> rhi_command_buffer;
    SLANG_CALL(m_rhi_command_encoder->finish(rhi_command_buffer.writeRef()));
    ref<CommandBuffer> command_buffer = make_ref<CommandBuffer>(m_device, rhi_command_buffer);
    m_open = false;
    return command_buffer;
}

NativeHandle CommandEncoder::get_native_handle() const
{
    rhi::NativeHandle rhi_handle;
    SLANG_CALL(m_rhi_command_encoder->getNativeHandle(&rhi_handle));
    return NativeHandle(rhi_handle);
}

std::string CommandEncoder::to_string() const
{
    return fmt::format(
        "CommandEncoder(\n"
        "  device = {}\n"
        ")",
        m_device
    );
}

// ----------------------------------------------------------------------------
// CommandBuffer
// ----------------------------------------------------------------------------

CommandBuffer::CommandBuffer(ref<Device> device, Slang::ComPtr<rhi::ICommandBuffer> command_buffer)
    : DeviceResource(std::move(device))
    , m_rhi_command_buffer(std::move(command_buffer))
{
}

CommandBuffer::~CommandBuffer() { }

std::string CommandBuffer::to_string() const
{
    return fmt::format(
        "CommandBuffer(\n"
        "  device = {}\n"
        ")",
        m_device
    );
}


#if 0

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
    rhi::IShaderObject* rhi_shader_object;
    SLANG_CALL(m_rhi_compute_command_encoder->bindPipeline(pipeline->rhi_pipeline(), &rhi_shader_object));
    ref<TransientShaderObject> transient_shader_object
        = make_ref<TransientShaderObject>(ref<Device>(m_command_buffer->device()), rhi_shader_object, m_command_buffer);
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
    SLANG_CALL(m_rhi_compute_command_encoder
                   ->bindPipelineWithRootObject(pipeline->rhi_pipeline(), shader_object->rhi_shader_object()));
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
        m_rhi_compute_command_encoder->dispatchCompute(thread_group_count.x, thread_group_count.y, thread_group_count.z)
    );
}

void ComputeCommandEncoder::dispatch_thread_groups_indirect(const Buffer* cmd_buffer, DeviceOffset offset)
{
    SGL_CHECK_NOT_NULL(cmd_buffer);
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    SLANG_CALL(m_rhi_compute_command_encoder->dispatchComputeIndirect(cmd_buffer->rhi_buffer(), offset));
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
    rhi::IShaderObject* rhi_shader_object;
    SLANG_CALL(m_rhi_render_command_encoder->bindPipeline(pipeline->rhi_pipeline(), &rhi_shader_object));
    ref<TransientShaderObject> transient_shader_object
        = make_ref<TransientShaderObject>(ref<Device>(m_command_buffer->device()), rhi_shader_object, m_command_buffer);
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
    SLANG_CALL(m_rhi_render_command_encoder
                   ->bindPipelineWithRootObject(pipeline->rhi_pipeline(), shader_object->rhi_shader_object()));
}

void RenderCommandEncoder::set_viewports(std::span<Viewport> viewports)
{
    m_rhi_render_command_encoder->setViewports(
        narrow_cast<rhi::GfxCount>(viewports.size()),
        reinterpret_cast<const rhi::Viewport*>(viewports.data())
    );
}

void RenderCommandEncoder::set_scissor_rects(std::span<ScissorRect> scissor_rects)
{
    m_rhi_render_command_encoder->setScissorRects(
        narrow_cast<rhi::GfxCount>(scissor_rects.size()),
        reinterpret_cast<const rhi::ScissorRect*>(scissor_rects.data())
    );
}

void RenderCommandEncoder::set_viewport_and_scissor_rect(const Viewport& viewport)
{
    m_rhi_render_command_encoder->setViewportAndScissor(reinterpret_cast<const rhi::Viewport&>(viewport));
}

void RenderCommandEncoder::set_primitive_topology(PrimitiveTopology topology)
{
    m_rhi_render_command_encoder->setPrimitiveTopology(static_cast<rhi::PrimitiveTopology>(topology));
}

void RenderCommandEncoder::set_stencil_reference(uint32_t reference_value)
{
    m_rhi_render_command_encoder->setStencilReference(reference_value);
}

void RenderCommandEncoder::set_vertex_buffers(uint32_t start_slot, std::span<Slot> slots)
{
    short_vector<rhi::IBufferResource*, 16> rhi_buffers(slots.size(), nullptr);
    short_vector<rhi::Offset, 16> rhi_offsets(slots.size(), 0);
    for (size_t i = 0; i < slots.size(); i++) {
        rhi_buffers[i] = slots[i].buffer->rhi_buffer();
        rhi_offsets[i] = slots[i].offset;
    }
    m_rhi_render_command_encoder->setVertexBuffers(
        start_slot,
        narrow_cast<rhi::GfxCount>(slots.size()),
        rhi_buffers.data(),
        rhi_offsets.data()
    );
}

void RenderCommandEncoder::set_vertex_buffer(uint32_t slot, const Buffer* buffer, DeviceOffset offset)
{
    rhi::IBufferResource* rhi_buffer = buffer->rhi_buffer();
    m_rhi_render_command_encoder->setVertexBuffer(slot, rhi_buffer, offset);
}

void RenderCommandEncoder::set_index_buffer(const Buffer* buffer, Format index_format, DeviceOffset offset)
{
    m_rhi_render_command_encoder
        ->setIndexBuffer(buffer->rhi_buffer(), static_cast<rhi::Format>(index_format), offset);
}

void RenderCommandEncoder::draw(uint32_t vertex_count, uint32_t start_vertex)
{
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

    SLANG_CALL(m_rhi_render_command_encoder->draw(vertex_count, start_vertex));
}

void RenderCommandEncoder::draw_indexed(uint32_t index_count, uint32_t start_index, uint32_t base_vertex)
{
    SGL_CHECK(m_bound_pipeline, "No pipeline bound");

    m_bound_shader_object->get_cuda_interop_buffers(m_command_buffer->m_cuda_interop_buffers);

    SLANG_CALL(m_rhi_render_command_encoder->drawIndexed(index_count, start_index, base_vertex));
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

    SLANG_CALL(m_rhi_render_command_encoder->drawInstanced(vertex_count, instance_count, start_vertex, start_instance));
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

    SLANG_CALL(m_rhi_render_command_encoder
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

    SLANG_CALL(m_rhi_render_command_encoder->drawIndirect(
        max_draw_count,
        arg_buffer->rhi_buffer(),
        arg_offset,
        count_buffer ? count_buffer->rhi_buffer() : nullptr,
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

    SLANG_CALL(m_rhi_render_command_encoder->drawIndexedIndirect(
        max_draw_count,
        arg_buffer->rhi_buffer(),
        arg_offset,
        count_buffer ? count_buffer->rhi_buffer() : nullptr,
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
    rhi::IShaderObject* rhi_shader_object;
    SLANG_CALL(m_rhi_ray_tracing_command_encoder->bindPipeline(pipeline->rhi_pipeline(), &rhi_shader_object));
    ref<TransientShaderObject> transient_shader_object
        = make_ref<TransientShaderObject>(ref<Device>(m_command_buffer->device()), rhi_shader_object, m_command_buffer);
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
    SLANG_CALL(m_rhi_ray_tracing_command_encoder
                   ->bindPipelineWithRootObject(pipeline->rhi_pipeline(), shader_object->rhi_shader_object()));
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

    SLANG_CALL(m_rhi_ray_tracing_command_encoder->dispatchRays(
        narrow_cast<rhi::GfxIndex>(ray_gen_shader_index),
        shader_table->rhi_shader_table(),
        narrow_cast<rhi::GfxCount>(dimensions.x),
        narrow_cast<rhi::GfxCount>(dimensions.y),
        narrow_cast<rhi::GfxCount>(dimensions.z)
    ));
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

    m_rhi_transient_resource_heap = m_device->_get_or_create_transient_resource_heap();
    SLANG_CALL(m_rhi_transient_resource_heap->createCommandBuffer(m_rhi_command_buffer.writeRef()));
    m_open = true;
}

void CommandBuffer::close()
{
    if (!m_open)
        return;

    if (m_encoder_open)
        SGL_THROW("Cannot close command buffer with an open encoder.");

    m_device->_set_open_command_buffer(nullptr);

    end_current_rhi_encoder();
    m_rhi_command_buffer->close();
    m_open = false;
}

uint64_t CommandBuffer::submit(CommandQueueType queue)
{
    close();
    return m_device->submit_command_buffer(this, queue);
}


bool CommandBuffer::set_resource_state(const Resource* resource, ResourceState new_state)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(resource);

    if (resource->type() == ResourceType::buffer) {
        return set_buffer_state(resource->as_buffer(), new_state);
    } else {
        return set_texture_state(resource->as_texture(), new_state);
    }
}

bool CommandBuffer::set_resource_state(const ResourceView* resource_view, ResourceState new_state)
{
    SGL_CHECK(m_open, "Command encoder is finished");
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


void CommandBuffer::uav_barrier(const Resource* resource)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(resource);

    if (set_resource_state(resource, ResourceState::unordered_access))
        return;

    if (resource->type() == ResourceType::buffer) {
        get_rhi_resource_command_encoder()->bufferBarrier(
            resource->as_buffer()->rhi_buffer(),
            rhi::ResourceState::UnorderedAccess,
            rhi::ResourceState::UnorderedAccess
        );
    } else {
        get_rhi_resource_command_encoder()->textureBarrier(
            resource->as_texture()->rhi_texture(),
            rhi::ResourceState::UnorderedAccess,
            rhi::ResourceState::UnorderedAccess
        );
    }
}

void CommandBuffer::clear_resource_view(ResourceView* resource_view, float4 clear_value)
{
    SGL_CHECK(m_open, "Command encoder is finished");
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

    rhi::ClearValue rhi_clear_value = {};
    rhi_clear_value.color.floatValues[0] = clear_value[0];
    rhi_clear_value.color.floatValues[1] = clear_value[1];
    rhi_clear_value.color.floatValues[2] = clear_value[2];
    rhi_clear_value.color.floatValues[3] = clear_value[3];
    get_rhi_resource_command_encoder()->clearResourceView(
        resource_view->rhi_resource_view(),
        &rhi_clear_value,
        rhi::ClearResourceViewFlags::FloatClearValues
    );
}

void CommandBuffer::clear_resource_view(ResourceView* resource_view, uint4 clear_value)
{
    SGL_CHECK(m_open, "Command encoder is finished");
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

    rhi::ClearValue rhi_clear_value = {};
    rhi_clear_value.color.uintValues[0] = clear_value[0];
    rhi_clear_value.color.uintValues[1] = clear_value[1];
    rhi_clear_value.color.uintValues[2] = clear_value[2];
    rhi_clear_value.color.uintValues[3] = clear_value[3];
    get_rhi_resource_command_encoder()
        ->clearResourceView(resource_view->rhi_resource_view(), &rhi_clear_value, rhi::ClearResourceViewFlags::None);
}

void CommandBuffer::clear_resource_view(
    ResourceView* resource_view,
    float depth_value,
    uint32_t stencil_value,
    bool clear_depth,
    bool clear_stencil
)
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(resource_view);

    switch (resource_view->type()) {
    case ResourceViewType::depth_stencil:
        set_resource_state(resource_view, ResourceState::depth_write);
        break;
    default:
        SGL_THROW("Invalid resource view type");
    }

    rhi::ClearValue rhi_clear_value = {};
    rhi_clear_value.depthStencil.depth = depth_value;
    rhi_clear_value.depthStencil.stencil = stencil_value;
    uint32_t rhi_flags = rhi::ClearResourceViewFlags::None;
    if (clear_depth)
        rhi_flags |= rhi::ClearResourceViewFlags::ClearDepth;
    if (clear_stencil)
        rhi_flags |= rhi::ClearResourceViewFlags::ClearStencil;
    get_rhi_resource_command_encoder()->clearResourceView(
        resource_view->rhi_resource_view(),
        &rhi_clear_value,
        rhi::ClearResourceViewFlags::Enum(rhi_flags)
    );
}

void CommandBuffer::clear_texture(Texture* texture, float4 clear_value)
{
    SGL_CHECK(m_open, "Command encoder is finished");
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
    SGL_CHECK(m_open, "Command encoder is finished");
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
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK_NOT_NULL(dst);
    SGL_CHECK_NOT_NULL(src);
    SGL_CHECK(dst->type() == src->type(), "Resources must be of the same type");

    set_resource_state(dst, ResourceState::copy_destination);
    set_resource_state(src, ResourceState::copy_source);

    if (dst->type() == ResourceType::buffer) {
        const Buffer* dst_buffer = dst->as_buffer();
        const Buffer* src_buffer = src->as_buffer();

        SGL_CHECK(src_buffer->size() <= dst_buffer->size(), "Source buffer is larger than destination buffer");

        get_rhi_resource_command_encoder()->copyBuffer(
            dst_buffer->rhi_buffer(),
            0,
            src_buffer->rhi_buffer(),
            0,
            src_buffer->size()
        );
    } else {
        const Texture* dst_texture = dst->as_texture();
        const Texture* src_texture = src->as_texture();
        rhi::SubresourceRange sr_range = {};

        get_rhi_resource_command_encoder()->copyTexture(
            dst_texture->rhi_texture(),
            rhi::ResourceState::CopyDestination,
            sr_range,
            rhi::ITextureResource::Offset3D(0, 0, 0),
            src_texture->rhi_texture(),
            rhi::ResourceState::CopySource,
            sr_range,
            rhi::ITextureResource::Offset3D(0, 0, 0),
            rhi::ITextureResource::Extents{0, 0, 0}
        );
    }
}




void CommandBuffer::resolve_texture(Texture* dst, const Texture* src)
{
    SGL_CHECK(m_open, "Command encoder is finished");
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

    rhi::SubresourceRange dst_sr = {};
    dst_sr.layerCount = dst->array_size();
    dst_sr.mipLevelCount = dst->mip_count();
    rhi::SubresourceRange src_sr = {};
    src_sr.layerCount = src->array_size();
    src_sr.mipLevelCount = src->mip_count();

    get_rhi_resource_command_encoder()->resolveResource(
        src->rhi_texture(),
        rhi::ResourceState::ResolveSource,
        src_sr,
        dst->rhi_texture(),
        rhi::ResourceState::ResolveDestination,
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
    SGL_CHECK(m_open, "Command encoder is finished");
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

    rhi::SubresourceRange dst_sr = {
        .mipLevel = narrow_cast<rhi::GfxIndex>(dst_mip_level),
        .mipLevelCount = 1,
        .baseArrayLayer = narrow_cast<rhi::GfxIndex>(dst_array_slice),
        .layerCount = 1,
    };
    rhi::SubresourceRange src_sr = {
        .mipLevel = narrow_cast<rhi::GfxIndex>(src_mip_level),
        .mipLevelCount = 1,
        .baseArrayLayer = narrow_cast<rhi::GfxIndex>(src_array_slice),
        .layerCount = 1,
    };

    get_rhi_resource_command_encoder()->resolveResource(
        src->rhi_texture(),
        rhi::ResourceState::ResolveSource,
        src_sr,
        dst->rhi_texture(),
        rhi::ResourceState::ResolveDestination,
        dst_sr
    );
}

ComputeCommandEncoder CommandBuffer::encode_compute_commands()
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK(!m_encoder_open, "CommandBuffer already has an active encoder");

    if (m_active_rhi_encoder != EncoderType::compute) {
        end_current_rhi_encoder();
        m_rhi_command_encoder = m_rhi_command_buffer->encodeComputeCommands();
        m_active_rhi_encoder = EncoderType::compute;
    }

    m_encoder_open = true;
    return ComputeCommandEncoder(this, (static_cast<rhi::IComputeCommandEncoder*>(m_rhi_command_encoder.get())));
}

RenderCommandEncoder CommandBuffer::encode_render_commands(Framebuffer* framebuffer)
{
    SGL_CHECK(m_open, "Command encoder is finished");
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

    if (m_active_rhi_encoder != EncoderType::render) {
        end_current_rhi_encoder();
        m_rhi_command_encoder = m_rhi_command_buffer->encodeRenderCommands(
            framebuffer->rhi_render_pass_layout(),
            framebuffer->rhi_framebuffer()
        );
        m_active_rhi_encoder = EncoderType::render;
    }

    m_encoder_open = true;
    return RenderCommandEncoder(this, (static_cast<rhi::IRenderCommandEncoder*>(m_rhi_command_encoder.get())));
}

RayTracingCommandEncoder CommandBuffer::encode_ray_tracing_commands()
{
    SGL_CHECK(m_open, "Command encoder is finished");
    SGL_CHECK(!m_encoder_open, "CommandBuffer already has an active encoder");

    if (m_active_rhi_encoder != EncoderType::raytracing) {
        end_current_rhi_encoder();
        m_rhi_command_encoder = m_rhi_command_buffer->encodeRayTracingCommands();
        m_active_rhi_encoder = EncoderType::raytracing;
    }

    m_encoder_open = true;
    return RayTracingCommandEncoder(this, (static_cast<rhi::IRayTracingCommandEncoder*>(m_rhi_command_encoder.get())));
}

void CommandBuffer::end_encoder()
{
    SGL_ASSERT(m_encoder_open);
    m_encoder_open = false;
    end_current_rhi_encoder();
}

rhi::IResourceCommandEncoder* CommandBuffer::get_rhi_resource_command_encoder()
{
    SGL_ASSERT(m_open);
    if (m_active_rhi_encoder == EncoderType::none) {
        m_rhi_command_encoder = m_rhi_command_buffer->encodeResourceCommands();
        m_active_rhi_encoder = EncoderType::resource;
    }
    return static_cast<rhi::IResourceCommandEncoder*>(m_rhi_command_encoder.get());
}

void CommandBuffer::end_current_rhi_encoder()
{
    if (m_active_rhi_encoder != EncoderType::none) {
        m_rhi_command_encoder->endEncoding();
        m_rhi_command_encoder = nullptr;
        m_active_rhi_encoder = EncoderType::none;
    }
}

#endif

} // namespace sgl
