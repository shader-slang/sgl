#include "nanobind.h"

#include "kali/device/command.h"
#include "kali/device/pipeline.h"
#include "kali/device/shader_object.h"

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
        .def("begin_compute_pass", &CommandStream::begin_compute_pass, nb::rv_policy::reference_internal);

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
            nb::overload_cast<const ComputePipelineState*>(&ComputePassEncoder::bind_pipeline),
            "pipeline"_a
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const ComputePipelineState*, const ShaderObject*>(&ComputePassEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a
        )
        .def("dispatch_thread_groups", &ComputePassEncoder::dispatch_thread_groups, "thread_group_count"_a);

    // nb::class_<CommandStream, Object>(m, "CommandStream")
    //     .def("submit", &CommandStream::submit)
    //     .def("signal", &CommandStream::signal, "fence"_a, "value"_a = Fence::AUTO)
    //     .def("wait", &CommandStream::wait, "fence"_a, "value"_a = Fence::AUTO)
    //     .def("buffer_barrier", &CommandStream::buffer_barrier, "buffer"_a, "new_state"_a)
    //     .def("texture_barrier", &CommandStream::texture_barrier, "texture"_a, "new_state"_a)
    //     .def("uav_barrier", &CommandStream::uav_barrier, "resource"_a);
    // .def("copy_resource", &CommandStream::copy_resource, "dst"_a, "src"_a)
    // .def("copy_subresource", &CommandStream::copy_subresource, "dst"_a, "dst_sub_index"_a, "src"_a,
    // "src_sub_index"_a) .def("copy_buffer_region", &CommandStream::copy_buffer_region, "dst"_a, "dst_offset"_a,
    // "src"_a, "src_offset"_a, "size"_a) .def("copy_texture_region", &CommandStream::copy_texture_region, "dst"_a,
    // "dst_sub_index"_a, "dst_offset"_a, "src"_a, "src_sub_index"_a, "src_offset"_a, "size"_a)
}
