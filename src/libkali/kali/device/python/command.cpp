#include "nanobind.h"

#include "kali/device/command.h"
#include "kali/device/query.h"
#include "kali/device/pipeline.h"
#include "kali/device/shader_object.h"
#include "kali/device/raytracing.h"

KALI_PY_EXPORT(device_command)
{
    using namespace kali;

    nb::kali_enum<CommandQueueType>(m, "CommandQueueType");

    nb::class_<CommandQueueDesc>(m, "CommandQueueDesc").def(nb::init<>()).def_rw("type", &CommandQueueDesc::type);

    nb::class_<CommandQueue, DeviceResource>(m, "CommandQueue")
        .def("desc", &CommandQueue::desc)
        .def("submit", &CommandQueue::submit, "command_buffer"_a)
        .def("wait", nb::overload_cast<>(&CommandQueue::wait))
        .def("signal", &CommandQueue::signal, "fence"_a, "value"_a = Fence::AUTO)
        .def(
            "wait",
            nb::overload_cast<const Fence*, uint64_t>(&CommandQueue::wait),
            "fence"_a,
            "value"_a = Fence::AUTO
        );

    nb::class_<CommandBuffer, DeviceResource>(m, "CommandBuffer");

    nb::class_<CommandStream, DeviceResource>(m, "CommandStream")
        .def("submit", &CommandStream::submit)
        .def("signal", &CommandStream::signal, "fence"_a, "value"_a = Fence::AUTO)
        .def(
            "wait",
            nb::overload_cast<const Fence*, uint64_t>(&CommandStream::wait),
            "fence"_a,
            "value"_a = Fence::AUTO
        )
        .def("write_timestamp", &CommandStream::write_timestamp, "query_pool"_a, "index"_a)
        .def(
            "resolve_query",
            &CommandStream::resolve_query,
            "query_pool"_a,
            "index"_a,
            "count"_a,
            "buffer"_a,
            "offset"_a
        )
        .def(
            "buffer_barrier",
            nb::overload_cast<const Buffer*, ResourceState>(&CommandStream::buffer_barrier),
            "buffer"_a,
            "new_state"_a
        )
        .def(
            "texture_barrier",
            nb::overload_cast<const Texture*, ResourceState>(&CommandStream::texture_barrier),
            "texture"_a,
            "new_state"_a
        )
        .def("uav_barrier", &CommandStream::uav_barrier, "resource"_a)
        // .def(
        //     "buffer_barrier",
        //     nb::overload_cast<std::span<Buffer*>, ResourceState,
        //     ResourceState>(&CommandStream::buffer_barrier
        //     ),
        //     "buffers"_a,
        //     "old_state"_a,
        //     "new_state"_a
        // )
        .def(
            "buffer_barrier",
            nb::overload_cast<const Buffer*, ResourceState, ResourceState>(&CommandStream::buffer_barrier),
            "buffer"_a,
            "old_state"_a,
            "new_state"_a
        )
        // .def(
        //     "texture_barrier",
        //     nb::overload_cast<std::span<Texture*>, ResourceState, ResourceState>(
        //         &CommandStream::texture_barrier
        //     ),
        //     "textures"_a,
        //     "old_state"_a,
        //     "new_state"_a
        // )
        .def(
            "texture_barrier",
            nb::overload_cast<const Texture*, ResourceState, ResourceState>(&CommandStream::texture_barrier),
            "texture"_a,
            "old_state"_a,
            "new_state"_a
        )
        .def("copy_resource", &CommandStream::copy_resource, "dst"_a, "src"_a)
        .def("begin_compute_pass", &CommandStream::begin_compute_pass, nb::rv_policy::reference_internal)
        .def("begin_ray_tracing_pass", &CommandStream::begin_ray_tracing_pass, nb::rv_policy::reference_internal);

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
        // .def(
        //     "dispatch_rays",
        //     &RayTracingPassEncoder::dispatch_rays,
        //     "ray_gen_shader_index"_a,
        //     "shader_table"_a,
        //     "dimensions"_a
        // );
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
