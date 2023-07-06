#include "rhi/types.h"
#include "rhi/device.h"
#include "rhi/swapchain.h"
#include "rhi/resource.h"
#include "rhi/sampler.h"
#include "rhi/program.h"

#include "core/object_python.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/bind_map.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace kali {

void register_rhi(nb::module_& m)
{
    // ------------------------------------------------------------------------
    // types.h
    // ------------------------------------------------------------------------

    nb::enum_<ComparisonFunc>(m, "ComparisonFunc")
        .value("never", ComparisonFunc::never)
        .value("less", ComparisonFunc::less)
        .value("equal", ComparisonFunc::equal)
        .value("less_equal", ComparisonFunc::less_equal)
        .value("greater", ComparisonFunc::greater)
        .value("not_equal", ComparisonFunc::not_equal)
        .value("greater_equal", ComparisonFunc::greater_equal)
        .value("always", ComparisonFunc::always);

    nb::enum_<TextureFilteringMode>(m, "TextureFilteringMode")
        .value("point", TextureFilteringMode::point)
        .value("linear", TextureFilteringMode::linear);

    nb::enum_<TextureAddressingMode>(m, "TextureAddressingMode")
        .value("wrap", TextureAddressingMode::wrap)
        .value("clamp_to_edge", TextureAddressingMode::clamp_to_edge)
        .value("clamp_to_border", TextureAddressingMode::clamp_to_border)
        .value("mirror_repeat", TextureAddressingMode::mirror_repeat)
        .value("mirror_once", TextureAddressingMode::mirror_once);

    nb::enum_<TextureReductionOp>(m, "TextureReductionOp")
        .value("average", TextureReductionOp::average)
        .value("comparison", TextureReductionOp::comparison)
        .value("minimum", TextureReductionOp::minimum)
        .value("maximum", TextureReductionOp::maximum);

    // ------------------------------------------------------------------------
    // formats.h
    // ------------------------------------------------------------------------

    nb::enum_<Format> format(m, "Format");
    for (uint32_t i = 0; i < uint32_t(Format::count); ++i) {
        const FormatInfo& info = get_format_info(Format(i));
        format.value(info.name.c_str(), info.format);
    }

    // ------------------------------------------------------------------------
    // resource.h
    // ------------------------------------------------------------------------

    nb::enum_<ResourceType>(m, "ResourceType")
        .value("unknown", ResourceType::unknown)
        .value("buffer", ResourceType::buffer)
        .value("texture_1d", ResourceType::texture_1d)
        .value("texture_2d", ResourceType::texture_2d)
        .value("texture_3d", ResourceType::texture_3d)
        .value("texture_cube", ResourceType::texture_cube);

    nb::enum_<TextureType>(m, "TextureType")
        .value("unknown", TextureType::unknown)
        .value("texture_1d", TextureType::texture_1d)
        .value("texture_2d", TextureType::texture_2d)
        .value("texture_3d", TextureType::texture_3d)
        .value("texture_cube", TextureType::texture_cube);

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

    nb::enum_<CpuAccess>(m, "CpuAccess")
        .value("none", CpuAccess::none)
        .value("read", CpuAccess::read)
        .value("write", CpuAccess::write);

    nb::class_<MemoryRange>(m, "MemoryRange").def_rw("offset", &MemoryRange::offset).def_rw("size", &MemoryRange::size);

    nb::class_<Resource, Object> resource(m, "Resource");
    resource.def("device_address", &Resource::get_device_address);

    nb::class_<BufferDesc>(m, "BufferDesc")
        .def_rw("size", &BufferDesc::size)
        .def_rw("struct_size", &BufferDesc::struct_size)
        .def_rw("format", &BufferDesc::format)
        .def_rw("initial_state", &BufferDesc::initial_state)
        .def_rw("usage", &BufferDesc::usage)
        .def_rw("cpu_access", &BufferDesc::cpu_access)
        .def_rw("debug_name", &BufferDesc::debug_name);

    nb::class_<Buffer, Resource> buffer(m, "Buffer");
    buffer.def_prop_ro("desc", &Buffer::get_desc);
    buffer.def_prop_ro("size", &Buffer::get_size);
    buffer.def_prop_ro("struct_size", &Buffer::get_struct_size);
    buffer.def_prop_ro("format", &Buffer::get_format);

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
        .def_rw("cpu_access", &TextureDesc::cpu_access)
        .def_rw("debug_name", &TextureDesc::debug_name);

    // ------------------------------------------------------------------------
    // sampler.h
    // ------------------------------------------------------------------------

    nb::class_<Sampler, Object> sampler(m, "Sampler");
    sampler.def_prop_ro("desc", &Sampler::get_desc);

    // ------------------------------------------------------------------------
    // swapchain.h
    // ------------------------------------------------------------------------

    nb::class_<Swapchain, Object> swapchain(m, "Swapchain");

    // ------------------------------------------------------------------------
    // program.h
    // ------------------------------------------------------------------------

    nb::enum_<ShaderType>(m, "ShaderType")
        .value("none", ShaderType::none)
        .value("vertex", ShaderType::vertex)
        .value("hull", ShaderType::hull)
        .value("domain", ShaderType::domain)
        .value("geometry", ShaderType::geometry)
        .value("fragment", ShaderType::fragment)
        .value("compute", ShaderType::compute)
        .value("ray_generation", ShaderType::ray_generation)
        .value("intersection", ShaderType::intersection)
        .value("any_hit", ShaderType::any_hit)
        .value("closest_hit", ShaderType::closest_hit)
        .value("miss", ShaderType::miss)
        .value("callable", ShaderType::callable)
        .value("mesh", ShaderType::mesh)
        .value("amplification", ShaderType::amplification);

    nb::enum_<ShaderModel>(m, "ShaderModel")
        .value("sm_6_0", ShaderModel::sm_6_0)
        .value("sm_6_1", ShaderModel::sm_6_1)
        .value("sm_6_2", ShaderModel::sm_6_2)
        .value("sm_6_3", ShaderModel::sm_6_3)
        .value("sm_6_4", ShaderModel::sm_6_4)
        .value("sm_6_5", ShaderModel::sm_6_5)
        .value("sm_6_6", ShaderModel::sm_6_6);

    nb::class_<TypeConformance>(m, "TypeConformance")
        .def_rw("type_name", &TypeConformance::type_name)
        .def_rw("interface_name", &TypeConformance::interface_name);

    nb::bind_map<std::map<TypeConformance, uint32_t>>(m, "TypeConformanceListBase");
    nb::class_<TypeConformanceList, std::map<TypeConformance, uint32_t>>(m, "TypeConformanceList")
        .def(
            "add",
            nb::overload_cast<std::string, std::string, uint32_t>(&TypeConformanceList::add),
            "type_name"_a,
            "interface_name"_a,
            "id"_a = -1
        )
        .def(
            "remove",
            nb::overload_cast<std::string, std::string>(&TypeConformanceList::remove),
            "type_name"_a,
            "interface_name"_a
        )
        .def("add", nb::overload_cast<const TypeConformanceList&>(&TypeConformanceList::add), "other"_a)
        .def("remove", nb::overload_cast<const TypeConformanceList&>(&TypeConformanceList::remove), "other"_a);

    nb::bind_map<std::map<std::string, std::string>>(m, "DefineListBase");
    nb::class_<DefineList, std::map<std::string, std::string>>(m, "DefineList")
        .def("add", nb::overload_cast<const std::string&, std::string>(&DefineList::add), "name"_a, "value"_a = "")
        .def("remove", nb::overload_cast<const std::string&>(&DefineList::remove), "name"_a)
        .def("add", nb::overload_cast<const DefineList&>(&DefineList::add), "other"_a)
        .def("remove", nb::overload_cast<const DefineList&>(&DefineList::remove), "other"_a);

    nb::enum_<ShaderSourceType>(m, "ShaderSourceType")
        .value("file", ShaderSourceType::file)
        .value("string", ShaderSourceType::string);

    nb::class_<ShaderSourceDesc>(m, "ShaderSourceDesc")
        .def_rw("type", &ShaderSourceDesc::type)
        .def_rw("file", &ShaderSourceDesc::file)
        .def_rw("string", &ShaderSourceDesc::string);

    // nb::class_<ShaderModuleDesc>(m, "ShaderModuleDesc")
    //     .def_rw("type", &ShaderModuleDesc::type)
    //     .def_rw("file", &ShaderModuleDesc::file)
    //     .def_rw("string", &ShaderModuleDesc::string)
    //     .def_rw("translation_unit", &ShaderModuleDesc::translation_unit);

    nb::class_<Program, Object> program(m, "Program");

    // ------------------------------------------------------------------------
    // device.h
    // ------------------------------------------------------------------------

    nb::enum_<DeviceType>(m, "DeviceType")
        .value("automatic", DeviceType::automatic)
        .value("d3d12", DeviceType::d3d12)
        .value("vulkan", DeviceType::vulkan)
        .value("cpu", DeviceType::cpu)
        .value("cuda", DeviceType::cuda);

    nb::class_<Device, Object> device(m, "Device");
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
        [](Device* device, const BufferDesc& desc) { return device->create_buffer(desc); },
        "desc"_a
    );
    device.def(
        "create_buffer",
        [](Device* device,
           size_t size,
           size_t struct_size,
           Format format,
           ResourceUsage usage,
           CpuAccess cpu_access,
           std::string debug_name)
        {
            return device->create_buffer(BufferDesc{
                .size = size,
                .struct_size = struct_size,
                .format = format,
                .usage = usage,
                .cpu_access = cpu_access,
                .debug_name = std::move(debug_name),
            });
        },
        "size"_a,
        "struct_size"_a = 0,
        "format"_a = Format::unknown,
        "usage"_a = ResourceUsage::none,
        "cpu_access"_a = CpuAccess::none,
        "debug_name"_a = ""
    );
    device.def(
        "create_texture",
        [](Device* device, const TextureDesc& desc) { return device->create_texture(desc); },
        "desc"_a
    );
    device.def(
        "create_texture",
        [](Device* device,
           TextureType type,
           uint32_t width,
           uint32_t height,
           uint32_t depth,
           uint32_t array_size,
           uint32_t mip_count,
           Format format)
        {
            return device->create_texture(TextureDesc{
                .type = type,
                .width = width,
                .height = height,
                .depth = depth,
                .array_size = array_size,
                .mip_count = mip_count,
                .format = format,
            });
        },
        "type"_a = TextureType::unknown,
        "width"_a = 0,
        "height"_a = 0,
        "depth"_a = 0,
        "array_size"_a = 0,
        "mip_count"_a = 0,
        "format"_a = Format::unknown
    );
    device.def(
        "create_sampler",
        [](Device* device,
           TextureFilteringMode min_filter,
           TextureFilteringMode mag_filter,
           TextureFilteringMode mip_filter,
           TextureReductionOp reduction_op,
           TextureAddressingMode address_u,
           TextureAddressingMode address_v,
           TextureAddressingMode address_w,
           float mip_lod_bias,
           uint32_t max_anisotropy,
           ComparisonFunc comparison_func,
           float4 border_color,
           float min_lod,
           float max_lod)
        {
            return device->create_sampler(SamplerDesc{
                .min_filter = min_filter,
                .mag_filter = mag_filter,
                .mip_filter = mip_filter,
                .reduction_op = reduction_op,
                .address_u = address_u,
                .address_v = address_v,
                .address_w = address_w,
                .mip_lod_bias = mip_lod_bias,
                .max_anisotropy = max_anisotropy,
                .comparison_func = comparison_func,
                .border_color = border_color,
                .min_lod = min_lod,
                .max_lod = max_lod,
            });
        },
        "min_filter"_a = TextureFilteringMode::linear,
        "mag_filter"_a = TextureFilteringMode::linear,
        "mip_filter"_a = TextureFilteringMode::linear,
        "reduction_op"_a = TextureReductionOp::average,
        "address_u"_a = TextureAddressingMode::wrap,
        "address_v"_a = TextureAddressingMode::wrap,
        "address_w"_a = TextureAddressingMode::wrap,
        "mip_lod_bias"_a = 0.f,
        "max_anisotropy"_a = 1,
        "comparison_func"_a = ComparisonFunc::never,
        "border_color"_a = float4{1.f, 1.f, 1.f, 1.f},
        "min_lod"_a = -1000.f,
        "max_lod"_a = 1000.f
    );

    device.def("create_program", nb::overload_cast<const ProgramDesc&>(&Device::create_program), "desc"_a);
    // device.def(
    //     "create_program",
    //     nb::overload_cast<std::filesystem::path, std::string>(&Device::create_program),
    //     "path"_a,
    //     "entrypoint"_a
    // );
}

} // namespace kali
