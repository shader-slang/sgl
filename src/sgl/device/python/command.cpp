// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/command.h"
#include "sgl/device/query.h"
#include "sgl/device/pipeline.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/raytracing.h"

namespace sgl {

SGL_DICT_TO_DESC_BEGIN(RenderState)
SGL_DICT_TO_DESC_FIELD(stencil_ref, uint32_t)
SGL_DICT_TO_DESC_FIELD_LIST(viewports, Viewport)
SGL_DICT_TO_DESC_FIELD_LIST(scissor_rects, ScissorRect)
SGL_DICT_TO_DESC_FIELD_LIST(vertex_buffers, BufferOffsetPair)
SGL_DICT_TO_DESC_FIELD(index_buffer, BufferOffsetPair)
SGL_DICT_TO_DESC_FIELD(index_format, IndexFormat)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(RenderPassColorAttachment)
SGL_DICT_TO_DESC_FIELD(view, TextureView*)
SGL_DICT_TO_DESC_FIELD(resolve_target, TextureView*)
SGL_DICT_TO_DESC_FIELD(load_op, LoadOp)
SGL_DICT_TO_DESC_FIELD(store_op, StoreOp)
SGL_DICT_TO_DESC_FIELD(clear_value, float4)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(RenderPassDepthStencilAttachment)
SGL_DICT_TO_DESC_FIELD(view, TextureView*)
SGL_DICT_TO_DESC_FIELD(depth_load_op, LoadOp)
SGL_DICT_TO_DESC_FIELD(depth_store_op, StoreOp)
SGL_DICT_TO_DESC_FIELD(depth_clear_value, float)
SGL_DICT_TO_DESC_FIELD(depth_read_only, bool)
SGL_DICT_TO_DESC_FIELD(stencil_load_op, LoadOp)
SGL_DICT_TO_DESC_FIELD(stencil_store_op, StoreOp)
SGL_DICT_TO_DESC_FIELD(stencil_clear_value, uint8_t)
SGL_DICT_TO_DESC_FIELD(stencil_read_only, bool)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(RenderPassDesc)
SGL_DICT_TO_DESC_FIELD_LIST(color_attachments, RenderPassColorAttachment)
SGL_DICT_TO_DESC_FIELD(depth_stencil_attachment, RenderPassDepthStencilAttachment)
SGL_DICT_TO_DESC_END()

} // namespace sgl

