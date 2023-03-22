#include "rhi/device.h"
#include "rhi/swapchain.h"
#include "rhi/resource.h"
#include "rhi/program.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace kali {

void register_rhi(nb::module_& m)
{
    nb::enum_<DeviceType>(m, "DeviceType")
        .value("automatic", DeviceType::automatic)
        .value("d3d12", DeviceType::d3d12)
        .value("vulkan", DeviceType::vulkan)
        .value("cpu", DeviceType::cpu)
        .value("cuda", DeviceType::cuda);

    nb::class_<Device> device(m, "Device");
    device.def(
        "__init__",
        [](Device* device, DeviceType type)
        {
            new (device) Device(DeviceDesc{
                .type = type,
            });
        },
        "type"_a = DeviceType::automatic);
    device.def("create_program", nb::overload_cast<const ProgramDesc&>(&Device::create_program), "desc"_a);
    device.def("create_program", nb::overload_cast<std::filesystem::path, std::string>(&Device::create_program),
        "path"_a, "entrypoint"_a);

    nb::class_<Swapchain> swapchain(m, "Swapchain");

    nb::enum_<ResourceType>(m, "ResourceType")
        .value("unknown", ResourceType::unknown)
        .value("buffer", ResourceType::buffer)
        .value("texture_1d", ResourceType::texture_1d)
        .value("texture_2d", ResourceType::texture_2d)
        .value("texture_3d", ResourceType::texture_3d)
        .value("texture_cube", ResourceType::texture_cube);

    nb::enum_<ResourceState>(m, "ResourceState");

    nb::enum_<MemoryType>(m, "MemoryType")
        .value("device_local", MemoryType::device_local)
        .value("upload", MemoryType::upload)
        .value("read_back", MemoryType::read_back);

    nb::class_<MemoryRange> memory_range(m, "MemoryRange");
    memory_range.def_rw("offset", &MemoryRange::offset);
    memory_range.def_rw("size", &MemoryRange::size);

    nb::class_<Resource> resource(m, "Resource");
    resource.def("device_address", &Resource::get_device_address);

    nb::class_<Program> program(m, "Program");
}

} // namespace kali
