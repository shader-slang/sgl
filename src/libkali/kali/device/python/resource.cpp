#include "nanobind.h"

#include "kali/device/device.h"
#include "kali/device/resource.h"

KALI_PY_EXPORT(device_resource)
{
    using namespace kali;

    nb::kali_enum<ResourceType>(m, "ResourceType");
    nb::kali_enum<ResourceState>(m, "ResourceState");

    nb::kali_enum<ResourceUsage>(m, "ResourceUsage")
        .def("__and__", [](ResourceUsage value1, ResourceUsage value2) { return value1 & value2; })
        .def("__or__", [](ResourceUsage value1, ResourceUsage value2) { return value1 | value2; });

    nb::kali_enum<MemoryType>(m, "MemoryType");

    nb::class_<Resource, Object>(m, "Resource").def("device_address", &Resource::get_device_address);

    nb::class_<BufferDesc>(m, "BufferDesc")
        .def_rw("size", &BufferDesc::size)
        .def_rw("struct_size", &BufferDesc::struct_size)
        .def_rw("format", &BufferDesc::format)
        .def_rw("initial_state", &BufferDesc::initial_state)
        .def_rw("usage", &BufferDesc::usage)
        .def_rw("memory_type", &BufferDesc::memory_type)
        .def_rw("debug_name", &BufferDesc::debug_name);

    nb::class_<Buffer, Resource>(m, "Buffer")
        .def(
            "__init__",
            [](Buffer* self,
               ref<Device> device,
               size_t size,
               size_t struct_size,
               Format format,
               ResourceState initial_state,
               ResourceUsage usage,
               MemoryType memory_type,
               std::string debug_name)
            {
                new (self) Buffer(
                    std::move(device),
                    BufferDesc{
                        .size = size,
                        .struct_size = struct_size,
                        .format = format,
                        .initial_state = initial_state,
                        .usage = usage,
                        .memory_type = memory_type,
                        .debug_name = std::move(debug_name),
                    },
                    nullptr
                );
            },
            "device"_a,
            "size"_a = 0,
            "struct_size"_a = 0,
            "format"_a = Format::unknown,
            "initial_state"_a = ResourceState::undefined,
            "usage"_a = ResourceUsage::none,
            "memory_type"_a = MemoryType::device_local,
            "debug_name"_a = ""
        )
        .def_prop_ro("desc", &Buffer::get_desc)
        .def_prop_ro("size", &Buffer::get_size)
        .def_prop_ro("struct_size", &Buffer::get_struct_size)
        .def_prop_ro("format", &Buffer::get_format);

    nb::kali_enum<TextureType>(m, "TextureType");

    nb::class_<TextureDesc>(m, "TextureDesc")
        .def_rw("type", &TextureDesc::type)
        .def_rw("width", &TextureDesc::width)
        .def_rw("height", &TextureDesc::height)
        .def_rw("depth", &TextureDesc::depth)
        .def_rw("array_size", &TextureDesc::array_size)
        .def_rw("mip_count", &TextureDesc::mip_count)
        .def_rw("format", &TextureDesc::format)
        .def_rw("initial_state", &TextureDesc::initial_state)
        .def_rw("usage", &TextureDesc::usage)
        .def_rw("memory_type", &TextureDesc::memory_type)
        .def_rw("debug_name", &TextureDesc::debug_name);

    nb::class_<Texture, Resource>(m, "Texture");
}
