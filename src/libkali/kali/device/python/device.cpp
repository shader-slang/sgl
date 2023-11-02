#include "nanobind.h"

#include "kali/device/device.h"
#include "kali/device/sampler.h"
#include "kali/device/fence.h"
#include "kali/device/resource.h"
#include "kali/device/swapchain.h"
#include "kali/device/shader.h"

KALI_PY_EXPORT(device_device)
{
    using namespace kali;

    nb::class_<AdapterInfo>(m, "AdapterInfo")
        .def_ro("name", &AdapterInfo::name)
        .def_ro("vendor_id", &AdapterInfo::vendor_id)
        .def_ro("device_id", &AdapterInfo::device_id)
        .def_ro("luid", &AdapterInfo::luid);

    nb::kali_enum<DeviceType>(m, "DeviceType");

    nb::class_<DeviceDesc>(m, "DeviceDesc")
        .def(nb::init<>())
        .def_rw("type", &DeviceDesc::type)
        .def_rw("enable_debug_layers", &DeviceDesc::enable_debug_layers)
        .def_rw("adapter_luid", &DeviceDesc::adapter_luid)
        .def_rw("default_shader_model", &DeviceDesc::default_shader_model)
        .def_rw("shader_cache_path", &DeviceDesc::shader_cache_path)
        .def("__repr__", &DeviceDesc::to_string);

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
        [](Device* self,
           DeviceType type,
           bool enable_debug_layers,
           std::optional<AdapterLUID> adapter_luid,
           ShaderModel default_shader_model,
           std::string shader_cache_path)
        {
            new (self) Device(DeviceDesc{
                .type = type,
                .enable_debug_layers = enable_debug_layers,
                .adapter_luid = adapter_luid ? &adapter_luid.value() : nullptr,
                .default_shader_model = default_shader_model,
                .shader_cache_path = std::move(shader_cache_path),
            });
        },
        "type"_a = DeviceType::automatic,
        "enable_debug_layers"_a = false,
        "adapter_luid"_a = std::optional<AdapterLUID>{},
        "default_shader_model"_a = ShaderModel::sm_6_6,
        "shader_cache_path"_a = std::string{}
    );
    device.def_prop_ro("desc", &Device::desc);
    device.def_prop_ro("info", &Device::info);
    device.def(
        "create_buffer",
        [](Device* self, const BufferDesc& desc) { return self->create_buffer(desc); },
        "desc"_a
    );
    device.def(
        "create_buffer",
        [](Device* self,
           size_t size,
           size_t struct_size,
           Format format,
           ResourceUsage usage,
           MemoryType memory_type,
           std::string debug_name)
        {
            return self->create_buffer(BufferDesc{
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
        [](Device* self, const TextureDesc& desc) { return self->create_texture(desc); },
        "desc"_a
    );
    device.def(
        "create_texture",
        [](Device* self,
           TextureType type,
           Format format,
           uint32_t width,
           uint32_t height,
           uint32_t depth,
           uint32_t array_size,
           uint32_t mip_count)
        {
            return self->create_texture(TextureDesc{
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
    device.def("create_sampler", &Device::create_sampler, "desc"_a);
    device.def(
        "create_sampler",
        [](Device* self,
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
            return self->create_sampler(SamplerDesc{
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
    device.def("create_fence", &Device::create_fence, "desc"_a);
    device.def(
        "create_fence",
        [](Device* self, uint64_t initial_value, bool shared)
        { return self->create_fence({.initial_value = initial_value, .shared = shared}); },
        "initial_value"_a = 0,
        "shared"_a = false
    );
    device.def("create_slang_session", [](Device* self) { return self->create_slang_session(SlangSessionDesc{}); });
    device.def("load_module", &Device::load_module, "path"_a);
    device.def("load_module_from_source", &Device::load_module_from_source, "source"_a);

    device.def_static("enumerate_adapters", &Device::enumerate_adapters, "type"_a = DeviceType::automatic);
    device.def_static("report_live_objects", &Device::report_live_objects);


    // device.def("create_program", nb::overload_cast<const ProgramDesc&>(&Device::create_program), "desc"_a);
    // device.def(
    //     "create_program",
    //     nb::overload_cast<std::filesystem::path, std::string>(&Device::create_program),
    //     "path"_a,
    //     "entrypoint"_a
    // );
}
