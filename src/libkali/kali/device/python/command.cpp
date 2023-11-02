#include "nanobind.h"

#include "kali/device/command_queue.h"
#include "kali/device/command_stream.h"

KALI_PY_EXPORT(device_command)
{
    using namespace kali;

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
