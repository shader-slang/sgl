// SPDX-License-Identifier: Apache-2.0

#include "device.h"

#include "sgl/device/surface.h"
#include "sgl/device/resource.h"
#include "sgl/device/sampler.h"
#include "sgl/device/fence.h"
#include "sgl/device/query.h"
#include "sgl/device/input_layout.h"
#include "sgl/device/shader.h"
#include "sgl/device/shader_object.h"
#include "sgl/device/pipeline.h"
#include "sgl/device/kernel.h"
#include "sgl/device/raytracing.h"
#include "sgl/device/memory_heap.h"
#include "sgl/device/command.h"
#include "sgl/device/helpers.h"
#include "sgl/device/native_handle_traits.h"
#include "sgl/device/agility_sdk.h"
#include "sgl/device/cuda_utils.h"
#include "sgl/device/cuda_interop.h"
#include "sgl/device/print.h"
#include "sgl/device/blit.h"
#include "sgl/device/hot_reload.h"

#include "sgl/core/file_system_watcher.h"
#include "sgl/core/config.h"
#include "sgl/core/error.h"
#include "sgl/core/window.h"
#include "sgl/core/string.h"

#if SGL_HAS_D3D12
#include <dxgi.h>
#include <d3d12.h>
#include <comdef.h>
#endif

#include <mutex>

