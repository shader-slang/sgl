// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/command.h"
#include "sgl/device/query.h"
#include "sgl/device/pipeline.h"
#include "sgl/device/framebuffer.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/raytracing.h"

SGL_PY_EXPORT(device_command)
{
    using namespace sgl;

    nb::class_<CommandBuffer, DeviceResource>(m, "CommandBuffer")
        .def("open", &CommandBuffer::open, D(CommandBuffer, open))
        .def("close", &CommandBuffer::close, D(CommandBuffer, close))
        .def("submit", &CommandBuffer::submit, "queue"_a = CommandQueueType::graphics, D(CommandBuffer, submit))
        .def("write_timestamp", &CommandBuffer::write_timestamp, "query_pool"_a, "index"_a)
        .def(
            "resolve_query",
            &CommandBuffer::resolve_query,
            "query_pool"_a,
            "index"_a,
            "count"_a,
            "buffer"_a,
            "offset"_a
        )
        .def(
            "set_resource_state",
            nb::overload_cast<const Resource*, ResourceState>(&CommandBuffer::set_resource_state),
            "resource"_a,
            "new_state"_a,
            D(CommandBuffer, set_resource_state)
        )
        .def(
            "set_resource_state",
            nb::overload_cast<const ResourceView*, ResourceState>(&CommandBuffer::set_resource_state),
            "resource_view"_a,
            "new_state"_a,
            D(CommandBuffer, set_resource_state, 2)
        )
        .def(
            "set_buffer_state",
            &CommandBuffer::set_buffer_state,
            "buffer"_a,
            "new_state"_a,
            D(CommandBuffer, set_buffer_state)
        )
        .def(
            "set_texture_state",
            &CommandBuffer::set_texture_state,
            "texture"_a,
            "new_state"_a,
            D(CommandBuffer, set_texture_state)
        )
        .def("uav_barrier", &CommandBuffer::uav_barrier, "resource"_a, D(CommandBuffer, uav_barrier))
        .def(
            "clear_resource_view",
            nb::overload_cast<ResourceView*, float4>(&CommandBuffer::clear_resource_view),
            "resource_view"_a,
            "clear_value"_a
        )
        .def(
            "clear_resource_view",
            nb::overload_cast<ResourceView*, uint4>(&CommandBuffer::clear_resource_view),
            "resource_view"_a,
            "clear_value"_a
        )
        .def(
            "clear_resource_view",
            nb::overload_cast<ResourceView*, float, uint32_t, bool, bool>(&CommandBuffer::clear_resource_view),
            "resource_view"_a,
            "depth_value"_a,
            "stencil_value"_a,
            "clear_depth"_a,
            "clear_stencil"_a
        )
        .def(
            "clear_texture",
            nb::overload_cast<Texture*, float4>(&CommandBuffer::clear_texture),
            "texture"_a,
            "clear_value"_a
        )
        .def(
            "clear_texture",
            nb::overload_cast<Texture*, uint4>(&CommandBuffer::clear_texture),
            "texture"_a,
            "clear_value"_a
        )
        .def("copy_resource", &CommandBuffer::copy_resource, "dst"_a, "src"_a)
        .def(
            "copy_buffer_region",
            &CommandBuffer::copy_buffer_region,
            "dst"_a,
            "dst_offset"_a,
            "src"_a,
            "src_offset"_a,
            "size"_a
        )
        .def(
            "copy_texture_region",
            &CommandBuffer::copy_texture_region,
            "dst"_a,
            "dst_subresource"_a,
            "dst_offset"_a,
            "src"_a,
            "src_subresource"_a,
            "src_offset"_a,
            "extent"_a = uint3(-1)
        )
        .def("encode_compute_commands", &CommandBuffer::encode_compute_commands, nb::rv_policy::reference_internal)
        .def("encode_render_commands", &CommandBuffer::encode_render_commands, nb::rv_policy::reference_internal)
        .def(
            "encode_ray_tracing_commands",
            &CommandBuffer::encode_ray_tracing_commands,
            nb::rv_policy::reference_internal
        );

    nb::class_<ComputeCommandEncoder>(m, "ComputeCommandEncoder")
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
            "pipeline"_a
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const ComputePipeline*, const ShaderObject*>(&ComputeCommandEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a
        )
        .def("dispatch", &ComputeCommandEncoder::dispatch, "thread_count"_a)
        .def("dispatch_thread_groups", &ComputeCommandEncoder::dispatch_thread_groups, "thread_group_count"_a);

    nb::class_<RenderCommandEncoder>(m, "RenderCommandEncoder")
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
            "pipeline"_a
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const GraphicsPipeline*, const ShaderObject*>(&RenderCommandEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a
        )
        .def("set_viewports", &RenderCommandEncoder::set_viewports, "viewports"_a)
        .def("set_scissor_rects", &RenderCommandEncoder::set_scissor_rects, "scissor_rects"_a)
        .def("set_viewport_and_scissor_rect", &RenderCommandEncoder::set_viewport_and_scissor_rect, "viewport"_a)
        .def("set_primitive_topology", &RenderCommandEncoder::set_primitive_topology, "topology"_a)
        .def("set_stencil_reference", &RenderCommandEncoder::set_stencil_reference, "reference_value"_a)
        .def("set_vertex_buffer", &RenderCommandEncoder::set_vertex_buffer, "slot"_a, "buffer"_a, "offset"_a = 0)
        .def("set_index_buffer", &RenderCommandEncoder::set_index_buffer, "buffer"_a, "index_format"_a, "offset"_a = 0)
        .def("draw", &RenderCommandEncoder::draw, "vertex_count"_a, "start_vertex"_a = 0)
        .def(
            "draw_indexed",
            &RenderCommandEncoder::draw_indexed,
            "index_count"_a,
            "start_index"_a = 0,
            "base_vertex"_a = 0
        )
        .def(
            "draw_instanced",
            &RenderCommandEncoder::draw_instanced,
            "vertex_count"_a,
            "instance_count"_a,
            "start_vertex"_a = 0,
            "start_instance"_a = 0
        )
        .def(
            "draw_indexed_instanced",
            &RenderCommandEncoder::draw_indexed_instanced,
            "index_count"_a,
            "instance_count"_a,
            "start_index"_a = 0,
            "base_vertex"_a = 0,
            "start_instance"_a = 0
        )
        .def(
            "draw_indirect",
            &RenderCommandEncoder::draw_indirect,
            "max_draw_count"_a,
            "arg_buffer"_a,
            "arg_offset"_a,
            "count_buffer"_a = nullptr,
            "count_offset"_a = 0
        )
        .def(
            "draw_indexed_indirect",
            &RenderCommandEncoder::draw_indexed_indirect,
            "max_draw_count"_a,
            "arg_buffer"_a,
            "arg_offset"_a,
            "count_buffer"_a = nullptr,
            "count_offset"_a = 0
        );

    nb::class_<RayTracingCommandEncoder>(m, "RayTracingCommandEncoder")
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
            "pipeline"_a
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const RayTracingPipeline*, const ShaderObject*>(&RayTracingCommandEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a
        )
        .def(
            "dispatch_rays",
            &RayTracingCommandEncoder::dispatch_rays,
            "ray_gen_shader_index"_a,
            "shader_table"_a,
            "dimensions"_a
        )
        .def(
            "build_acceleration_structure",
            [](RayTracingCommandEncoder* self,
               const AccelerationStructureBuildInputs& inputs,
               AccelerationStructure* dst,
               DeviceAddress scratch_data,
               AccelerationStructure* src) {
                self->build_acceleration_structure(
                    {.inputs = inputs, .src = src, .dst = dst, .scratch_data = scratch_data}
                );
            },
            "inputs"_a,
            "dst"_a,
            "scratch_data"_a,
            "src"_a = nullptr
        )
        .def(
            "copy_acceleration_structure",
            &RayTracingCommandEncoder::copy_acceleration_structure,
            "src"_a,
            "dst"_a,
            "mode"_a
        );
}
