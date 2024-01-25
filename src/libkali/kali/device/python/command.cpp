#include "nanobind.h"

#include "kali/device/command.h"
#include "kali/device/query.h"
#include "kali/device/pipeline.h"
#include "kali/device/framebuffer.h"
#include "kali/device/shader_object.h"
#include "kali/device/raytracing.h"

KALI_PY_EXPORT(device_command)
{
    using namespace kali;

    nb::kali_enum<CommandQueueType>(m, "CommandQueueType");

    nb::class_<CommandQueueDesc>(m, "CommandQueueDesc").def(nb::init<>()).def_rw("type", &CommandQueueDesc::type);

    nb::class_<CommandQueue, DeviceResource>(m, "CommandQueue")
        .def("desc", &CommandQueue::desc)
        .def("submit", nb::overload_cast<const CommandBuffer*>(&CommandQueue::submit), "command_buffer"_a)
        .def(
            "submit_and_wait",
            nb::overload_cast<const CommandBuffer*>(&CommandQueue::submit_and_wait),
            "command_buffer"_a
        )
        .def("wait", nb::overload_cast<>(&CommandQueue::wait))
        .def("signal", &CommandQueue::signal, "fence"_a, "value"_a = Fence::AUTO)
        .def(
            "wait",
            nb::overload_cast<const Fence*, uint64_t>(&CommandQueue::wait),
            "fence"_a,
            "value"_a = Fence::AUTO
        );

    nb::class_<CommandBuffer, DeviceResource>(m, "CommandBuffer")
        .def("close", &CommandBuffer::close)
        .def("submit", &CommandBuffer::submit)
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
            "buffer_barrier",
            nb::overload_cast<const Buffer*, ResourceState>(&CommandBuffer::buffer_barrier),
            "buffer"_a,
            "new_state"_a
        )
        .def(
            "texture_barrier",
            nb::overload_cast<const Texture*, ResourceState>(&CommandBuffer::texture_barrier),
            "texture"_a,
            "new_state"_a
        )
        .def("uav_barrier", &CommandBuffer::uav_barrier, "resource"_a)
        // .def(
        //     "buffer_barrier",
        //     nb::overload_cast<std::span<Buffer*>, ResourceState,
        //     ResourceState>(&CommandBuffer::buffer_barrier
        //     ),
        //     "buffers"_a,
        //     "old_state"_a,
        //     "new_state"_a
        // )
        .def(
            "buffer_barrier",
            nb::overload_cast<const Buffer*, ResourceState, ResourceState>(&CommandBuffer::buffer_barrier),
            "buffer"_a,
            "old_state"_a,
            "new_state"_a
        )
        // .def(
        //     "texture_barrier",
        //     nb::overload_cast<std::span<Texture*>, ResourceState, ResourceState>(
        //         &CommandBuffer::texture_barrier
        //     ),
        //     "textures"_a,
        //     "old_state"_a,
        //     "new_state"_a
        // )
        .def(
            "texture_barrier",
            nb::overload_cast<const Texture*, ResourceState, ResourceState>(&CommandBuffer::texture_barrier),
            "texture"_a,
            "old_state"_a,
            "new_state"_a
        )
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
        .def("begin_compute_pass", &CommandBuffer::begin_compute_pass, nb::rv_policy::reference_internal)
        .def("begin_render_pass", &CommandBuffer::begin_render_pass, nb::rv_policy::reference_internal)
        .def("begin_ray_tracing_pass", &CommandBuffer::begin_ray_tracing_pass, nb::rv_policy::reference_internal);

    nb::class_<ComputePassEncoder>(m, "ComputePassEncoder")
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
            nb::overload_cast<const ComputePipeline*>(&ComputePassEncoder::bind_pipeline),
            "pipeline"_a
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const ComputePipeline*, const ShaderObject*>(&ComputePassEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a
        )
        .def("dispatch", &ComputePassEncoder::dispatch, "thread_count"_a)
        .def("dispatch_thread_groups", &ComputePassEncoder::dispatch_thread_groups, "thread_group_count"_a);

    nb::class_<RenderPassEncoder>(m, "RenderPassEncoder")
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
            nb::overload_cast<const GraphicsPipeline*>(&RenderPassEncoder::bind_pipeline),
            "pipeline"_a
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const GraphicsPipeline*, const ShaderObject*>(&RenderPassEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a
        )
        .def("set_viewports", &RenderPassEncoder::set_viewports, "viewports"_a)
        .def("set_scissor_rects", &RenderPassEncoder::set_scissor_rects, "scissor_rects"_a)
        .def("set_viewport_and_scissor_rect", &RenderPassEncoder::set_viewport_and_scissor_rect, "viewport"_a)
        .def("set_primitive_topology", &RenderPassEncoder::set_primitive_topology, "topology"_a)
        .def("set_stencil_reference", &RenderPassEncoder::set_stencil_reference, "reference_value"_a)
        .def("set_vertex_buffer", &RenderPassEncoder::set_vertex_buffer, "slot"_a, "buffer"_a, "offset"_a = 0)
        .def("set_index_buffer", &RenderPassEncoder::set_index_buffer, "buffer"_a, "index_format"_a, "offset"_a = 0)
        .def("draw", &RenderPassEncoder::draw, "vertex_count"_a, "start_vertex"_a = 0);


    nb::class_<RayTracingPassEncoder>(m, "RayTracingPassEncoder")
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
            nb::overload_cast<const RayTracingPipeline*>(&RayTracingPassEncoder::bind_pipeline),
            "pipeline"_a
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const RayTracingPipeline*, const ShaderObject*>(&RayTracingPassEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a
        )
        .def(
            "dispatch_rays",
            &RayTracingPassEncoder::dispatch_rays,
            "ray_gen_shader_index"_a,
            "shader_table"_a,
            "dimensions"_a
        )
        .def(
            "build_acceleration_structure",
            [](RayTracingPassEncoder* self,
               const AccelerationStructureBuildInputs& inputs,
               AccelerationStructure* src,
               AccelerationStructure* dst,
               DeviceAddress scratch_data) {
                self->build_acceleration_structure(
                    {.inputs = inputs, .src = src, .dst = dst, .scratch_data = scratch_data}
                );
            },
            "inputs"_a,
            "src"_a = nullptr,
            "dst"_a,
            "scratch_data"_a
        )
        .def(
            "copy_acceleration_structure",
            &RayTracingPassEncoder::copy_acceleration_structure,
            "src"_a,
            "dst"_a,
            "mode"_a
        );
}