namespace sgl {

static constexpr size_t TEXTURE_UPLOAD_ALIGNMENT = 512;

static std::vector<Device*> s_devices;
static std::mutex s_devices_mutex;

class DebugLogger : public rhi::IDebugCallback {
public:
    DebugLogger()
    {
        m_logger = Logger::create(LogLevel::debug, "gfx", false);
        m_logger->use_same_outputs(Logger::get());
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    handleMessage(rhi::DebugMessageType type, rhi::DebugMessageSource source, const char* message)
    {
        const char* source_str = "";
        switch (source) {
        case rhi::DebugMessageSource::Layer:
            source_str = "layer";
            break;
        case rhi::DebugMessageSource::Driver:
            source_str = "driver";
            break;
        case rhi::DebugMessageSource::Slang:
            source_str = "slang";
            break;
        }
        switch (type) {
        case rhi::DebugMessageType::Info:
            m_logger->info("{}: {}", source_str, message);
            break;
        case rhi::DebugMessageType::Warning:
            m_logger->warn("{}: {}", source_str, message);
            break;
        case rhi::DebugMessageType::Error:
            m_logger->error("{}: {}", source_str, message);
            break;
        }
    }

    static DebugLogger& get()
    {
        static DebugLogger instance;
        return instance;
    }

private:
    ref<Logger> m_logger;
};

Device::Device(const DeviceDesc& desc)
    : m_desc(desc)
{
    ConstructorRefGuard ref_guard(this);

    if (desc.enable_debug_layers)
        rhi::getRHI()->enableDebugLayers();

    // Create hot reload system before creating any sessions.
    if (m_desc.enable_hot_reload)
        m_hot_reload = make_ref<HotReload>(ref<Device>(this));

    SLANG_CALL(slang::createGlobalSession(m_global_session.writeRef()));

    // Setup path for slang's downstream compilers.
    for (SlangPassThrough pass_through : {SLANG_PASS_THROUGH_DXC, SLANG_PASS_THROUGH_GLSLANG}) {
        m_global_session->setDownstreamCompilerPath(pass_through, platform::runtime_directory().string().c_str());
    }

    if (m_desc.type == DeviceType::automatic) {
#if SGL_WINDOWS
        m_desc.type = DeviceType::d3d12;
#elif SGL_LINUX || SGL_MACOS
        m_desc.type = DeviceType::vulkan;
#endif
    }

    // Setup shader cache.
    if (m_desc.shader_cache_path) {
        m_shader_cache_enabled = true;
        m_shader_cache_path = *m_desc.shader_cache_path;
        if (m_shader_cache_path.is_relative())
            m_shader_cache_path = platform::app_data_directory() / m_shader_cache_path;
        std::filesystem::create_directories(m_shader_cache_path);
    }
    std::string rhi_shader_cache_path = (m_shader_cache_path / "gfx").string();

    // Setup extensions.
    rhi::D3D12DeviceExtendedDesc d3d12_extended_desc{
        .structType = rhi::StructType::D3D12DeviceExtendedDesc,
        .rootParameterShaderAttributeName = "root",
        .debugBreakOnD3D12Error = false,
        .highestShaderModel = 0,
    };

    rhi::DeviceDesc rhi_desc{
        .next = &d3d12_extended_desc,
        .deviceType = static_cast<rhi::DeviceType>(m_desc.type),
        .adapterLUID
        = m_desc.adapter_luid ? reinterpret_cast<const rhi::AdapterLUID*>(m_desc.adapter_luid->data()) : nullptr,
        .slang{
            .slangGlobalSession = m_global_session,
        },
        // TODO(slang-rhi) make configurable but default to true
        .enableValidation = true,
        .debugCallback = &DebugLogger::get(),
    };
    log_debug(
        "Creating graphics device (type: {}, luid: {}, shader_cache_path: {}).",
        m_desc.type,
        m_desc.adapter_luid,
        m_shader_cache_path
    );
    if (SLANG_FAILED(rhi::getRHI()->createDevice(rhi_desc, m_rhi_device.writeRef())))
        SGL_THROW("Failed to create device!");

    // Get device info.
    const rhi::DeviceInfo& rhi_device_info = m_rhi_device->getDeviceInfo();
    m_info.type = m_desc.type;
    m_info.api_name = rhi_device_info.apiName;
    m_info.adapter_name = rhi_device_info.adapterName;
    m_info.adapter_luid = m_desc.adapter_luid ? *m_desc.adapter_luid : AdapterLUID();
    m_info.timestamp_frequency = rhi_device_info.timestampFrequency;
    m_info.limits.max_texture_dimension_1d = rhi_device_info.limits.maxTextureDimension1D;
    m_info.limits.max_texture_dimension_2d = rhi_device_info.limits.maxTextureDimension2D;
    m_info.limits.max_texture_dimension_3d = rhi_device_info.limits.maxTextureDimension3D;
    m_info.limits.max_texture_dimension_cube = rhi_device_info.limits.maxTextureDimensionCube;
    m_info.limits.max_texture_array_layers = rhi_device_info.limits.maxTextureArrayLayers;
    m_info.limits.max_vertex_input_elements = rhi_device_info.limits.maxVertexInputElements;
    m_info.limits.max_vertex_input_element_offset = rhi_device_info.limits.maxVertexInputElementOffset;
    m_info.limits.max_vertex_streams = rhi_device_info.limits.maxVertexStreams;
    m_info.limits.max_vertex_stream_stride = rhi_device_info.limits.maxVertexStreamStride;
    m_info.limits.max_compute_threads_per_group = rhi_device_info.limits.maxComputeThreadsPerGroup;
    m_info.limits.max_compute_thread_group_size = uint3(
        rhi_device_info.limits.maxComputeThreadGroupSize[0],
        rhi_device_info.limits.maxComputeThreadGroupSize[1],
        rhi_device_info.limits.maxComputeThreadGroupSize[2]
    );
    m_info.limits.max_compute_dispatch_thread_groups = uint3(
        rhi_device_info.limits.maxComputeDispatchThreadGroups[0],
        rhi_device_info.limits.maxComputeDispatchThreadGroups[1],
        rhi_device_info.limits.maxComputeDispatchThreadGroups[2]
    );
    m_info.limits.max_viewports = rhi_device_info.limits.maxViewports;
    m_info.limits.max_viewport_dimensions
        = uint2(rhi_device_info.limits.maxViewportDimensions[0], rhi_device_info.limits.maxViewportDimensions[1]);
    m_info.limits.max_framebuffer_dimensions = uint3(
        rhi_device_info.limits.maxFramebufferDimensions[0],
        rhi_device_info.limits.maxFramebufferDimensions[1],
        rhi_device_info.limits.maxFramebufferDimensions[2]
    );
    m_info.limits.max_shader_visible_samplers = rhi_device_info.limits.maxShaderVisibleSamplers;

    // Get supported shader model.
    const std::vector<std::pair<ShaderModel, const char*>> available_shader_models = {
        {ShaderModel::sm_6_7, "sm_6_7"},
        {ShaderModel::sm_6_6, "sm_6_6"},
        {ShaderModel::sm_6_5, "sm_6_5"},
        {ShaderModel::sm_6_4, "sm_6_4"},
        {ShaderModel::sm_6_3, "sm_6_3"},
        {ShaderModel::sm_6_2, "sm_6_2"},
        {ShaderModel::sm_6_1, "sm_6_1"},
        {ShaderModel::sm_6_0, "sm_6_0"},
    };
    for (const auto& [sm, sm_str] : available_shader_models) {
        if (m_rhi_device->hasFeature(sm_str)) {
            m_supported_shader_model = sm;
            break;
        }
    }
    if (m_supported_shader_model == ShaderModel::unknown) {
        m_supported_shader_model = ShaderModel::sm_6_0;
        log_warn("No supported shader model found, pretending to support {}.", m_supported_shader_model);
    }
    log_debug("Supported shader model: {}", m_supported_shader_model);

    // Get features.
    const char* features[256];
    uint32_t feature_count = 0;
    SLANG_CALL(m_rhi_device->getFeatures(features, std::size(features), &feature_count));
    for (uint32_t i = 0; i < feature_count; ++i)
        m_features.push_back(features[i]);
    log_debug("Supported features: {}", string::join(m_features, ", "));

    // Create graphics queue.
    SLANG_CALL(m_rhi_device->getQueue(rhi::QueueType::Graphics, m_rhi_graphics_queue.writeRef()));

    // Create default slang session.
    m_slang_session = create_slang_session({
        .compiler_options = m_desc.compiler_options,
        .add_default_include_paths = true,
        .cache_path = m_shader_cache_enabled ? std::optional(m_shader_cache_path) : std::nullopt,
    });

    // Create global fence to synchronize command submission.
    m_global_fence = create_fence({.shared = m_desc.enable_cuda_interop});

    // Setup CUDA interop.
    if (m_desc.enable_cuda_interop) {
        SGL_CHECK(sgl_cuda_api_init(), "Failed to initialize CUDA driver API.");
        m_cuda_device = make_ref<cuda::Device>(this);
        m_cuda_semaphore = make_ref<cuda::ExternalSemaphore>(m_global_fence);
        m_supports_cuda_interop = true;
    }

    m_upload_heap = create_memory_heap(
        {.memory_type = MemoryType::upload,
         .usage = BufferUsage::copy_source,
         .page_size = 1024 * 1024 * 4,
         .label = "default_upload_heap"}
    );

    m_read_back_heap = create_memory_heap(
        {.memory_type = MemoryType::read_back,
         .usage = BufferUsage::copy_destination,
         .page_size = 1024 * 1024 * 4,
         .label = "default_read_back_heap"}
    );

    if (m_desc.enable_print)
        m_debug_printer = std::make_unique<DebugPrinter>(this);

    // Add device to global device list.
    {
        std::lock_guard lock(s_devices_mutex);
        s_devices.push_back(this);
    }
}

Device::~Device()
{
    // Remove device from global device list.
    {
        std::lock_guard lock(s_devices_mutex);
        s_devices.erase(std::remove(s_devices.begin(), s_devices.end(), this), s_devices.end());
    }

    SGL_CHECK(m_closed, "Device is not close. Call close() before destroying the device.");

    m_rhi_graphics_queue.setNull();
    m_rhi_device.setNull();
}

ShaderCacheStats Device::shader_cache_stats() const
{
    // TODO(slang-rhi)
    return {
        .entry_count = 0,
        .hit_count = 0,
        .miss_count = 0,
    };
}

FormatSupport Device::get_format_support(Format format) const
{
    rhi::FormatSupport rhi_format_support;
    SLANG_CALL(m_rhi_device->getFormatSupport(static_cast<rhi::Format>(format), &rhi_format_support));
    return static_cast<FormatSupport>(rhi_format_support);
}

void Device::close()
{
    if (m_closed)
        return;

    log_debug("Closing device {}", fmt::ptr(this));

    wait();

    // Handle device close callbacks
    for (const DeviceCloseCallback& callback : m_device_close_callbacks)
        callback(this);

    // Make sure Device's ref count is not going to zero when releasing resources.
    inc_ref();

    m_closed = true;

    m_shader_hot_reload_callbacks.clear();
    m_device_close_callbacks.clear();

    m_blitter.reset();
    m_debug_printer.reset();

    m_read_back_heap.reset();
    m_upload_heap.reset();

    m_global_fence.reset();

    m_slang_session.reset();
    m_hot_reload.reset();
    m_coop_vec.reset();

    dec_ref();
}

void Device::close_all_devices()
{
    std::vector<Device*> devices;
    {
        std::lock_guard lock(s_devices_mutex);
        devices = s_devices;
    }
    for (Device* device : devices)
        device->close();
}

ref<Surface> Device::create_surface(Window* window)
{
    return make_ref<Surface>(window, ref<Device>(this));
}

ref<Surface> Device::create_surface(WindowHandle window_handle)
{
    return make_ref<Surface>(window_handle, ref<Device>(this));
}

ref<Buffer> Device::create_buffer(BufferDesc desc)
{
    return make_ref<Buffer>(ref<Device>(this), std::move(desc));
}

ref<BufferView> Device::create_buffer_view(Buffer* buffer, BufferViewDesc desc)
{
    return make_ref<BufferView>(ref<Device>(this), ref<Buffer>(buffer), std::move(desc));
}

ref<Texture> Device::create_texture(TextureDesc desc)
{
    return make_ref<Texture>(ref<Device>(this), std::move(desc));
}

ref<Texture> Device::create_texture_from_resource(TextureDesc desc, rhi::ITexture* resource)
{
    return make_ref<Texture>(ref<Device>(this), std::move(desc), resource);
}

ref<TextureView> Device::create_texture_view(Texture* texture, TextureViewDesc desc)
{
    return make_ref<TextureView>(ref<Device>(this), ref<Texture>(texture), std::move(desc));
}

ref<Sampler> Device::create_sampler(SamplerDesc desc)
{
    return make_ref<Sampler>(ref<Device>(this), std::move(desc));
}

ref<Fence> Device::create_fence(FenceDesc desc)
{
    return make_ref<Fence>(ref<Device>(this), std::move(desc));
}

ref<QueryPool> Device::create_query_pool(QueryPoolDesc desc)
{
    return make_ref<QueryPool>(ref<Device>(this), std::move(desc));
}

ref<InputLayout> Device::create_input_layout(InputLayoutDesc desc)
{
    return make_ref<InputLayout>(ref<Device>(this), std::move(desc));
}

#if 0
AccelerationStructurePrebuildInfo
Device::get_acceleration_structure_prebuild_info(const AccelerationStructureBuildInputs& build_inputs)
{
    const rhi::IAccelerationStructure::BuildInputs& rhi_build_inputs
        = reinterpret_cast<const rhi::IAccelerationStructure::BuildInputs&>(build_inputs);
    rhi::IAccelerationStructure::PrebuildInfo rhi_prebuild_info;
    SLANG_CALL(m_rhi_device->getAccelerationStructurePrebuildInfo(rhi_build_inputs, &rhi_prebuild_info));
    return AccelerationStructurePrebuildInfo{
        .result_data_max_size = rhi_prebuild_info.resultDataMaxSize,
        .scratch_data_size = rhi_prebuild_info.scratchDataSize,
        .update_scratch_data_size = rhi_prebuild_info.updateScratchDataSize,
    };
}
#endif

ref<AccelerationStructure> Device::create_acceleration_structure(AccelerationStructureDesc desc)
{
    return make_ref<AccelerationStructure>(ref<Device>(this), std::move(desc));
}

/*size_t Device::query_coopvec_matrix_size(uint32_t rows, uint32_t columns, CoopVecMatrixLayout layout)
{
    return m_coop_vec->query_matrix_size(rows, columns, layout);
}*/

ref<CoopVec> Device::get_or_create_coop_vec()
{
    if (!m_coop_vec)
        m_coop_vec.reset(new CoopVec(ref<Device>(this)));
    return m_coop_vec;
}

ref<ShaderTable> Device::create_shader_table(ShaderTableDesc desc)
{
    return make_ref<ShaderTable>(ref<Device>(this), std::move(desc));
}

ref<SlangSession> Device::create_slang_session(SlangSessionDesc desc)
{
    return make_ref<SlangSession>(ref<Device>(this), std::move(desc));
}

void Device::reload_all_programs()
{
    if (m_hot_reload)
        m_hot_reload->recreate_all_sessions();
}

ref<SlangModule> Device::load_module(std::string_view module_name)
{
    return m_slang_session->load_module(module_name);
}

ref<SlangModule> Device::load_module_from_source(
    std::string_view module_name,
    std::string_view source,
    std::optional<std::filesystem::path> path
)
{
    return m_slang_session->load_module_from_source(module_name, source, path);
}

ref<ShaderProgram> Device::link_program(
    std::vector<ref<SlangModule>> modules,
    std::vector<ref<SlangEntryPoint>> entry_points,
    std::optional<SlangLinkOptions> link_options
)
{
    return m_slang_session->link_program(std::move(modules), std::move(entry_points), link_options);
}

ref<ShaderProgram> Device::load_program(
    std::string_view module_name,
    std::vector<std::string_view> entry_point_names,
    std::optional<std::string_view> additional_source,
    std::optional<SlangLinkOptions> link_options
)
{
    return m_slang_session->load_program(module_name, entry_point_names, additional_source, link_options);
}

ref<ShaderObject> Device::create_root_shader_object(const ShaderProgram* shader_program)
{
    Slang::ComPtr<rhi::IShaderObject> rhi_shader_object;
    SLANG_CALL(m_rhi_device->createRootShaderObject(shader_program->rhi_shader_program(), rhi_shader_object.writeRef())
    );

    ref<ShaderObject> shader_object = make_ref<ShaderObject>(ref<Device>(this), rhi_shader_object);

    // Bind the debug printer to the new shader object, if enabled.
    if (m_debug_printer)
        m_debug_printer->bind(shader_object.get());

    return shader_object;
}

ref<ShaderObject> Device::create_shader_object(const TypeLayoutReflection* type_layout)
{
    Slang::ComPtr<rhi::IShaderObject> rhi_shader_object;
    SLANG_CALL(m_rhi_device->createShaderObjectFromTypeLayout(
        type_layout->get_slang_type_layout(),
        rhi_shader_object.writeRef()
    ));

    return make_ref<ShaderObject>(ref<Device>(this), rhi_shader_object);
}

ref<ShaderObject> Device::create_shader_object(ReflectionCursor cursor)
{
    SGL_CHECK(cursor.is_valid(), "Invalid reflection cursor");
    return create_shader_object(cursor.type_layout().get());
}

ref<ComputePipeline> Device::create_compute_pipeline(ComputePipelineDesc desc)
{
    return make_ref<ComputePipeline>(ref<Device>(this), std::move(desc));
}

ref<RenderPipeline> Device::create_render_pipeline(RenderPipelineDesc desc)
{
    return make_ref<RenderPipeline>(ref<Device>(this), std::move(desc));
}

ref<RayTracingPipeline> Device::create_ray_tracing_pipeline(RayTracingPipelineDesc desc)
{
    return make_ref<RayTracingPipeline>(ref<Device>(this), std::move(desc));
}

ref<ComputeKernel> Device::create_compute_kernel(ComputeKernelDesc desc)
{
    return make_ref<ComputeKernel>(ref(this), std::move(desc));
}

ref<CommandEncoder> Device::create_command_encoder(CommandQueueType queue)
{
    SGL_CHECK(queue == CommandQueueType::graphics, "Only graphics queue is supported.");

    Slang::ComPtr<rhi::ICommandEncoder> rhi_command_encoder;
    SLANG_CALL(m_rhi_graphics_queue->createCommandEncoder(rhi_command_encoder.writeRef()));
    return make_ref<CommandEncoder>(ref(this), rhi_command_encoder);
}

uint64_t Device::submit_command_buffer(CommandBuffer* command_buffer, CommandQueueType queue)
{
    SGL_CHECK_NOT_NULL(command_buffer);
    SGL_CHECK(queue == CommandQueueType::graphics, "Only graphics queue is supported.");

    // TODO make parameter
    void* cuda_stream = 0;

    if (m_supports_cuda_interop && command_buffer->m_cuda_interop_buffers.size() > 0) {
        for (const auto& buffer : command_buffer->m_cuda_interop_buffers)
            buffer->copy_from_cuda(cuda_stream);

        sync_to_cuda(cuda_stream);
    }

    uint64_t fence_value = m_global_fence->update_signaled_value();
    m_rhi_graphics_queue->submit(command_buffer->rhi_command_buffer(), m_global_fence->rhi_fence(), fence_value);

    if (m_supports_cuda_interop && command_buffer->m_cuda_interop_buffers.size() > 0) {
        sync_to_device(cuda_stream);

        for (const auto& buffer : command_buffer->m_cuda_interop_buffers)
            if (buffer->is_uav())
                buffer->copy_to_cuda(cuda_stream);
    }

    return fence_value;
}

bool Device::is_command_buffer_complete(uint64_t id)
{
    return id <= m_global_fence->current_value();
}

void Device::wait_command_buffer(uint64_t id)
{
    m_global_fence->wait(id);
}

void Device::wait_for_idle(CommandQueueType queue)
{
    SGL_CHECK(queue == CommandQueueType::graphics, "Only graphics queue is supported.");
    m_rhi_graphics_queue->waitOnHost();
}

void Device::sync_to_cuda(void* cuda_stream)
{
    // Signal fence from CUDA, wait for it on graphics queue.
    SGL_CU_SCOPE(this);
    uint64_t signal_value = m_global_fence->update_signaled_value();
    m_cuda_semaphore->signal(signal_value, CUstream(cuda_stream));
    rhi::IFence* fence = m_global_fence->rhi_fence();
    m_rhi_graphics_queue->waitForFenceValuesOnDevice(1, &fence, &signal_value);
}

void Device::sync_to_device(void* cuda_stream)
{
    SGL_CU_SCOPE(this);
    m_cuda_semaphore->wait(m_global_fence->signaled_value(), CUstream(cuda_stream));
}

void Device::run_garbage_collection()
{
    uint64_t signaled_value = m_global_fence->signaled_value();

    // Execute deferred releases on the upload and read-back heaps.
    m_upload_heap->execute_deferred_releases();
    m_read_back_heap->execute_deferred_releases();

    uint64_t current_value = m_global_fence->current_value();

    // Update hot reload system if created.
    if (m_hot_reload)
        m_hot_reload->update();
}

ref<MemoryHeap> Device::create_memory_heap(MemoryHeapDesc desc)
{
    return make_ref<MemoryHeap>(ref<Device>(this), m_global_fence, std::move(desc));
}

void Device::flush_print()
{
    if (m_debug_printer)
        m_debug_printer->flush();
}

std::string Device::flush_print_to_string()
{
    return m_debug_printer ? m_debug_printer->flush_to_string() : "";
}

void Device::wait()
{
    wait_for_idle();
    run_garbage_collection();
}

void Device::upload_buffer_data(Buffer* buffer, const void* data, size_t size, size_t offset)
{
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK(offset + size <= buffer->size(), "Buffer write is out of bounds");
    SGL_CHECK_NOT_NULL(data);

    auto alloc = m_upload_heap->allocate(size, TEXTURE_UPLOAD_ALIGNMENT);

    std::memcpy(alloc->data, data, size);

    auto command_encoder = create_command_encoder();
    command_encoder->copy_buffer(buffer, offset, alloc->buffer, alloc->offset, size);
    submit_command_buffer(command_encoder->finish());
}

void Device::read_buffer_data(const Buffer* buffer, void* data, size_t size, size_t offset)
{
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK(offset + size <= buffer->size(), "Buffer read is out of bounds");
    SGL_CHECK_NOT_NULL(data);

    auto alloc = m_read_back_heap->allocate(size, TEXTURE_UPLOAD_ALIGNMENT);

    ref<CommandEncoder> command_encoder = create_command_encoder();
    command_encoder->copy_buffer(alloc->buffer, alloc->offset, buffer, offset, size);
    submit_command_buffer(command_encoder->finish());
    wait_for_idle();

    std::memcpy(data, alloc->data, size);
}

void Device::upload_texture_data(Texture* texture, uint32_t subresource, SubresourceData subresource_data)
{
    SGL_CHECK_NOT_NULL(texture);
    SGL_CHECK_LT(subresource, texture->subresource_count());

    ref<CommandEncoder> command_encoder = create_command_encoder();
    command_encoder->upload_texture_data(texture, subresource, subresource_data);
    submit_command_buffer(command_encoder->finish());
}

OwnedSubresourceData Device::read_texture_data(const Texture* texture, uint32_t subresource)
{
    SGL_CHECK_NOT_NULL(texture);
    SGL_CHECK_LT(subresource, texture->subresource_count());

    SubresourceLayout layout = texture->get_subresource_layout(subresource);

    size_t size = layout.total_size_aligned();
    auto alloc = m_read_back_heap->allocate(size, TEXTURE_UPLOAD_ALIGNMENT);

    ref<CommandEncoder> command_encoder = create_command_encoder();
    command_encoder->copy_texture_to_buffer(
        alloc->buffer,
        alloc->offset,
        alloc->size,
        layout.row_pitch_aligned,
        texture,
        subresource
    );
    submit_command_buffer(command_encoder->finish());
    wait_for_idle();

    OwnedSubresourceData subresource_data;
    subresource_data.size = layout.total_size();
    subresource_data.owned_data = std::make_unique<uint8_t[]>(subresource_data.size);
    subresource_data.data = subresource_data.owned_data.get();
    subresource_data.row_pitch = layout.row_pitch;
    subresource_data.slice_pitch = layout.row_count * layout.row_pitch;

    const uint8_t* src = alloc->data;
    uint8_t* dst = subresource_data.owned_data.get();
    for (uint32_t depth = 0; depth < layout.depth; ++depth) {
        for (uint32_t row = 0; row < layout.row_count; ++row) {
            std::memcpy(dst, src, layout.row_pitch);
            src += layout.row_pitch_aligned;
            dst += layout.row_pitch;
        }
    }

    return subresource_data;
}

NativeHandle Device::get_native_handle(uint32_t index) const
{
    rhi::DeviceNativeHandles handles = {};
    SLANG_CALL(m_rhi_device->getNativeDeviceHandles(&handles));

#if SGL_HAS_D3D12
    if (type() == DeviceType::d3d12) {
        SGL_ASSERT(index == 0);
        if (index == 0)
            return NativeHandle(reinterpret_cast<ID3D12Device*>(handles.handles[0].value));
    }
#endif
#if SGL_HAS_VULKAN
    if (type() == DeviceType::vulkan) {
        SGL_ASSERT(index < 3);
        if (index == 0)
            return NativeHandle(reinterpret_cast<VkInstance>(handles.handles[0].value));
        else if (index == 1)
            return NativeHandle(reinterpret_cast<VkPhysicalDevice>(handles.handles[1].value));
        else if (index == 2)
            return NativeHandle(reinterpret_cast<VkDevice>(handles.handles[2].value));
    }
#endif
    return {};
}

NativeHandle Device::get_native_command_queue_handle(CommandQueueType queue) const
{
    SGL_CHECK(queue == CommandQueueType::graphics, "Only graphics queue is supported.");
    rhi::NativeHandle rhi_handle = {};
    SLANG_CALL(m_rhi_graphics_queue->getNativeHandle(&rhi_handle));
    return NativeHandle(rhi_handle);
}

std::vector<AdapterInfo> Device::enumerate_adapters(DeviceType type)
{
    if (type == DeviceType::automatic) {
#if SGL_WINDOWS
        type = DeviceType::d3d12;
#elif SGL_LINUX
        type = DeviceType::vulkan;
#elif SGL_MACOS
        type = DeviceType::metal;
#endif
    }

    auto convert_luid = [](const rhi::AdapterLUID& rhi_luid) -> AdapterLUID
    {
        AdapterLUID luid;
        for (size_t i = 0; i < 16; ++i)
            luid[i] = rhi_luid.luid[i];
        return luid;
    };

    rhi::AdapterList rhi_adapters = rhi::getRHI()->getAdapters(static_cast<rhi::DeviceType>(type));

    std::vector<AdapterInfo> adapters(rhi_adapters.getCount());
    for (size_t i = 0; i < adapters.size(); ++i) {
        const auto& rhi_adapter = rhi_adapters.getAdapters()[i];
        adapters[i] = AdapterInfo{
            .name = rhi_adapter.name,
            .vendor_id = rhi_adapter.vendorID,
            .device_id = rhi_adapter.deviceID,
            .luid = convert_luid(rhi_adapter.luid),
        };
    }

    return adapters;
}

void Device::report_live_objects()
{
    rhi::getRHI()->reportLiveObjects();
}

bool Device::enable_agility_sdk()
{
#if SGL_HAS_D3D12 && SGL_HAS_AGILITY_SDK
    std::filesystem::path exe_dir = platform::executable_directory();
    std::filesystem::path sdk_dir = platform::runtime_directory() / SGL_AGILITY_SDK_PATH;

    // Agility SDK can only be loaded from a relative path to the executable.
    // Make sure both paths use the same drive letter.
    if (std::tolower(exe_dir.string()[0]) != std::tolower(sdk_dir.string()[0])) {
        log_warn(
            "Cannot enable D3D12 Agility SDK: "
            "Executable directory \"{}\" is not on the same drive as the SDK directory \"{}\".",
            exe_dir,
            sdk_dir
        );
        return false;
    }

    // Get relative path and make sure there is the required trailing path delimiter.
    auto rel_path = std::filesystem::relative(sdk_dir, exe_dir) / "";

    // Load D3D12 library.
    LoadLibraryA("d3d12.dll");
    HMODULE handle = GetModuleHandleA("d3d12.dll");

    // Get the D3D12GetInterface procedure.
    typedef HRESULT(WINAPI * D3D12GetInterfaceFn)(REFCLSID rclsid, REFIID riid, void** ppvDebug);
    D3D12GetInterfaceFn pD3D12GetInterface
        = handle ? (D3D12GetInterfaceFn)GetProcAddress(handle, "D3D12GetInterface") : nullptr;
    if (!pD3D12GetInterface) {
        log_warn("Cannot enable D3D12 Agility SDK: "
                 "Failed to get D3D12GetInterface.");
        return false;
    }

    // Local definition of CLSID_D3D12SDKConfiguration from d3d12.h
    const GUID CLSID_D3D12SDKConfiguration__
        = {0x7cda6aca, 0xa03e, 0x49c8, {0x94, 0x58, 0x03, 0x34, 0xd2, 0x0e, 0x07, 0xce}};
    // Get the D3D12SDKConfiguration interface.
    _COM_SMARTPTR_TYPEDEF(ID3D12SDKConfiguration, __uuidof(ID3D12SDKConfiguration));
    ID3D12SDKConfigurationPtr pD3D12SDKConfiguration;
    if (!SUCCEEDED(pD3D12GetInterface(CLSID_D3D12SDKConfiguration__, IID_PPV_ARGS(&pD3D12SDKConfiguration)))) {
        log_warn("Cannot enable D3D12 Agility SDK: "
                 "Failed to get D3D12SDKConfiguration interface.");
        return false;
    }

    // Set the SDK version and path.
    if (!SUCCEEDED(pD3D12SDKConfiguration->SetSDKVersion(SGL_AGILITY_SDK_VERSION, rel_path.string().c_str()))) {
        log_warn("Cannot enable D3D12 Agility SDK: "
                 "Calling SetSDKVersion failed.");
        return false;
    }

    return true;
#endif
    return false;
}

std::string Device::to_string() const
{
    return fmt::format(
        "Device(\n"
        "  type = {},\n"
        "  adapter_name = \"{}\",\n"
        "  adapter_luid = {},\n"
        "  enable_debug_layers = {},\n"
        "  supported_shader_model = {},\n"
        "  shader_cache_enabled = {},\n"
        "  shader_cache_path = \"{}\"\n"
        ")",
        m_info.type,
        m_info.adapter_name,
        string::hexlify(m_info.adapter_luid),
        m_desc.enable_debug_layers,
        m_supported_shader_model,
        m_shader_cache_enabled,
        m_shader_cache_path
    );
}

Blitter* Device::_blitter()
{
    if (!m_blitter)
        m_blitter = ref(new Blitter(this));
    return m_blitter;
}

} // namespace sgl
