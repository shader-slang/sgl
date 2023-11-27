#include "nanobind.h"

#include "kali/device/command.h"
#include "kali/device/pipeline.h"

KALI_PY_EXPORT(device_command)
{
    using namespace kali;

    nb::kali_enum<CommandQueueType>(m, "CommandQueueType");

    nb::class_<CommandQueueDesc>(m, "CommandQueueDesc").def(nb::init<>()).def_rw("type", &CommandQueueDesc::type);

    nb::class_<CommandQueue, DeviceResource>(m, "CommandQueue").def("desc", &CommandQueue::desc);

    nb::class_<CommandBuffer, Object>(m, "CommandBuffer");

    nb::class_<CommandEncoder>(m, "CommandEncoder");

    nb::class_<ResourceCommandEncoder, CommandEncoder>(m, "ResourceCommandEncoder")
        // .def(
        //     "buffer_barrier",
        //     nb::overload_cast<std::span<Buffer*>, ResourceState,
        //     ResourceState>(&ResourceCommandEncoder::buffer_barrier
        //     ),
        //     "buffers"_a,
        //     "old_state"_a,
        //     "new_state"_a
        // )
        .def(
            "buffer_barrier",
            nb::overload_cast<const Buffer*, ResourceState, ResourceState>(&ResourceCommandEncoder::buffer_barrier),
            "buffer"_a,
            "old_state"_a,
            "new_state"_a
        )
        // .def(
        //     "texture_barrier",
        //     nb::overload_cast<std::span<Texture*>, ResourceState, ResourceState>(
        //         &ResourceCommandEncoder::texture_barrier
        //     ),
        //     "textures"_a,
        //     "old_state"_a,
        //     "new_state"_a
        // )
        .def(
            "texture_barrier",
            nb::overload_cast<const Texture*, ResourceState, ResourceState>(&ResourceCommandEncoder::texture_barrier),
            "texture"_a,
            "old_state"_a,
            "new_state"_a
        );

    nb::class_<ComputeCommandEncoder, ResourceCommandEncoder>(m, "ComputeCommandEncoder")
        .def(
            "bind_pipeline",
            nb::overload_cast<const ComputePipelineState*>(&ComputeCommandEncoder::bind_pipeline),
            "pipeline"_a
        )
        .def(
            "bind_pipeline",
            nb::overload_cast<const ComputePipelineState*, const ShaderObject*>(&ComputeCommandEncoder::bind_pipeline),
            "pipeline"_a,
            "shader_object"_a
        )
        .def("dispatch_compute", &ComputeCommandEncoder::dispatch_compute, "thread_group_count"_a);

    nb::class_<CommandStream, Object>(m, "CommandStream")
        .def("submit", &CommandStream::submit)
        .def("signal", &CommandStream::signal, "fence"_a, "value"_a = Fence::AUTO)
        .def("wait", &CommandStream::wait, "fence"_a, "value"_a = Fence::AUTO)
        .def("buffer_barrier", &CommandStream::buffer_barrier, "buffer"_a, "new_state"_a)
        .def("texture_barrier", &CommandStream::texture_barrier, "texture"_a, "new_state"_a)
        .def("uav_barrier", &CommandStream::uav_barrier, "resource"_a);
    // .def("copy_resource", &CommandStream::copy_resource, "dst"_a, "src"_a)
    // .def("copy_subresource", &CommandStream::copy_subresource, "dst"_a, "dst_sub_index"_a, "src"_a,
    // "src_sub_index"_a) .def("copy_buffer_region", &CommandStream::copy_buffer_region, "dst"_a, "dst_offset"_a,
    // "src"_a, "src_offset"_a, "size"_a) .def("copy_texture_region", &CommandStream::copy_texture_region, "dst"_a,
    // "dst_sub_index"_a, "dst_offset"_a, "src"_a, "src_sub_index"_a, "src_offset"_a, "size"_a)
}
