// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/command.h"
#include "sgl/device/query.h"
#include "sgl/device/pipeline.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/raytracing.h"

SGL_PY_EXPORT(device_command)
{
    using namespace sgl;

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
    // clear_buffer
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
        // TODO(slang-rhi)
        // .def(
        //     "build_acceleration_structure",
        //     [](CommandEncoder* self,
        //        const AccelerationStructureBuildInputs& inputs,
        //        AccelerationStructure* dst,
        //        DeviceAddress scratch_data,
        //        AccelerationStructure* src) {
        //         self->build_acceleration_structure(
        //             {.inputs = inputs, .src = src, .dst = dst, .scratch_data = scratch_data}
        //         );
        //     },
        //     "inputs"_a,
        //     "dst"_a,
        //     "scratch_data"_a,
        //     "src"_a = nullptr,
        //     D(CommandEncoder, build_acceleration_structure)
        // )
        // .def(
        //     "copy_acceleration_structure",
        //     &CommandEncoder::copy_acceleration_structure,
        //     "src"_a,
        //     "dst"_a,
        //     "mode"_a,
        //     D(CommandEncoder, copy_acceleration_structure)
        // )
        // TODO(slang-rhi)
        // query_acceleration_structure_properties
        // serialize_acceleration_structure
        // deserialize_acceleration_structure
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

    nb::class_<RenderPassEncoder, PassEncoder>(m, "RenderPassEncoder", D_NA(RenderPassEncoder));

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
            "dispatch_thread_groups",
            &ComputePassEncoder::dispatch_thread_groups,
            "thread_group_count"_a,
            D_NA(ComputePassEncoder, dispatch_thread_groups)
        )
        .def(
            "dispatch_thread_groups_indirect",
            &ComputePassEncoder::dispatch_thread_groups_indirect,
            "arg_buffer"_a,
            D_NA(ComputePassEncoder, dispatch_thread_groups_indirect)
        );

    nb::class_<RayTracingPassEncoder, PassEncoder>(m, "RayTracingPassEncoder", D_NA(RayTracingPassEncoder))
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
