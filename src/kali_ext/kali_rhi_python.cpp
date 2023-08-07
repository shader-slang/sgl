#include "object_python.h"
#include "helpers.h"

#include "kali/rhi/types.h"
#include "kali/rhi/device.h"
#include "kali/rhi/swapchain.h"
#include "kali/rhi/resource.h"
#include "kali/rhi/sampler.h"
#include "kali/rhi/program.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/bind_map.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace kali {

void register_kali_rhi(nb::module_& m)
{
    // ------------------------------------------------------------------------
    // types.h
    // ------------------------------------------------------------------------

    nb::kali_enum<ComparisonFunc>(m, "ComparisonFunc");
    nb::kali_enum<TextureFilteringMode>(m, "TextureFilteringMode");
    nb::kali_enum<TextureAddressingMode>(m, "TextureAddressingMode");
    nb::kali_enum<TextureReductionOp>(m, "TextureReductionOp");

    // ------------------------------------------------------------------------
    // formats.h
    // ------------------------------------------------------------------------

    nb::kali_enum<Format>(m, "Format");

    // ------------------------------------------------------------------------
    // resource.h
    // ------------------------------------------------------------------------

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

    nb::kali_enum<ShaderStage>(m, "ShaderStage");
    nb::kali_enum<ShaderModel>(m, "ShaderModel");

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

    nb::kali_enum<DeviceType>(m, "DeviceType");

    nb::class_<DeviceLimits>(m, "DeviceLimits")
        .def_ro("max_texture_dimension_1d", &DeviceLimits::max_texture_dimension_1d)
        .def_ro("max_texture_dimension_2d", &DeviceLimits::max_texture_dimension_2d)
        .def_ro("max_texture_dimension_3d", &DeviceLimits::max_texture_dimension_3d)
        .def_ro("max_texture_dimension_cube", &DeviceLimits::max_texture_dimension_cube)
        .def_ro("max_texture_array_layers", &DeviceLimits::max_texture_array_layers)
        .def_ro("max_vertex_input_elements", &DeviceLimits::max_vertex_input_elements)
        .def_ro("max_vertex_input_element_offset", &DeviceLimits::max_vertex_input_element_offset)
        .def_ro("max_vertex_streams", &DeviceLimits::max_vertex_streams)
        .def_ro("max_vertex_stream_stride", &DeviceLimits::max_vertex_stream_stride)
        .def_ro("max_compute_threads_per_group", &DeviceLimits::max_compute_threads_per_group)
        .def_ro("max_compute_thread_group_size", &DeviceLimits::max_compute_thread_group_size)
        .def_ro("max_compute_dispatch_thread_groups", &DeviceLimits::max_compute_dispatch_thread_groups)
        .def_ro("max_viewports", &DeviceLimits::max_viewports)
        .def_ro("max_viewport_dimensions", &DeviceLimits::max_viewport_dimensions)
        .def_ro("max_framebuffer_dimensions", &DeviceLimits::max_framebuffer_dimensions)
        .def_ro("max_shader_visible_samplers", &DeviceLimits::max_shader_visible_samplers);

    nb::class_<DeviceInfo>(m, "DeviceInfo")
        .def_ro("type", &DeviceInfo::type)
        .def_ro("api_name", &DeviceInfo::api_name)
        .def_ro("adapter_name", &DeviceInfo::adapter_name)
        .def_ro("limits", &DeviceInfo::limits);

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
    device.def_prop_ro("info", &Device::get_info);
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
           MemoryType memory_type,
           std::string debug_name)
        {
            return device->create_buffer(BufferDesc{
                .size = size,
                .struct_size = struct_size,
                .format = format,
                .usage = usage,
                .memory_type = memory_type,
                .debug_name = std::move(debug_name),
            });
        },
        "size"_a,
        "struct_size"_a = 0,
        "format"_a = Format::unknown,
        "usage"_a = ResourceUsage::none,
        "memory_type"_a = MemoryType::device_local,
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
           Format format,
           uint32_t width,
           uint32_t height,
           uint32_t depth,
           uint32_t array_size,
           uint32_t mip_count)
        {
            return device->create_texture(TextureDesc{
                .type = type,
                .format = format,
                .width = width,
                .height = height,
                .depth = depth,
                .array_size = array_size,
                .mip_count = mip_count,
            });
        },
        "type"_a = TextureType::unknown,
        "format"_a = Format::unknown,
        "width"_a = 1,
        "height"_a = 1,
        "depth"_a = 1,
        "array_size"_a = 0,
        "mip_count"_a = 0
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

    // device.def("create_program", nb::overload_cast<const ProgramDesc&>(&Device::create_program), "desc"_a);
    // device.def(
    //     "create_program",
    //     nb::overload_cast<std::filesystem::path, std::string>(&Device::create_program),
    //     "path"_a,
    //     "entrypoint"_a
    // );
}

} // namespace kali