SGL_PY_EXPORT(device_command)
{
    using namespace sgl;

    // TODO(slang-rhi) move to types?

    nb::class_<RenderState>(m, "RenderState", D_NA(RenderState))
        .def(nb::init<>())
        .def("__init__", [](RenderState* self, nb::dict dict) { new (self) RenderState(dict_to_RenderState(dict)); })
        .def_rw("stencil_ref", &RenderState::stencil_ref, D_NA(RenderState, stencil_ref))
        .def_rw("viewports", &RenderState::viewports, D_NA(RenderState, viewports))
        .def_rw("scissor_rects", &RenderState::scissor_rects, D_NA(RenderState, scissor_rects))
        .def_rw("vertex_buffers", &RenderState::vertex_buffers, D_NA(RenderState, vertex_buffers))
        .def_rw("index_buffer", &RenderState::index_buffer, D_NA(RenderState, index_buffer))
        .def_rw("index_format", &RenderState::index_format, D_NA(RenderState, index_format));
    nb::implicitly_convertible<nb::dict, RenderState>();

    nb::class_<RenderPassColorAttachment>(m, "RenderPassColorAttachment", D_NA(RenderPassColorAttachment))
        .def(nb::init<>())
        .def(
            "__init__",
            [](RenderPassColorAttachment* self, nb::dict dict)
            { new (self) RenderPassColorAttachment(dict_to_RenderPassColorAttachment(dict)); }
        )
        .def_rw("view", &RenderPassColorAttachment::view, D_NA(RenderPassColorAttachment, view))
        .def_rw(
            "resolve_target",
            &RenderPassColorAttachment::resolve_target,
            D_NA(RenderPassColorAttachment, resolve_target)
        )
        .def_rw("load_op", &RenderPassColorAttachment::load_op, D_NA(RenderPassColorAttachment, load_op))
        .def_rw("store_op", &RenderPassColorAttachment::store_op, D_NA(RenderPassColorAttachment, store_op))
        .def_rw("clear_value", &RenderPassColorAttachment::clear_value, D_NA(RenderPassColorAttachment, clear_value));
    nb::implicitly_convertible<nb::dict, RenderPassColorAttachment>();

    nb::class_<RenderPassDepthStencilAttachment>(
        m,
        "RenderPassDepthStencilAttachment",
        D_NA(RenderPassDepthStencilAttachment)
    )
        .def(nb::init<>())
        .def(
            "__init__",
            [](RenderPassDepthStencilAttachment* self, nb::dict dict)
            { new (self) RenderPassDepthStencilAttachment(dict_to_RenderPassDepthStencilAttachment(dict)); }
        )
        .def_rw("view", &RenderPassDepthStencilAttachment::view, D_NA(RenderPassDepthStencilAttachment, view))
        .def_rw(
            "depth_load_op",
            &RenderPassDepthStencilAttachment::depth_load_op,
            D_NA(RenderPassDepthStencilAttachment, depth_load_op)
        )
        .def_rw(
            "depth_store_op",
            &RenderPassDepthStencilAttachment::depth_store_op,
            D_NA(RenderPassDepthStencilAttachment, depth_store_op)
        )
        .def_rw(
            "depth_clear_value",
            &RenderPassDepthStencilAttachment::depth_clear_value,
            D_NA(RenderPassDepthStencilAttachment, depth_clear_value)
        )
        .def_rw(
            "depth_read_only",
            &RenderPassDepthStencilAttachment::depth_read_only,
            D_NA(RenderPassDepthStencilAttachment, depth_read_only)
        )
        .def_rw(
            "stencil_load_op",
            &RenderPassDepthStencilAttachment::stencil_load_op,
            D_NA(RenderPassDepthStencilAttachment, stencil_load_op)
        )
        .def_rw(
            "stencil_store_op",
            &RenderPassDepthStencilAttachment::stencil_store_op,
            D_NA(RenderPassDepthStencilAttachment, stencil_store_op)
        )
        .def_rw(
            "stencil_clear_value",
            &RenderPassDepthStencilAttachment::stencil_clear_value,
            D_NA(RenderPassDepthStencilAttachment, stencil_clear_value)
        )
        .def_rw(
            "stencil_read_only",
            &RenderPassDepthStencilAttachment::stencil_read_only,
            D_NA(RenderPassDepthStencilAttachment, stencil_read_only)
        );
    nb::implicitly_convertible<nb::dict, RenderPassDepthStencilAttachment>();

    nb::class_<RenderPassDesc>(m, "RenderPassDesc", D_NA(RenderPassDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](RenderPassDesc* self, nb::dict dict) { new (self) RenderPassDesc(dict_to_RenderPassDesc(dict)); }
        )
        .def_rw("color_attachments", &RenderPassDesc::color_attachments, D_NA(RenderPassDesc, color_attachments))
        .def_rw(
            "depth_stencil_attachment",
            &RenderPassDesc::depth_stencil_attachment,
            D_NA(RenderPassDesc, depth_stencil_attachment)
        );
    nb::implicitly_convertible<nb::dict, RenderPassDesc>();

    nb::class_<CommandEncoder, DeviceResource>(m, "CommandEncoder", D_NA(CommandEncoder))
        .def("begin_render_pass", &CommandEncoder::begin_render_pass, "desc"_a, D_NA(CommandEncoder, begin_render_pass))
        .def("begin_compute_pass", &CommandEncoder::begin_compute_pass, D_NA(CommandEncoder, begin_compute_pass))
        .def(
            "begin_ray_tracing_pass",
            &CommandEncoder::begin_ray_tracing_pass,
            D_NA(CommandEncoder, begin_ray_tracing_pass)
        )
        .def(
            "copy_buffer",
            &CommandEncoder::copy_buffer,
            "dst"_a,
            "dst_offset"_a,
            "src"_a,
            "src_offset"_a,
            "size"_a,
            D_NA(CommandEncoder, copy_buffer)
        )
        .def(
            "copy_texture",
            &CommandEncoder::copy_texture,
            "dst"_a,
            "dst_subresource"_a,
            "dst_offset"_a,
            "src"_a,
            "src_subresource"_a,
            "src_offset"_a,
            "extent"_a = uint3(-1),
            D_NA(CommandEncoder, copy_texture)
        )
        // TODO(slang-rhi)
        // copy_texture_to_buffer
        // upload_buffer_data
        // upload_texture_data
        .def(
            "clear_buffer",
            &CommandEncoder::clear_buffer,
            "buffer"_a,
            "range"_a = BufferRange{},
            D_NA(CommandEncoder, clear_buffer)
        )
    // TODO(slang-rhi)
#if 0
        .def(
            "clear_texture",
            nb::overload_cast<Texture*, float4>(&CommandEncoder::clear_texture),
            "texture"_a,
            "clear_value"_a,
            "range" D(CommandEncoder, clear_texture)
        )
        .def(
            "clear_texture",
            nb::overload_cast<Texture*, uint4>(&CommandEncoder::clear_texture),
            "texture"_a,
            "clear_value"_a,
            D(CommandEncoder, clear_texture, 2)
        )
#endif
        .def(
            "blit",
            nb::overload_cast<TextureView*, TextureView*, TextureFilteringMode>(&CommandEncoder::blit),
            "dst"_a,
            "src"_a,
            "filter"_a = TextureFilteringMode::linear,
            D_NA(CommandEncoder, blit)
        )
        .def(
            "blit",
            nb::overload_cast<Texture*, Texture*, TextureFilteringMode>(&CommandEncoder::blit),
            "dst"_a,
            "src"_a,
            "filter"_a = TextureFilteringMode::linear,
            D_NA(CommandEncoder, blit, 2)
        )
        .def(
            "resolve_query",
            &CommandEncoder::resolve_query,
            "query_pool"_a,
            "index"_a,
            "count"_a,
            "buffer"_a,
            "offset"_a,
            D_NA(CommandEncoder, resolve_query)
        )
        .def(
            "build_acceleration_structure",
            &CommandEncoder::build_acceleration_structure,
            "desc"_a,
            "dst"_a,
            "src"_a.none(),
            "scratch_buffer"_a,
            "queries"_a = std::span<AccelerationStructureQueryDesc>(),
            D_NA(CommandEncoder, build_acceleration_structure)
        )
        .def(
            "copy_acceleration_structure",
            &CommandEncoder::copy_acceleration_structure,
            "src"_a,
            "dst"_a,
            "mode"_a,
            D_NA(CommandEncoder, copy_acceleration_structure)
        )
        .def(
            "query_acceleration_structure_properties",
            &CommandEncoder::query_acceleration_structure_properties,
            "acceleration_structures"_a,
            "queries"_a,
            D_NA(CommandEncoder, query_acceleration_structure_properties)
        )
        .def(
            "serialize_acceleration_structure",
            &CommandEncoder::serialize_acceleration_structure,
            "dst"_a,
            "src"_a,
            D_NA(CommandEncoder, serialize_acceleration_structure)
        )
        .def(
            "deserialize_acceleration_structure",
            &CommandEncoder::deserialize_acceleration_structure,
            "dst"_a,
            "src"_a,
            D_NA(CommandEncoder, deserialize_acceleration_structure)
        )
        .def(
            "set_buffer_state",
            &CommandEncoder::set_buffer_state,
            "buffer"_a,
            "state"_a,
            D_NA(CommandEncoder, set_buffer_state)
        )
        .def(
            "set_texture_state",
            nb::overload_cast<Texture*, ResourceState>(&CommandEncoder::set_texture_state),
            "texture"_a,
            "state"_a,
            D_NA(CommandEncoder, set_texture_state)
        )
        .def(
            "set_texture_state",
            nb::overload_cast<Texture*, SubresourceRange, ResourceState>(&CommandEncoder::set_texture_state),
            "texture"_a,
            "range"_a,
            "state"_a,
            D_NA(CommandEncoder, set_texture_state)
        )
        .def(
            "push_debug_group",
            &CommandEncoder::push_debug_group,
            "name"_a,
            "color"_a,
            D_NA(CommandEncoder, push_debug_group)
        )
        .def("pop_debug_group", &CommandEncoder::pop_debug_group, D_NA(CommandEncoder, pop_debug_group))
        .def(
            "insert_debug_marker",
            &CommandEncoder::insert_debug_marker,
            "name"_a,
            "color"_a,
            D_NA(CommandEncoder, insert_debug_marker)
        )
        .def(
            "write_timestamp",
            &CommandEncoder::write_timestamp,
            "query_pool"_a,
            "index"_a,
            D_NA(CommandEncoder, write_timestamp)
        )
        .def("finish", &CommandEncoder::finish, D_NA(CommandEncoder, finish));

    nb::class_<PassEncoder, Object>(m, "PassEncoder", D_NA(PassEncoder))
        .def("end", &PassEncoder::end, D_NA(PassEncoder, end))
        .def(
            "push_debug_group",
            &PassEncoder::push_debug_group,
            "name"_a,
            "color"_a,
            D_NA(PassEncoder, push_debug_group)
        )
        .def("pop_debug_group", &PassEncoder::pop_debug_group, D_NA(PassEncoder, pop_debug_group))
        .def(
            "insert_debug_marker",
            &PassEncoder::insert_debug_marker,
            "name"_a,
            "color"_a,
            D_NA(PassEncoder, insert_debug_marker)
        );

    nb::class_<RenderPassEncoder, PassEncoder>(m, "RenderPassEncoder", D_NA(RenderPassEncoder))
        .def("__enter__", [](RenderPassEncoder* self) { return self; })
        .def(
            "__exit__",
            [](RenderPassEncoder* self, nb::object, nb::object, nb::object) { self->end(); },
            "exc_type"_a = nb::none(),
            "exc_value"_a = nb::none(),
            "traceback"_a = nb::none()
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<RenderPipeline*>(&RenderPassEncoder::bind_pipeline),
            "pipeline"_a,
            D_NA(RenderPassEncoder, bind_pipeline)
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<RenderPipeline*, ShaderObject*>(&RenderPassEncoder::bind_pipeline),
            "pipeline"_a,
            "root_object"_a,
            D_NA(RenderPassEncoder, bind_pipeline, 2)
        )
        .def(
            "set_render_state",
            &RenderPassEncoder::set_render_state,
            "state"_a,
            D_NA(RenderPassEncoder, set_render_state)
        )
        .def("draw", &RenderPassEncoder::draw, "args"_a, D_NA(RenderPassEncoder, draw))
        .def("draw_indexed", &RenderPassEncoder::draw_indexed, "args"_a, D_NA(RenderPassEncoder, draw_indexed))
        .def(
            "draw_indirect",
            &RenderPassEncoder::draw_indirect,
            "max_draw_count"_a,
            "arg_buffer"_a,
            "count_buffer"_a = BufferOffsetPair{},
            D_NA(RenderPassEncoder, draw_indirect)
        )
        .def(
            "draw_indexed_indirect",
            &RenderPassEncoder::draw_indexed_indirect,
            "max_draw_count"_a,
            "arg_buffer"_a,
            "count_buffer"_a = BufferOffsetPair{},
            D_NA(RenderPassEncoder, draw_indexed_indirect)
        )
        .def(
            "draw_mesh_tasks",
            &RenderPassEncoder::draw_mesh_tasks,
            "dimensions"_a,
            D_NA(RenderPassEncoder, draw_mesh_tasks)
        );


    nb::class_<ComputePassEncoder, PassEncoder>(m, "ComputePassEncoder", D_NA(ComputePassEncoder))
        .def("__enter__", [](ComputePassEncoder* self) { return self; })
        .def(
            "__exit__",
            [](ComputePassEncoder* self, nb::object, nb::object, nb::object) { self->end(); },
            "exc_type"_a = nb::none(),
            "exc_value"_a = nb::none(),
            "traceback"_a = nb::none()
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<ComputePipeline*>(&ComputePassEncoder::bind_pipeline),
            "pipeline"_a,
            D_NA(ComputePassEncoder, bind_pipeline)
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<ComputePipeline*, ShaderObject*>(&ComputePassEncoder::bind_pipeline),
            "pipeline"_a,
            "root_object"_a,
            D_NA(ComputePassEncoder, bind_pipeline, 2)
        )
        .def("dispatch", &ComputePassEncoder::dispatch, "thread_count"_a, D_NA(ComputePassEncoder, dispatch))
        .def(
            "dispatch_compute",
            &ComputePassEncoder::dispatch_compute,
            "thread_group_count"_a,
            D_NA(ComputePassEncoder, dispatch_compute)
        )
        .def(
            "dispatch_compute_indirect",
            &ComputePassEncoder::dispatch_compute_indirect,
            "arg_buffer"_a,
            D_NA(ComputePassEncoder, dispatch_compute_indirect)
        );

    nb::class_<RayTracingPassEncoder, PassEncoder>(m, "RayTracingPassEncoder", D_NA(RayTracingPassEncoder))
        .def("__enter__", [](RayTracingPassEncoder* self) { return self; })
        .def(
            "__exit__",
            [](RayTracingPassEncoder* self, nb::object, nb::object, nb::object) { self->end(); },
            "exc_type"_a = nb::none(),
            "exc_value"_a = nb::none(),
            "traceback"_a = nb::none()
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<RayTracingPipeline*, ShaderTable*>(&RayTracingPassEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_table"_a,
            D_NA(RayTracingPassEncoder, bind_pipeline)
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<RayTracingPipeline*, ShaderTable*, ShaderObject*>(&RayTracingPassEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_table"_a,
            "root_object"_a,
            D_NA(RayTracingPassEncoder, bind_pipeline, 2)
        )
        .def(
            "dispatch_rays",
            &RayTracingPassEncoder::dispatch_rays,
            "ray_gen_shader_index"_a,
            "dimensions"_a,
            D_NA(RayTracingPassEncoder, dispatch_rays)
        );

#if 0
    nb::class_<ComputeCommandEncoder>(m, "ComputeCommandEncoder", D(ComputeCommandEncoder))
        .def("__enter__", [](ComputeCommandEncoder* self) { return self; })
        .def(
            "__exit__",
            [](ComputeCommandEncoder* self, nb::object, nb::object, nb::object) { self->end(); },
            "exc_type"_a = nb::none(),
            "exc_value"_a = nb::none(),
            "traceback"_a = nb::none()
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const ComputePipeline*>(&ComputeCommandEncoder::bind_pipeline),
            "pipeline"_a,
            D(ComputeCommandEncoder, bind_pipeline)
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const ComputePipeline*, const ShaderObject*>(&ComputeCommandEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a,
            D(ComputeCommandEncoder, bind_pipeline, 2)
        )
        .def("dispatch", &ComputeCommandEncoder::dispatch, "thread_count"_a, D(ComputeCommandEncoder, dispatch))
        .def(
            "dispatch_thread_groups",
            &ComputeCommandEncoder::dispatch_thread_groups,
            "thread_group_count"_a,
            D(ComputeCommandEncoder, dispatch_thread_groups)
        );

    nb::class_<RenderCommandEncoder>(m, "RenderCommandEncoder", D(RenderCommandEncoder))
        .def("__enter__", [](RenderCommandEncoder* self) { return self; })
        .def(
            "__exit__",
            [](RenderCommandEncoder* self, nb::object, nb::object, nb::object) { self->end(); },
            "exc_type"_a = nb::none(),
            "exc_value"_a = nb::none(),
            "traceback"_a = nb::none()
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const GraphicsPipeline*>(&RenderCommandEncoder::bind_pipeline),
            "pipeline"_a,
            D(RenderCommandEncoder, bind_pipeline)
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const GraphicsPipeline*, const ShaderObject*>(&RenderCommandEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a,
            D(RenderCommandEncoder, bind_pipeline, 2)
        )
        .def(
            "set_viewports",
            &RenderCommandEncoder::set_viewports,
            "viewports"_a,
            D(RenderCommandEncoder, set_viewports)
        )
        .def(
            "set_scissor_rects",
            &RenderCommandEncoder::set_scissor_rects,
            "scissor_rects"_a,
            D(RenderCommandEncoder, set_scissor_rects)
        )
        .def(
            "set_viewport_and_scissor_rect",
            &RenderCommandEncoder::set_viewport_and_scissor_rect,
            "viewport"_a,
            D(RenderCommandEncoder, set_viewport_and_scissor_rect)
        )
        .def(
            "set_primitive_topology",
            &RenderCommandEncoder::set_primitive_topology,
            "topology"_a,
            D(RenderCommandEncoder, set_primitive_topology)
        )
        .def(
            "set_stencil_reference",
            &RenderCommandEncoder::set_stencil_reference,
            "reference_value"_a,
            D(RenderCommandEncoder, set_stencil_reference)
        )
        .def(
            "set_vertex_buffer",
            &RenderCommandEncoder::set_vertex_buffer,
            "slot"_a,
            "buffer"_a,
            "offset"_a = 0,
            D(RenderCommandEncoder, set_vertex_buffer)
        )
        .def(
            "set_index_buffer",
            &RenderCommandEncoder::set_index_buffer,
            "buffer"_a,
            "index_format"_a,
            "offset"_a = 0,
            D(RenderCommandEncoder, set_index_buffer)
        )
        .def("draw", &RenderCommandEncoder::draw, "vertex_count"_a, "start_vertex"_a = 0, D(RenderCommandEncoder, draw))
        .def(
            "draw_indexed",
            &RenderCommandEncoder::draw_indexed,
            "index_count"_a,
            "start_index"_a = 0,
            "base_vertex"_a = 0,
            D(RenderCommandEncoder, draw_indexed)
        )
        .def(
            "draw_instanced",
            &RenderCommandEncoder::draw_instanced,
            "vertex_count"_a,
            "instance_count"_a,
            "start_vertex"_a = 0,
            "start_instance"_a = 0,
            D(RenderCommandEncoder, draw_instanced)
        )
        .def(
            "draw_indexed_instanced",
            &RenderCommandEncoder::draw_indexed_instanced,
            "index_count"_a,
            "instance_count"_a,
            "start_index"_a = 0,
            "base_vertex"_a = 0,
            "start_instance"_a = 0,
            D(RenderCommandEncoder, draw_indexed_instanced)
        )
        .def(
            "draw_indirect",
            &RenderCommandEncoder::draw_indirect,
            "max_draw_count"_a,
            "arg_buffer"_a,
            "arg_offset"_a,
            "count_buffer"_a = nullptr,
            "count_offset"_a = 0,
            D(RenderCommandEncoder, draw_indirect)
        )
        .def(
            "draw_indexed_indirect",
            &RenderCommandEncoder::draw_indexed_indirect,
            "max_draw_count"_a,
            "arg_buffer"_a,
            "arg_offset"_a,
            "count_buffer"_a = nullptr,
            "count_offset"_a = 0,
            D(RenderCommandEncoder, draw_indexed_indirect)
        );

    nb::class_<RayTracingCommandEncoder>(m, "RayTracingCommandEncoder", D(RayTracingCommandEncoder))
        .def("__enter__", [](RayTracingCommandEncoder* self) { return self; })
        .def(
            "__exit__",
            [](RayTracingCommandEncoder* self, nb::object, nb::object, nb::object) { self->end(); },
            "exc_type"_a = nb::none(),
            "exc_value"_a = nb::none(),
            "traceback"_a = nb::none()
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const RayTracingPipeline*>(&RayTracingCommandEncoder::bind_pipeline),
            "pipeline"_a,
            D(RayTracingCommandEncoder, bind_pipeline)
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const RayTracingPipeline*, const ShaderObject*>(&RayTracingCommandEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a,
            D(RayTracingCommandEncoder, bind_pipeline, 2)
        )
        .def(
            "dispatch_rays",
            &RayTracingCommandEncoder::dispatch_rays,
            "ray_gen_shader_index"_a,
            "shader_table"_a,
            "dimensions"_a,
            D(RayTracingCommandEncoder, dispatch_rays)
        )

#endif
    nb::class_<CommandBuffer, DeviceResource>(m, "CommandBuffer", D(CommandBuffer));
}
