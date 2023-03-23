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
        "type"_a = DeviceType::automatic
    );
    device.def("create_program", nb::overload_cast<const ProgramDesc&>(&Device::create_program), "desc"_a);
    device.def(
        "create_program",
        nb::overload_cast<std::filesystem::path, std::string>(&Device::create_program),
        "path"_a,
        "entrypoint"_a
    );

    nb::class_<Swapchain> swapchain(m, "Swapchain");

    nb::enum_<ResourceType>(m, "ResourceType")
        .value("unknown", ResourceType::unknown)
        .value("buffer", ResourceType::buffer)
        .value("texture_1d", ResourceType::texture_1d)
        .value("texture_2d", ResourceType::texture_2d)
        .value("texture_3d", ResourceType::texture_3d)
        .value("texture_cube", ResourceType::texture_cube);

    nb::enum_<ResourceState>(m, "ResourceState")
        .value("undefined", ResourceState::undefined)
        .value("general", ResourceState::general)
        .value("pre_initialized", ResourceState::pre_initialized)
        .value("vertex_buffer", ResourceState::vertex_buffer)
        .value("index_buffer", ResourceState::index_buffer)
        .value("constant_buffer", ResourceState::constant_buffer)
        .value("stream_output", ResourceState::stream_output)
        .value("shader_resource", ResourceState::shader_resource)
        .value("unordered_access", ResourceState::unordered_access)
        .value("render_target", ResourceState::render_target)
        .value("depth_read", ResourceState::depth_read)
        .value("depth_write", ResourceState::depth_write)
        .value("present", ResourceState::present)
        .value("indirect_argument", ResourceState::indirect_argument)
        .value("copy_source", ResourceState::copy_source)
        .value("copy_destination", ResourceState::copy_destination)
        .value("resolve_source", ResourceState::resolve_source)
        .value("resolve_destination", ResourceState::resolve_destination)
        .value("acceleration_structure", ResourceState::acceleration_structure)
        .value("acceleration_structure_build_output", ResourceState::acceleration_structure_build_output)
        .value("pixel_shader_resource", ResourceState::pixel_shader_resource)
        .value("non_pixel_shader_resource", ResourceState::non_pixel_shader_resource);

    nb::enum_<ResourceUsage>(m, "ResourceUsage")
        .value("none", ResourceUsage::none)
        .value("vertex", ResourceUsage::vertex)
        .value("index", ResourceUsage::index)
        .value("constant", ResourceUsage::constant)
        .value("stream_output", ResourceUsage::stream_output)
        .value("shader_resource", ResourceUsage::shader_resource)
        .value("unordered_access", ResourceUsage::unordered_access)
        .value("render_target", ResourceUsage::render_target)
        .value("depth_stencil", ResourceUsage::depth_stencil)
        .value("indirect_arg", ResourceUsage::indirect_arg)
        .value("shared", ResourceUsage::shared)
        .value("acceleration_structure", ResourceUsage::acceleration_structure)
        .def("__and__", [](ResourceUsage value1, ResourceUsage value2) { return value1 & value2; })
        .def("__or__", [](ResourceUsage value1, ResourceUsage value2) { return value1 | value2; });

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
