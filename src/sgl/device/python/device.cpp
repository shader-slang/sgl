// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/device.h"
#include "sgl/device/reflection.h"
#include "sgl/device/sampler.h"
#include "sgl/device/fence.h"
#include "sgl/device/resource.h"
#include "sgl/device/pipeline.h"
#include "sgl/device/kernel.h"
#include "sgl/device/raytracing.h"
#include "sgl/device/query.h"
#include "sgl/device/input_layout.h"
#include "sgl/device/framebuffer.h"
#include "sgl/device/memory_heap.h"
#include "sgl/device/swapchain.h"
#include "sgl/device/shader.h"
#include "sgl/device/command.h"

#include "sgl/core/window.h"

namespace sgl {
SGL_DICT_TO_DESC_BEGIN(DeviceDesc)
SGL_DICT_TO_DESC_FIELD(type, DeviceType)
SGL_DICT_TO_DESC_FIELD(enable_debug_layers, bool)
SGL_DICT_TO_DESC_FIELD(enable_cuda_interop, bool)
SGL_DICT_TO_DESC_FIELD(enable_print, bool)
SGL_DICT_TO_DESC_FIELD(enable_hot_reload, bool)
SGL_DICT_TO_DESC_FIELD(adapter_luid, AdapterLUID)
SGL_DICT_TO_DESC_FIELD_DICT(compiler_options, SlangCompilerOptions)
SGL_DICT_TO_DESC_FIELD(shader_cache_path, std::filesystem::path)
SGL_DICT_TO_DESC_END()
} // namespace sgl

