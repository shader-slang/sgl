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
    nb::enum_<Format>(m, "Format")
        .value("unknown", Format::unknown)
        .value("r32g32b32a32_typeless", Format::r32g32b32a32_typeless)
        .value("r32g32b32_typeless", Format::r32g32b32_typeless)
        .value("r32g32_typeless", Format::r32g32_typeless)
        .value("r32_typeless", Format::r32_typeless)
        .value("r16g16b16a16_typeless", Format::r16g16b16a16_typeless)
        .value("r16g16_typeless", Format::r16g16_typeless)
        .value("r16_typeless", Format::r16_typeless)
        .value("r8g8b8a8_typeless", Format::r8g8b8a8_typeless)
        .value("r8g8_typeless", Format::r8g8_typeless)
        .value("r8_typeless", Format::r8_typeless)
        .value("b8g8r8a8_typeless", Format::b8g8r8a8_typeless)
        .value("r32g32b32a32_float", Format::r32g32b32a32_float)
        .value("r32g32b32_float", Format::r32g32b32_float)
        .value("r32g32_float", Format::r32g32_float)
        .value("r32_float", Format::r32_float)
        .value("r16g16b16a16_float", Format::r16g16b16a16_float)
        .value("r16g16_float", Format::r16g16_float)
        .value("r16_float", Format::r16_float)
        .value("r32g32b32a32_uint", Format::r32g32b32a32_uint)
        .value("r32g32b32_uint", Format::r32g32b32_uint)
        .value("r32g32_uint", Format::r32g32_uint)
        .value("r32_uint", Format::r32_uint)
        .value("r16g16b16a16_uint", Format::r16g16b16a16_uint)
        .value("r16g16_uint", Format::r16g16_uint)
        .value("r16_uint", Format::r16_uint)
        .value("r8g8b8a8_uint", Format::r8g8b8a8_uint)
        .value("r8g8_uint", Format::r8g8_uint)
        .value("r8_uint", Format::r8_uint)
        .value("r32g32b32a32_sint", Format::r32g32b32a32_sint)
        .value("r32g32b32_sint", Format::r32g32b32_sint)
        .value("r32g32_sint", Format::r32g32_sint)
        .value("r32_sint", Format::r32_sint)
        .value("r16g16b16a16_sint", Format::r16g16b16a16_sint)
        .value("r16g16_sint", Format::r16g16_sint)
        .value("r16_sint", Format::r16_sint)
        .value("r8g8b8a8_sint", Format::r8g8b8a8_sint)
        .value("r8g8_sint", Format::r8g8_sint)
        .value("r8_sint", Format::r8_sint)
        .value("r16g16b16a16_unorm", Format::r16g16b16a16_unorm)
        .value("r16g16_unorm", Format::r16g16_unorm)
        .value("r16_unorm", Format::r16_unorm)
        .value("r8g8b8a8_unorm", Format::r8g8b8a8_unorm)
        .value("r8g8b8a8_unorm_srgb", Format::r8g8b8a8_unorm_srgb)
        .value("r8g8_unorm", Format::r8g8_unorm)
        .value("r8_unorm", Format::r8_unorm)
        .value("b8g8r8a8_unorm", Format::b8g8r8a8_unorm)
        .value("b8g8r8a8_unorm_srgb", Format::b8g8r8a8_unorm_srgb)
        .value("b8g8r8x8_unorm", Format::b8g8r8x8_unorm)
        .value("b8g8r8x8_unorm_srgb", Format::b8g8r8x8_unorm_srgb)
        .value("r16g16b16a16_snorm", Format::r16g16b16a16_snorm)
        .value("r16g16_snorm", Format::r16g16_snorm)
        .value("r16_snorm", Format::r16_snorm)
        .value("r8g8b8a8_snorm", Format::r8g8b8a8_snorm)
        .value("r8g8_snorm", Format::r8g8_snorm)
        .value("r8_snorm", Format::r8_snorm)
        .value("d32_float", Format::d32_float)
        .value("d16_unorm", Format::d16_unorm)
        .value("b4g4r4a4_unorm", Format::b4g4r4a4_unorm)
        .value("b5g6r5_unorm", Format::b5g6r5_unorm)
        .value("b5g5r5a1_unorm", Format::b5g5r5a1_unorm)
        .value("r9g9b9e5_sharedexp", Format::r9g9b9e5_sharedexp)
        .value("r10g10b10a2_typeless", Format::r10g10b10a2_typeless)
        .value("r10g10b10a2_unorm", Format::r10g10b10a2_unorm)
        .value("r10g10b10a2_uint", Format::r10g10b10a2_uint)
        .value("r11g11b10_float", Format::r11g11b10_float)
        .value("bc1_unorm", Format::bc1_unorm)
        .value("bc1_unorm_srgb", Format::bc1_unorm_srgb)
        .value("bc2_unorm", Format::bc2_unorm)
        .value("bc2_unorm_srgb", Format::bc2_unorm_srgb)
        .value("bc3_unorm", Format::bc3_unorm)
        .value("bc3_unorm_srgb", Format::bc3_unorm_srgb)
        .value("bc4_unorm", Format::bc4_unorm)
        .value("bc4_snorm", Format::bc4_snorm)
        .value("bc5_unorm", Format::bc5_unorm)
        .value("bc5_snorm", Format::bc5_snorm)
        .value("bc6h_uf16", Format::bc6h_uf16)
        .value("bc6h_sf16", Format::bc6h_sf16)
        .value("bc7_unorm", Format::bc7_unorm)
        .value("bc7_unorm_srgb", Format::bc7_unorm_srgb);

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
    device.def(
        "create_buffer",
        [](Device* device, size_t size, size_t struct_size, Format format, std::string debug_name)
        {
            return device->create_buffer(BufferDesc{
                .size = size,
                .struct_size = struct_size,
                .format = format,
                .debug_name = std::move(debug_name),
                .usage = ResourceUsage::vertex,
                .cpu_access = CpuAccess::none,
            });
        },
        "size"_a,
        "struct_size"_a = 0,
        "format"_a = Format::unknown,
        "debug_name"_a = ""
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

    nb::class_<Buffer> buffer(m, "Buffer");
    buffer.def_prop_ro("size", &Buffer::get_size);
    buffer.def_prop_ro("struct_size", &Buffer::get_struct_size);
    buffer.def_prop_ro("format", &Buffer::get_format);

    nb::class_<Program> program(m, "Program");
}

} // namespace kali