SGL_PY_EXPORT(device_device)
{
    using namespace sgl;

    nb::class_<AdapterInfo>(m, "AdapterInfo", D(AdapterInfo))
        .def_ro("name", &AdapterInfo::name, D(AdapterInfo, name))
        .def_ro("vendor_id", &AdapterInfo::vendor_id, D(AdapterInfo, vendor_id))
        .def_ro("device_id", &AdapterInfo::device_id, D(AdapterInfo, device_id))
        .def_ro("luid", &AdapterInfo::luid, D(AdapterInfo, luid))
        .def("__repr__", &AdapterInfo::to_string);

    nb::sgl_enum<DeviceType>(m, "DeviceType");

    nb::class_<DeviceDesc>(m, "DeviceDesc", D(DeviceDesc))
        .def(nb::init<>())
        .def("__init__", [](DeviceDesc* self, nb::dict dict) { new (self) DeviceDesc(dict_to_DeviceDesc(dict)); })
        .def_rw("type", &DeviceDesc::type, D(DeviceDesc, type))
        .def_rw("enable_debug_layers", &DeviceDesc::enable_debug_layers, D(DeviceDesc, enable_debug_layers))
        .def_rw("enable_cuda_interop", &DeviceDesc::enable_cuda_interop, D(DeviceDesc, enable_cuda_interop))
        .def_rw("enable_print", &DeviceDesc::enable_print, D(DeviceDesc, enable_print))
        .def_rw("enable_hot_reload", &DeviceDesc::enable_hot_reload, D(DeviceDesc, adapter_luid))
        .def_rw("adapter_luid", &DeviceDesc::adapter_luid, D(DeviceDesc, adapter_luid))
        .def_rw("compiler_options", &DeviceDesc::compiler_options, D(DeviceDesc, compiler_options))
        .def_rw("shader_cache_path", &DeviceDesc::shader_cache_path, D(DeviceDesc, shader_cache_path));
    nb::implicitly_convertible<nb::dict, DeviceDesc>();

    nb::class_<DeviceLimits>(m, "DeviceLimits", D(DeviceLimits))
        .def_ro(
            "max_texture_dimension_1d",
            &DeviceLimits::max_texture_dimension_1d,
            D(DeviceLimits, max_texture_dimension_1d)
        )
        .def_ro(
            "max_texture_dimension_2d",
            &DeviceLimits::max_texture_dimension_2d,
            D(DeviceLimits, max_texture_dimension_2d)
        )
        .def_ro(
            "max_texture_dimension_3d",
            &DeviceLimits::max_texture_dimension_3d,
            D(DeviceLimits, max_texture_dimension_3d)
        )
        .def_ro(
            "max_texture_dimension_cube",
            &DeviceLimits::max_texture_dimension_cube,
            D(DeviceLimits, max_texture_dimension_cube)
        )
        .def_ro(
            "max_texture_array_layers",
            &DeviceLimits::max_texture_array_layers,
            D(DeviceLimits, max_texture_array_layers)
        )
        .def_ro(
            "max_vertex_input_elements",
            &DeviceLimits::max_vertex_input_elements,
            D(DeviceLimits, max_vertex_input_elements)
        )
        .def_ro(
            "max_vertex_input_element_offset",
            &DeviceLimits::max_vertex_input_element_offset,
            D(DeviceLimits, max_vertex_input_element_offset)
        )
        .def_ro("max_vertex_streams", &DeviceLimits::max_vertex_streams, D(DeviceLimits, max_vertex_streams))
        .def_ro(
            "max_vertex_stream_stride",
            &DeviceLimits::max_vertex_stream_stride,
            D(DeviceLimits, max_vertex_stream_stride)
        )
        .def_ro(
            "max_compute_threads_per_group",
            &DeviceLimits::max_compute_threads_per_group,
            D(DeviceLimits, max_compute_threads_per_group)
        )
        .def_ro(
            "max_compute_thread_group_size",
            &DeviceLimits::max_compute_thread_group_size,
            D(DeviceLimits, max_compute_thread_group_size)
        )
        .def_ro(
            "max_compute_dispatch_thread_groups",
            &DeviceLimits::max_compute_dispatch_thread_groups,
            D(DeviceLimits, max_compute_dispatch_thread_groups)
        )
        .def_ro("max_viewports", &DeviceLimits::max_viewports, D(DeviceLimits, max_viewports))
        .def_ro(
            "max_viewport_dimensions",
            &DeviceLimits::max_viewport_dimensions,
            D(DeviceLimits, max_viewport_dimensions)
        )
        .def_ro(
            "max_framebuffer_dimensions",
            &DeviceLimits::max_framebuffer_dimensions,
            D(DeviceLimits, max_framebuffer_dimensions)
        )
        .def_ro(
            "max_shader_visible_samplers",
            &DeviceLimits::max_shader_visible_samplers,
            D(DeviceLimits, max_shader_visible_samplers)
        );

    nb::class_<DeviceInfo>(m, "DeviceInfo", D(DeviceInfo))
        .def_ro("type", &DeviceInfo::type, D(DeviceInfo, type))
        .def_ro("api_name", &DeviceInfo::api_name, D(DeviceInfo, api_name))
        .def_ro("adapter_name", &DeviceInfo::adapter_name, D(DeviceInfo, adapter_name))
        .def_ro("timestamp_frequency", &DeviceInfo::timestamp_frequency, D(DeviceInfo, timestamp_frequency))
        .def_ro("limits", &DeviceInfo::limits, D(DeviceInfo, limits));

    nb::class_<ShaderCacheStats>(m, "ShaderCacheStats", D(ShaderCacheStats))
        .def_ro("entry_count", &ShaderCacheStats::entry_count, D(ShaderCacheStats, entry_count))
        .def_ro("hit_count", &ShaderCacheStats::hit_count, D(ShaderCacheStats, hit_count))
        .def_ro("miss_count", &ShaderCacheStats::miss_count, D(ShaderCacheStats, miss_count));

    nb::class_<Device, Object> device(m, "Device", D(Device));
    device.def(
        "__init__",
        [](Device* self,
           DeviceType type,
           bool enable_debug_layers,
           bool enable_cuda_interop,
           bool enable_print,
           bool enable_hot_reload,
           std::optional<AdapterLUID> adapter_luid,
           std::optional<SlangCompilerOptions> compiler_options,
           std::optional<std::filesystem::path> shader_cache_path)
        {
            new (self) Device({
                .type = type,
                .enable_debug_layers = enable_debug_layers,
                .enable_cuda_interop = enable_cuda_interop,
                .enable_print = enable_print,
                .enable_hot_reload = enable_hot_reload,
                .adapter_luid = adapter_luid,
                .compiler_options = compiler_options.value_or(SlangCompilerOptions{}),
                .shader_cache_path = shader_cache_path,
            });
        },
        "type"_a = DeviceDesc().type,
        "enable_debug_layers"_a = DeviceDesc().enable_debug_layers,
        "enable_cuda_interop"_a = DeviceDesc().enable_cuda_interop,
        "enable_print"_a = DeviceDesc().enable_print,
        "enable_hot_reload"_a = true,
        "adapter_luid"_a.none() = nb::none(),
        "compiler_options"_a.none() = nb::none(),
        "shader_cache_path"_a.none() = nb::none(),
        D(Device, Device)
    );
    device.def(nb::init<DeviceDesc>(), "desc"_a, D(Device, Device));
    device.def_prop_ro("desc", &Device::desc, D(Device, desc));
    device.def_prop_ro("info", &Device::info, D(Device, info));
    device.def_prop_ro("shader_cache_stats", &Device::shader_cache_stats, D(Device, shader_cache_stats));
    device.def_prop_ro("supported_shader_model", &Device::supported_shader_model, D(Device, supported_shader_model));
    device.def_prop_ro("features", &Device::features, D(Device, features));
    device.def_prop_ro("supports_cuda_interop", &Device::supports_cuda_interop, D(Device, supports_cuda_interop));
    device.def(
        "get_format_supported_resource_states",
        &Device::get_format_supported_resource_states,
        "format"_a,
        D_NA(Device, get_format_supported_states)
    );

    device.def_prop_ro("slang_session", &Device::slang_session, D(Device, slang_session));
    device.def(
        "create_swapchain",
        [](Device* self,
           ref<Window> window,
           Format format,
           uint32_t width,
           uint32_t height,
           uint32_t image_count,
           bool enable_vsync)
        {
            return self->create_swapchain(
                {
                    .format = format,
                    .width = width,
                    .height = height,
                    .image_count = image_count,
                    .enable_vsync = enable_vsync,
                },
                window
            );
        },
        "window"_a,
        "format"_a = SwapchainDesc().format,
        "width"_a = SwapchainDesc().width,
        "height"_a = SwapchainDesc().height,
        "image_count"_a = SwapchainDesc().image_count,
        "enable_vsync"_a = SwapchainDesc().enable_vsync,
        D(Device, create_swapchain)
    );
    device.def(
        "create_swapchain",
        [](Device* self,
           WindowHandle window_handle,
           Format format,
           uint32_t width,
           uint32_t height,
           uint32_t image_count,
           bool enable_vsync)
        {
            return self->create_swapchain(
                {
                    .format = format,
                    .width = width,
                    .height = height,
                    .image_count = image_count,
                    .enable_vsync = enable_vsync,
                },
                window_handle
            );
        },
        "window_handle"_a,
        "format"_a = SwapchainDesc().format,
        "width"_a = SwapchainDesc().width,
        "height"_a = SwapchainDesc().height,
        "image_count"_a = SwapchainDesc().image_count,
        "enable_vsync"_a = SwapchainDesc().enable_vsync,
        D_NA(Device, create_swapchain, 2)
    );
    device.def(
        "create_swapchain",
        [](Device* self, const SwapchainDesc& desc, ref<Window> window)
        { return self->create_swapchain(desc, window); },
        "desc"_a,
        "window"_a,
        D(Device, create_swapchain)
    );
    device.def(
        "create_buffer",
        [](Device* self,
           size_t size,
           size_t element_count,
           size_t struct_size,
           nb::object struct_type,
           Format format,
           ResourceUsage usage,
           MemoryType memory_type,
           std::string debug_name,
           std::optional<nb::ndarray<nb::numpy>> data)
        {
            if (data) {
                SGL_CHECK(is_ndarray_contiguous(*data), "Data is not contiguous.");
            }

            // Note: nanobind can't try cast to a ref counted pointer, however the reflection
            // cursor code below needs to ensure it maintains a reference to the type layout if
            // returned, so we attempt to convert to the raw ptr here, and then immediately
            // store it in a local ref counted ptr.
            const TypeLayoutReflection* _rawptr_resolved_struct_type = nullptr;
            nb::try_cast(struct_type, _rawptr_resolved_struct_type);
            ref<const TypeLayoutReflection> resolved_struct_type(_rawptr_resolved_struct_type);

            // If this is a reflection cursor, get type layout from it
            ReflectionCursor reflection_cursor;
            if (nb::try_cast(struct_type, reflection_cursor)) {
                resolved_struct_type = reflection_cursor.type_layout();
            }

            return self->create_buffer({
                .size = size,
                .element_count = element_count,
                .struct_size = struct_size,
                .struct_type = resolved_struct_type,
                .format = format,
                .usage = usage,
                .memory_type = memory_type,
                .debug_name = std::move(debug_name),
                .data = data ? data->data() : nullptr,
                .data_size = data ? data->nbytes() : 0,
            });
        },
        "size"_a = BufferDesc().size,
        "element_count"_a = BufferDesc().element_count,
        "struct_size"_a = BufferDesc().struct_size,
        "struct_type"_a.none() = nb::none(),
        "format"_a = BufferDesc().format,
        "usage"_a = BufferDesc().usage,
        "memory_type"_a = BufferDesc().memory_type,
        "debug_name"_a = BufferDesc().debug_name,
        "data"_a.none() = nb::none(),
        D(Device, create_buffer)
    );
    device.def(
        "create_buffer",
        [](Device* self, const BufferDesc& desc) { return self->create_buffer(desc); },
        "desc"_a,
        D(Device, create_buffer)
    );

    device.def(
        "create_texture",
        [](Device* self,
           ResourceType type,
           Format format,
           uint32_t width,
           uint32_t height,
           uint32_t depth,
           uint32_t array_size,
           uint32_t mip_count,
           uint32_t sample_count,
           uint32_t quality,
           ResourceUsage usage,
           MemoryType memory_type,
           std::string debug_name,
           std::optional<nb::ndarray<nb::numpy>> data)
        {
            if (data) {
                SGL_CHECK(is_ndarray_contiguous(*data), "Data is not contiguous.");
            }
            return self->create_texture({
                .type = type,
                .format = format,
                .width = width,
                .height = height,
                .depth = depth,
                .array_size = array_size,
                .mip_count = mip_count,
                .sample_count = sample_count,
                .quality = quality,
                .usage = usage,
                .memory_type = memory_type,
                .debug_name = std::move(debug_name),
                .data = data ? data->data() : nullptr,
                .data_size = data ? data->nbytes() : 0,
            });
        },
        "type"_a = TextureDesc().type,
        "format"_a = TextureDesc().format,
        "width"_a = TextureDesc().width,
        "height"_a = TextureDesc().height,
        "depth"_a = TextureDesc().depth,
        "array_size"_a = TextureDesc().array_size,
        "mip_count"_a = TextureDesc().mip_count,
        "sample_count"_a = TextureDesc().sample_count,
        "quality"_a = TextureDesc().quality,
        "usage"_a = TextureDesc().usage,
        "memory_type"_a = TextureDesc().memory_type,
        "debug_name"_a = TextureDesc().debug_name,
        "data"_a.none() = nb::none(),
        D(Device, create_texture)
    );
    device.def(
        "create_texture",
        [](Device* self, const TextureDesc& desc) { return self->create_texture(desc); },
        "desc"_a,
        D(Device, create_texture)
    );

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
            return self->create_sampler({
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
        "min_filter"_a = SamplerDesc().min_filter,
        "mag_filter"_a = SamplerDesc().mag_filter,
        "mip_filter"_a = SamplerDesc().mip_filter,
        "reduction_op"_a = SamplerDesc().reduction_op,
        "address_u"_a = SamplerDesc().address_u,
        "address_v"_a = SamplerDesc().address_v,
        "address_w"_a = SamplerDesc().address_w,
        "mip_lod_bias"_a = SamplerDesc().mip_lod_bias,
        "max_anisotropy"_a = SamplerDesc().max_anisotropy,
        "comparison_func"_a = SamplerDesc().comparison_func,
        "border_color"_a = SamplerDesc().border_color,
        "min_lod"_a = SamplerDesc().min_lod,
        "max_lod"_a = SamplerDesc().max_lod,
        D(Device, create_sampler)
    );
    device.def("create_sampler", &Device::create_sampler, "desc"_a, D(Device, create_sampler));

    device.def(
        "create_fence",
        [](Device* self, uint64_t initial_value, bool shared)
        { return self->create_fence({.initial_value = initial_value, .shared = shared}); },
        "initial_value"_a = FenceDesc().initial_value,
        "shared"_a = FenceDesc().shared,
        D(Device, create_fence)
    );
    device.def("create_fence", &Device::create_fence, "desc"_a, D(Device, create_fence));

    device.def(
        "create_query_pool",
        [](Device* self, QueryType type, uint32_t count)
        { return self->create_query_pool({.type = type, .count = count}); },
        "type"_a,
        "count"_a,
        D(Device, create_query_pool)
    );

    device.def(
        "create_input_layout",
        [](Device* self, std::vector<InputElementDesc> input_elements, std::vector<VertexStreamDesc> vertex_streams)
        {
            return self->create_input_layout({
                .input_elements = std::move(input_elements),
                .vertex_streams = std::move(vertex_streams),
            });
        },
        "input_elements"_a,
        "vertex_streams"_a,
        D(Device, create_input_layout)
    );
    device.def("create_input_layout", &Device::create_input_layout, "desc"_a, D(Device, create_input_layout));

    device.def(
        "create_framebuffer",
        [](Device* self,
           std::vector<ref<ResourceView>> render_targets,
           ref<ResourceView> depth_stencil,
           ref<FramebufferLayout> layout)
        {
            return self->create_framebuffer({
                .render_targets = std::move(render_targets),
                .depth_stencil = std::move(depth_stencil),
                .layout = std::move(layout),
            });
        },
        "render_targets"_a,
        "depth_stencil"_a.none() = nb::none(),
        "layout"_a.none() = nb::none(),
        D(Device, create_framebuffer)
    );
    device.def("create_framebuffer", &Device::create_framebuffer, "desc"_a, D(Device, create_framebuffer));

    device.def("create_command_buffer", &Device::create_command_buffer, D(Device, create_command_buffer));
    device.def(
        "submit_command_buffer",
        &Device::submit_command_buffer,
        "command_buffer"_a,
        "queue"_a = CommandQueueType::graphics,
        D(Device, submit_command_buffer)
    );
    device.def(
        "is_command_buffer_complete",
        &Device::is_command_buffer_complete,
        "id"_a,
        D(Device, is_command_buffer_complete)
    );
    device.def("wait_command_buffer", &Device::wait_command_buffer, "id"_a, D(Device, wait_command_buffer));
    device
        .def("wait_for_idle", &Device::wait_for_idle, "queue"_a = CommandQueueType::graphics, D(Device, wait_for_idle));
    device.def(
        "sync_to_cuda",
        [](Device* self, uint64_t cuda_stream) { self->sync_to_cuda(reinterpret_cast<void*>(cuda_stream)); },
        "cuda_stream"_a = 0,
        D(Device, sync_to_cuda)
    );
    device.def(
        "sync_to_device",
        [](Device* self, uint64_t cuda_stream) { self->sync_to_device(reinterpret_cast<void*>(cuda_stream)); },
        "cuda_stream"_a = 0,
        D(Device, sync_to_device)
    );
    device.def(
        "get_acceleration_structure_prebuild_info",
        &Device::get_acceleration_structure_prebuild_info,
        "build_inputs"_a,
        D(Device, get_acceleration_structure_prebuild_info)
    );
    device.def(
        "create_acceleration_structure",
        [](Device* self, AccelerationStructureKind kind, ref<Buffer> buffer, DeviceOffset offset, DeviceSize size)
        {
            return self->create_acceleration_structure({
                .kind = kind,
                .buffer = std::move(buffer),
                .offset = offset,
                .size = size,
            });
        },
        "kind"_a,
        "buffer"_a,
        "offset"_a = 0,
        "size"_a = 0,
        D(Device, create_acceleration_structure)
    );
    device.def(
        "create_shader_table",
        [](Device* self,
           ref<ShaderProgram> program,
           std::vector<std::string> ray_gen_entry_points,
           std::vector<std::string> miss_entry_points,
           std::vector<std::string> hit_group_names,
           std::vector<std::string> callable_entry_points)
        {
            return self->create_shader_table({
                .program = program,
                .ray_gen_entry_points = std::move(ray_gen_entry_points),
                .miss_entry_points = std::move(miss_entry_points),
                .hit_group_names = std::move(hit_group_names),
                .callable_entry_points = std::move(callable_entry_points),
            });
        },
        "program"_a,
        "ray_gen_entry_points"_a = std::vector<std::string>{},
        "miss_entry_points"_a = std::vector<std::string>{},
        "hit_group_names"_a = std::vector<std::string>{},
        "callable_entry_points"_a = std::vector<std::string>{},
        D(Device, create_shader_table)
    );
    device.def("create_shader_table", &Device::create_shader_table, "desc"_a, D(Device, create_shader_table));
    device.def(
        "create_slang_session",
        [](Device* self,
           std::optional<SlangCompilerOptions> compiler_options,
           bool add_default_include_paths,
           std::optional<std::filesystem::path> cache_path)
        {
            return self->create_slang_session(SlangSessionDesc{
                .compiler_options = compiler_options.value_or(SlangCompilerOptions{}),
                .add_default_include_paths = add_default_include_paths,
                .cache_path = cache_path,
            });
        },
        "compiler_options"_a.none() = nb::none(),
        "add_default_include_paths"_a = SlangSessionDesc().add_default_include_paths,
        "cache_path"_a.none() = nb::none(),
        D(Device, create_slang_session)
    );
    device.def("reload_all_programs", &Device::reload_all_programs, D_NA(Device, reload_all_programs));
    device.def("load_module", &Device::load_module, "module_name"_a, D(Device, load_module));
    device.def(
        "load_module_from_source",
        &Device::load_module_from_source,
        "module_name"_a,
        "source"_a,
        "path"_a.none() = nb::none(),
        D(Device, load_module_from_source)
    );
    device.def(
        "link_program",
        &Device::link_program,
        "modules"_a,
        "entry_points"_a,
        "link_options"_a.none() = nb::none(),
        D(Device, link_program)
    );
    device.def(
        "load_program",
        &Device::load_program,
        "module_name"_a,
        "entry_point_names"_a,
        "additional_source"_a.none() = nb::none(),
        "link_options"_a.none() = nb::none(),
        D(Device, load_program)
    );

    device.def(
        "create_mutable_shader_object",
        nb::overload_cast<const ShaderProgram*>(&Device::create_mutable_shader_object),
        "shader_program"_a,
        D(Device, create_mutable_shader_object)
    );
    device.def(
        "create_mutable_shader_object",
        [](Device* self, ref<TypeLayoutReflection> type_layout)
        { return self->create_mutable_shader_object(type_layout); },
        "type_layout"_a,
        D(Device, create_mutable_shader_object, 2)
    );
    device.def(
        "create_mutable_shader_object",
        nb::overload_cast<ReflectionCursor>(&Device::create_mutable_shader_object),
        "cursor"_a,
        D(Device, create_mutable_shader_object, 3)
    );

    device.def(
        "create_compute_pipeline",
        [](Device* self, ref<ShaderProgram> program)
        { return self->create_compute_pipeline({.program = std::move(program)}); },
        "program"_a,
        D(Device, create_compute_pipeline)
    );
    device
        .def("create_compute_pipeline", &Device::create_compute_pipeline, "desc"_a, D(Device, create_compute_pipeline));

    device.def(
        "create_graphics_pipeline",
        [](Device* self,
           ref<ShaderProgram> program,
           ref<InputLayout> input_layout,
           ref<FramebufferLayout> framebuffer_layout,
           PrimitiveType primitive_type,
           std::optional<DepthStencilDesc> depth_stencil,
           std::optional<RasterizerDesc> rasterizer,
           std::optional<BlendDesc> blend)
        {
            return self->create_graphics_pipeline({
                .program = std::move(program),
                .input_layout = input_layout,
                .framebuffer_layout = framebuffer_layout,
                .primitive_type = primitive_type,
                .depth_stencil = depth_stencil.value_or(DepthStencilDesc{}),
                .rasterizer = rasterizer.value_or(RasterizerDesc{}),
                .blend = blend.value_or(BlendDesc{}),
            });
        },
        "program"_a,
        "input_layout"_a.none(),
        "framebuffer_layout"_a,
        "primitive_type"_a = GraphicsPipelineDesc().primitive_type,
        "depth_stencil"_a.none() = nb::none(),
        "rasterizer"_a.none() = nb::none(),
        "blend"_a.none() = nb::none(),
        D(Device, create_graphics_pipeline)
    );
    device.def(
        "create_graphics_pipeline",
        &Device::create_graphics_pipeline,
        "desc"_a,
        D(Device, create_graphics_pipeline)
    );

    device.def(
        "create_ray_tracing_pipeline",
        [](Device* self,
           ref<ShaderProgram> program,
           std::vector<HitGroupDesc> hit_groups,
           uint32_t max_recursion,
           uint32_t max_ray_payload_size,
           uint32_t max_attribute_size,
           RayTracingPipelineFlags flags)
        {
            return self->create_ray_tracing_pipeline({
                .program = std::move(program),
                .hit_groups = std::move(hit_groups),
                .max_recursion = max_recursion,
                .max_ray_payload_size = max_ray_payload_size,
                .max_attribute_size = max_attribute_size,
                .flags = flags,
            });
        },
        "program"_a,
        "hit_groups"_a,
        "max_recursion"_a = RayTracingPipelineDesc().max_recursion,
        "max_ray_payload_size"_a = RayTracingPipelineDesc().max_ray_payload_size,
        "max_attribute_size"_a = RayTracingPipelineDesc().max_attribute_size,
        "flags"_a = RayTracingPipelineDesc().flags,
        D(Device, create_ray_tracing_pipeline)
    );
    device.def(
        "create_ray_tracing_pipeline",
        &Device::create_ray_tracing_pipeline,
        "desc"_a,
        D(Device, create_ray_tracing_pipeline)
    );

    device.def(
        "create_compute_kernel",
        [](Device* self, ref<ShaderProgram> program)
        { return self->create_compute_kernel({.program = std::move(program)}); },
        "program"_a,
        D(Device, create_compute_kernel)
    );
    device.def("create_compute_kernel", &Device::create_compute_kernel, "desc"_a, D(Device, create_compute_kernel));

    device.def(
        "create_memory_heap",
        [](Device* self,
           MemoryType memory_type,
           ResourceUsage usage,
           DeviceSize page_size,
           bool retain_large_pages,
           std::string debug_name)
        {
            return self->create_memory_heap({
                .memory_type = memory_type,
                .usage = usage,
                .page_size = page_size,
                .retain_large_pages = retain_large_pages,
                .debug_name = std::move(debug_name),
            });
        },
        "memory_type"_a,
        "usage"_a,
        "page_size"_a = MemoryHeapDesc().page_size,
        "retain_large_pages"_a = MemoryHeapDesc().retain_large_pages,
        "debug_name"_a = MemoryHeapDesc().debug_name,
        D(Device, create_memory_heap)
    );
    device.def("create_memory_heap", &Device::create_memory_heap, "desc"_a, D(Device, create_memory_heap));

    device.def_prop_ro("upload_heap", &Device::upload_heap, D(Device, upload_heap));
    device.def_prop_ro("read_back_heap", &Device::read_back_heap, D(Device, read_back_heap));
    device.def("flush_print", &Device::flush_print, D(Device, flush_print));
    device.def("flush_print_to_string", &Device::flush_print_to_string, D(Device, flush_print_to_string));
    device.def("run_garbage_collection", &Device::run_garbage_collection, D(Device, run_garbage_collection));
    device.def("wait", &Device::wait, D(Device, wait));

    device.def_static(
        "enumerate_adapters",
        &Device::enumerate_adapters,
        "type"_a = DeviceType::automatic,
        D(Device, enumerate_adapters)
    );
    device.def_static("report_live_objects", &Device::report_live_objects, D(Device, report_live_objects));
}
