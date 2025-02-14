// SPDX-License-Identifier: Apache-2.0

#include "device.h"

#include "sgl/device/swapchain.h"
#include "sgl/device/resource.h"
#include "sgl/device/sampler.h"
#include "sgl/device/fence.h"
#include "sgl/device/query.h"
#include "sgl/device/input_layout.h"
#include "sgl/device/framebuffer.h"
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

#if SGL_HAS_NVAPI
#include <nvapi.h>
#endif

#include <mutex>

namespace sgl {

static constexpr size_t TEXTURE_UPLOAD_ALIGNMENT = 512;

static std::vector<Device*> s_devices;
static std::mutex s_devices_mutex;

class DebugLogger : public gfx::IDebugCallback {
public:
    DebugLogger()
    {
        m_logger = Logger::create(LogLevel::debug, "gfx", false);
        m_logger->use_same_outputs(Logger::get());
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    handleMessage(gfx::DebugMessageType type, gfx::DebugMessageSource source, const char* message)
    {
        const char* source_str = "";
        switch (source) {
        case gfx::DebugMessageSource::Layer:
            source_str = "layer";
            break;
        case gfx::DebugMessageSource::Driver:
            source_str = "driver";
            break;
        case gfx::DebugMessageSource::Slang:
            source_str = "slang";
            break;
        }
        switch (type) {
        case gfx::DebugMessageType::Info:
            m_logger->info("{}: {}", source_str, message);
            break;
        case gfx::DebugMessageType::Warning:
            m_logger->warn("{}: {}", source_str, message);
            break;
        case gfx::DebugMessageType::Error:
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

#if SGL_HAS_NVAPI
// In order to use NVAPI, we intercept the pipeline state creation calls in the gfx layer
// and dispatch into the `NVAPI_Create*PipelineState()` functions.
// This is done by implementing the `gfx::IPipelineCreationAPIDispatcher` interface,
// and passing an instance to `gfxCreateDevice`.
class Device::PipelineCreationAPIDispatcher : public gfx::IPipelineCreationAPIDispatcher {
public:
    PipelineCreationAPIDispatcher()
    {
        if (NvAPI_Initialize() != NVAPI_OK)
            SGL_THROW("Failed to initialize NVAPI.");
    }

    ~PipelineCreationAPIDispatcher()
    {
        if (NvAPI_Unload() != NVAPI_OK)
            SGL_THROW("Failed to unload NVAPI.");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(const SlangUUID& uuid, void** outObject) override
    {
        if (uuid == SlangUUID SLANG_UUID_IPipelineCreationAPIDispatcher) {
            *outObject = static_cast<gfx::IPipelineCreationAPIDispatcher*>(this);
            return SLANG_OK;
        }
        return SLANG_E_NO_INTERFACE;
    }

    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override
    {
        // The lifetime of this object is tied to the device.
        // Do not perform any reference counting.
        return 2;
    }

    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override
    {
        // Returning 2 is important here, because when releasing a COM pointer, it checks
        // if the ref count **was 1 before releasing** in order to free the object.
        return 2;
    }

    // This method will be called by the gfx layer to create an API object for a compute pipeline state.
    virtual gfx::Result createComputePipelineState(
        gfx::IDevice* device,
        slang::IComponentType* program,
        void* pipelineDesc,
        void** outPipelineState
    )
    {
        gfx::IDevice::InteropHandles nativeHandle;
        SLANG_CALL(device->getNativeDeviceHandles(&nativeHandle));
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(nativeHandle.handles[0].handleValue);

        uint32_t uavSpace, uavSlot;
        if (findNvApiShaderParameter(program, uavSpace, uavSlot)) {
            auto desc = createShaderExtensionSlotDesc(uavSpace, uavSlot);
            const NVAPI_D3D12_PSO_EXTENSION_DESC* ppPSOExtensionsDesc[1] = {&desc};
            auto result = NvAPI_D3D12_CreateComputePipelineState(
                pD3D12Device,
                reinterpret_cast<D3D12_COMPUTE_PIPELINE_STATE_DESC*>(pipelineDesc),
                1,
                ppPSOExtensionsDesc,
                (ID3D12PipelineState**)outPipelineState
            );
            return (result == NVAPI_OK) ? SLANG_OK : SLANG_FAIL;
        } else {
            ID3D12PipelineState* pState = nullptr;
            SLANG_RETURN_ON_FAIL(pD3D12Device->CreateComputePipelineState(
                reinterpret_cast<D3D12_COMPUTE_PIPELINE_STATE_DESC*>(pipelineDesc),
                IID_PPV_ARGS(&pState)
            ));
            *outPipelineState = pState;
        }
        return SLANG_OK;
    }

    // This method will be called by the gfx layer to create an API object for a graphics pipeline state.
    virtual gfx::Result createGraphicsPipelineState(
        gfx::IDevice* device,
        slang::IComponentType* program,
        void* pipelineDesc,
        void** outPipelineState
    )
    {
        gfx::IDevice::InteropHandles nativeHandle;
        SLANG_CALL(device->getNativeDeviceHandles(&nativeHandle));
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(nativeHandle.handles[0].handleValue);

        uint32_t uavSpace, uavSlot;
        if (findNvApiShaderParameter(program, uavSpace, uavSlot)) {
            auto desc = createShaderExtensionSlotDesc(uavSpace, uavSlot);
            const NVAPI_D3D12_PSO_EXTENSION_DESC* ppPSOExtensionsDesc[1] = {&desc};
            auto result = NvAPI_D3D12_CreateGraphicsPipelineState(
                pD3D12Device,
                reinterpret_cast<D3D12_GRAPHICS_PIPELINE_STATE_DESC*>(pipelineDesc),
                1,
                ppPSOExtensionsDesc,
                (ID3D12PipelineState**)outPipelineState
            );
            return (result == NVAPI_OK) ? SLANG_OK : SLANG_FAIL;
        } else {
            ID3D12PipelineState* pState = nullptr;
            SLANG_RETURN_ON_FAIL(pD3D12Device->CreateGraphicsPipelineState(
                reinterpret_cast<D3D12_GRAPHICS_PIPELINE_STATE_DESC*>(pipelineDesc),
                IID_PPV_ARGS(&pState)
            ));
            *outPipelineState = pState;
        }
        return SLANG_OK;
    }

    virtual gfx::Result createMeshPipelineState(
        gfx::IDevice* device,
        slang::IComponentType* program,
        void* pipelineDesc,
        void** outPipelineState
    )
    {
        SGL_UNUSED(device, program, pipelineDesc, outPipelineState);
        SGL_THROW("Mesh pipelines are not supported.");
    }

    // This method will be called by the gfx layer right before creating a ray tracing state object.
    virtual gfx::Result beforeCreateRayTracingState(gfx::IDevice* device, slang::IComponentType* program)
    {
        gfx::IDevice::InteropHandles nativeHandle;
        SLANG_CALL(device->getNativeDeviceHandles(&nativeHandle));
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(nativeHandle.handles[0].handleValue);

        uint32_t uavSpace, uavSlot;
        if (findNvApiShaderParameter(program, uavSpace, uavSlot)) {
            if (NvAPI_D3D12_SetNvShaderExtnSlotSpace(pD3D12Device, uavSlot, uavSpace) != NVAPI_OK) {
                SGL_THROW("Failed to set NVAPI extension.");
            }
        }
        return SLANG_OK;
    }

    // This method will be called by the gfx layer right after creating a ray tracing state object.
    virtual gfx::Result afterCreateRayTracingState(gfx::IDevice* device, slang::IComponentType* program)
    {
        gfx::IDevice::InteropHandles nativeHandle;
        SLANG_CALL(device->getNativeDeviceHandles(&nativeHandle));
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(nativeHandle.handles[0].handleValue);

        uint32_t uavSpace, uavSlot;
        if (findNvApiShaderParameter(program, uavSpace, uavSlot)) {
            if (NvAPI_D3D12_SetNvShaderExtnSlotSpace(pD3D12Device, 0xFFFFFFFF, 0) != NVAPI_OK) {
                SGL_THROW("Failed to set NVAPI extension.");
            }
        }
        return SLANG_OK;
    }

private:
    bool findNvApiShaderParameter(slang::IComponentType* program, uint32_t& uavSpace, uint32_t& uavSlot)
    {
        auto globalTypeLayout = program->getLayout()->getGlobalParamsVarLayout()->getTypeLayout();
        auto index = globalTypeLayout->findFieldIndexByName("g_NvidiaExt");
        if (index != -1) {
            auto field = globalTypeLayout->getFieldByIndex((unsigned int)index);
            uavSpace = field->getBindingSpace();
            uavSlot = field->getBindingIndex();
            return true;
        }
        return false;
    }

    NVAPI_D3D12_PSO_SET_SHADER_EXTENSION_SLOT_DESC createShaderExtensionSlotDesc(uint32_t uavSpace, uint32_t uavSlot)
    {
        NVAPI_D3D12_PSO_SET_SHADER_EXTENSION_SLOT_DESC desc = {};
        desc.psoExtension = NV_PSO_SET_SHADER_EXTNENSION_SLOT_AND_SPACE;
        desc.version = NV_SET_SHADER_EXTENSION_SLOT_DESC_VER;
        desc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
        desc.uavSlot = uavSlot;
        desc.registerSpace = uavSpace;
        return desc;
    }
};
#endif // SGL_HAS_NVAPI

inline gfx::DeviceType gfx_device_type(DeviceType device_type)
{
    switch (device_type) {
    case DeviceType::automatic:
        return gfx::DeviceType::Default;
    case DeviceType::d3d12:
        return gfx::DeviceType::DirectX12;
    case DeviceType::vulkan:
        return gfx::DeviceType::Vulkan;
    case DeviceType::metal:
        return gfx::DeviceType::Metal;
    case DeviceType::cpu:
        return gfx::DeviceType::CPU;
    case DeviceType::cuda:
        return gfx::DeviceType::CUDA;
    }
    SGL_UNREACHABLE();
}

Device::Device(const DeviceDesc& desc)
    : m_desc(desc)
{
    ConstructorRefGuard ref_guard(this);

    // Create hot reload system before creating any sessions.
    if (m_desc.enable_hot_reload)
        m_hot_reload = make_ref<HotReload>(ref<Device>(this));

    SLANG_CALL(slang::createGlobalSession(m_global_session.writeRef()));

    // Setup path for slang's downstream compilers.
    for (SlangPassThrough pass_through : {SLANG_PASS_THROUGH_DXC, SLANG_PASS_THROUGH_GLSLANG}) {
        m_global_session->setDownstreamCompilerPath(pass_through, platform::runtime_directory().string().c_str());
    }

    gfx::gfxSetDebugCallback(&DebugLogger::get());
    if (m_desc.enable_debug_layers) {
        log_debug("Enabling GFX debug layers.");
        gfx::gfxEnableDebugLayer();
    }

    if (m_desc.type == DeviceType::automatic) {
#if SGL_WINDOWS
        m_desc.type = DeviceType::d3d12;
#elif SGL_LINUX || SGL_MACOS
        m_desc.type = DeviceType::vulkan;
#endif
    }

#if SGL_HAS_NVAPI
    m_api_dispatcher.reset(new PipelineCreationAPIDispatcher());
#endif

    // Setup shader cache.
    if (m_desc.shader_cache_path) {
        m_shader_cache_enabled = true;
        m_shader_cache_path = *m_desc.shader_cache_path;
        if (m_shader_cache_path.is_relative())
            m_shader_cache_path = platform::app_data_directory() / m_shader_cache_path;
        std::filesystem::create_directories(m_shader_cache_path);
    }
    std::string gfx_shader_cache_path = (m_shader_cache_path / "gfx").string();

    // Setup extensions.
    std::vector<void*> extended_descs;
    gfx::D3D12DeviceExtendedDesc d3d12_extended_desc{
        .structType = gfx::StructType::D3D12DeviceExtendedDesc,
        .rootParameterShaderAttributeName = "root",
        .debugBreakOnD3D12Error = false,
        .highestShaderModel = 0,
    };
    if (m_desc.type == DeviceType::d3d12)
        extended_descs.push_back(&d3d12_extended_desc);

    gfx::IDevice::Desc gfx_desc
    {
        .deviceType = gfx_device_type(m_desc.type),
        .adapterLUID
            = m_desc.adapter_luid ? reinterpret_cast<const gfx::AdapterLUID*>(m_desc.adapter_luid->data()) : nullptr,
#if SGL_HAS_NVAPI
        .apiCommandDispatcher = m_api_dispatcher.get(),
#endif
        .shaderCache{
            .shaderCachePath = m_shader_cache_enabled ? gfx_shader_cache_path.c_str() : nullptr,
            .maxEntryCount = 4096,
        },
        .slang{
            .slangGlobalSession = m_global_session,
        },
        .extendedDescCount = narrow_cast<gfx::GfxCount>(extended_descs.size()), .extendedDescs = extended_descs.data(),
    };
    log_debug(
        "Creating graphics device (type: {}, luid: {}, shader_cache_path: {}).",
        m_desc.type,
        m_desc.adapter_luid,
        m_shader_cache_path
    );
    if (SLANG_FAILED(gfx::gfxCreateDevice(&gfx_desc, m_gfx_device.writeRef())))
        SGL_THROW("Failed to create device!");

    // Get device info.
    const gfx::DeviceInfo& gfx_device_info = m_gfx_device->getDeviceInfo();
    m_info.type = m_desc.type;
    m_info.api_name = gfx_device_info.apiName;
    m_info.adapter_name = gfx_device_info.adapterName;
    m_info.adapter_luid = m_desc.adapter_luid ? *m_desc.adapter_luid : AdapterLUID();
    m_info.timestamp_frequency = gfx_device_info.timestampFrequency;
    m_info.limits.max_texture_dimension_1d = gfx_device_info.limits.maxTextureDimension1D;
    m_info.limits.max_texture_dimension_2d = gfx_device_info.limits.maxTextureDimension2D;
    m_info.limits.max_texture_dimension_3d = gfx_device_info.limits.maxTextureDimension3D;
    m_info.limits.max_texture_dimension_cube = gfx_device_info.limits.maxTextureDimensionCube;
    m_info.limits.max_texture_array_layers = gfx_device_info.limits.maxTextureArrayLayers;
    m_info.limits.max_vertex_input_elements = gfx_device_info.limits.maxVertexInputElements;
    m_info.limits.max_vertex_input_element_offset = gfx_device_info.limits.maxVertexInputElementOffset;
    m_info.limits.max_vertex_streams = gfx_device_info.limits.maxVertexStreams;
    m_info.limits.max_vertex_stream_stride = gfx_device_info.limits.maxVertexStreamStride;
    m_info.limits.max_compute_threads_per_group = gfx_device_info.limits.maxComputeThreadsPerGroup;
    m_info.limits.max_compute_thread_group_size = uint3(
        gfx_device_info.limits.maxComputeThreadGroupSize[0],
        gfx_device_info.limits.maxComputeThreadGroupSize[1],
        gfx_device_info.limits.maxComputeThreadGroupSize[2]
    );
    m_info.limits.max_compute_dispatch_thread_groups = uint3(
        gfx_device_info.limits.maxComputeDispatchThreadGroups[0],
        gfx_device_info.limits.maxComputeDispatchThreadGroups[1],
        gfx_device_info.limits.maxComputeDispatchThreadGroups[2]
    );
    m_info.limits.max_viewports = gfx_device_info.limits.maxViewports;
    m_info.limits.max_viewport_dimensions
        = uint2(gfx_device_info.limits.maxViewportDimensions[0], gfx_device_info.limits.maxViewportDimensions[1]);
    m_info.limits.max_framebuffer_dimensions = uint3(
        gfx_device_info.limits.maxFramebufferDimensions[0],
        gfx_device_info.limits.maxFramebufferDimensions[1],
        gfx_device_info.limits.maxFramebufferDimensions[2]
    );
    m_info.limits.max_shader_visible_samplers = gfx_device_info.limits.maxShaderVisibleSamplers;

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
        if (m_gfx_device->hasFeature(sm_str)) {
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
    gfx::GfxCount feature_count = 0;
    SLANG_CALL(m_gfx_device->getFeatures(features, std::size(features), &feature_count));
    for (gfx::GfxCount i = 0; i < feature_count; ++i)
        m_features.push_back(features[i]);
    log_debug("Supported features: {}", string::join(m_features, ", "));

    // Create graphics queue.
    SLANG_CALL(m_gfx_device->createCommandQueue(
        {.type = gfx::ICommandQueue::QueueType::Graphics},
        m_gfx_graphics_queue.writeRef()
    ));

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
         .usage = ResourceUsage::none,
         .page_size = 1024 * 1024 * 4,
         .debug_name = "default_upload_heap"}
    );

    m_read_back_heap = create_memory_heap(
        {.memory_type = MemoryType::read_back,
         .usage = ResourceUsage::none,
         .page_size = 1024 * 1024 * 4,
         .debug_name = "default_read_back_heap"}
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

    m_gfx_graphics_queue.setNull();
    m_gfx_device.setNull();

#if SGL_HAS_NVAPI
    m_api_dispatcher.reset();
#endif
}

ShaderCacheStats Device::shader_cache_stats() const
{
    Slang::ComPtr<gfx::IShaderCache> gfx_shader_cache;
    SLANG_CALL(m_gfx_device->queryInterface(SLANG_UUID_IShaderCache, (void**)gfx_shader_cache.writeRef()));
    if (!gfx_shader_cache)
        return {
            .entry_count = 0,
            .hit_count = 0,
            .miss_count = 0,
        };

    gfx::ShaderCacheStats gfx_stats;
    SLANG_CALL(gfx_shader_cache->getShaderCacheStats(&gfx_stats));
    return {
        .entry_count = static_cast<size_t>(gfx_stats.entryCount),
        .hit_count = static_cast<size_t>(gfx_stats.hitCount),
        .miss_count = static_cast<size_t>(gfx_stats.missCount),
    };
}

ResourceStateSet Device::get_format_supported_resource_states(Format format) const
{
    gfx::ResourceStateSet gfx_state_set;
    SLANG_CALL(m_gfx_device->getFormatSupportedResourceStates(static_cast<gfx::Format>(format), &gfx_state_set));
    ResourceStateSet state_set;
    for (uint32_t i = 0; i < uint32_t(gfx::ResourceState::_Count); ++i)
        if (gfx_state_set.contains(static_cast<gfx::ResourceState>(i)))
            state_set.insert(static_cast<ResourceState>(i));
    return state_set;
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

    m_current_transient_resource_heap.setNull();
    m_in_flight_transient_resource_heaps = {};
    m_transient_resource_heap_pool = {};
    m_deferred_release_queue = {};

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

ref<Swapchain> Device::create_swapchain(SwapchainDesc desc, Window* window)
{
    return make_ref<Swapchain>(std::move(desc), window, ref<Device>(this));
}

ref<Swapchain> Device::create_swapchain(SwapchainDesc desc, WindowHandle window_handle)
{
    return make_ref<Swapchain>(std::move(desc), window_handle, ref<Device>(this));
}

ref<Buffer> Device::create_buffer(BufferDesc desc)
{
    return make_ref<Buffer>(ref<Device>(this), std::move(desc));
}

ref<Texture> Device::create_texture(TextureDesc desc)
{
    return make_ref<Texture>(ref<Device>(this), std::move(desc));
}

ref<Texture>
Device::create_texture_from_resource(TextureDesc desc, gfx::ITextureResource* resource, bool deferred_release)
{
    return make_ref<Texture>(ref<Device>(this), std::move(desc), resource, deferred_release);
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

ref<Framebuffer> Device::create_framebuffer(FramebufferDesc desc)
{
    return make_ref<Framebuffer>(ref<Device>(this), std::move(desc));
}

AccelerationStructurePrebuildInfo
Device::get_acceleration_structure_prebuild_info(const AccelerationStructureBuildInputs& build_inputs)
{
    const gfx::IAccelerationStructure::BuildInputs& gfx_build_inputs
        = reinterpret_cast<const gfx::IAccelerationStructure::BuildInputs&>(build_inputs);
    gfx::IAccelerationStructure::PrebuildInfo gfx_prebuild_info;
    SLANG_CALL(m_gfx_device->getAccelerationStructurePrebuildInfo(gfx_build_inputs, &gfx_prebuild_info));
    return AccelerationStructurePrebuildInfo{
        .result_data_max_size = gfx_prebuild_info.resultDataMaxSize,
        .scratch_data_size = gfx_prebuild_info.scratchDataSize,
        .update_scratch_data_size = gfx_prebuild_info.updateScratchDataSize,
    };
}

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

ref<MutableShaderObject> Device::create_mutable_shader_object(const ShaderProgram* shader_program)
{
    ref<MutableShaderObject> shader_object = make_ref<MutableShaderObject>(ref<Device>(this), shader_program);

    // Bind the debug printer to the new shader object, if enabled.
    if (m_debug_printer)
        m_debug_printer->bind(shader_object.get());

    return shader_object;
}

ref<MutableShaderObject> Device::create_mutable_shader_object(const TypeLayoutReflection* type_layout)
{
    return make_ref<MutableShaderObject>(ref<Device>(this), type_layout);
}

ref<MutableShaderObject> Device::create_mutable_shader_object(ReflectionCursor cursor)
{
    SGL_CHECK(cursor.is_valid(), "Invalid reflection cursor");
    return create_mutable_shader_object(cursor.type_layout().get());
}

ref<ComputePipeline> Device::create_compute_pipeline(ComputePipelineDesc desc)
{
    return make_ref<ComputePipeline>(ref<Device>(this), std::move(desc));
}

ref<GraphicsPipeline> Device::create_graphics_pipeline(GraphicsPipelineDesc desc)
{
    return make_ref<GraphicsPipeline>(ref<Device>(this), std::move(desc));
}

ref<RayTracingPipeline> Device::create_ray_tracing_pipeline(RayTracingPipelineDesc desc)
{
    return make_ref<RayTracingPipeline>(ref<Device>(this), std::move(desc));
}

ref<ComputeKernel> Device::create_compute_kernel(ComputeKernelDesc desc)
{
    return make_ref<ComputeKernel>(ref(this), std::move(desc));
}

ref<CommandBuffer> Device::create_command_buffer()
{
    SGL_ASSERT(m_shared_command_buffer == nullptr);

    return make_ref<CommandBuffer>(ref<Device>(this));
}

void Device::_set_open_command_buffer(CommandBuffer* command_buffer)
{
    SGL_CHECK(
        m_open_command_buffer == nullptr || command_buffer == nullptr,
        "Only one command buffer can be open at any time."
    );

    m_open_command_buffer = command_buffer;
}

Slang::ComPtr<gfx::ITransientResourceHeap> Device::_get_or_create_transient_resource_heap()
{
    if (m_current_transient_resource_heap)
        return m_current_transient_resource_heap;

    if (!m_transient_resource_heap_pool.empty()) {
        m_current_transient_resource_heap = m_transient_resource_heap_pool.front();
        m_transient_resource_heap_pool.pop();
        return m_current_transient_resource_heap;
    }

    SLANG_CALL(m_gfx_device->createTransientResourceHeap(
        gfx::ITransientResourceHeap::Desc{
            .flags = gfx::ITransientResourceHeap::Flags::AllowResizing,
            .constantBufferSize = 1024 * 1024 * 4,
            .samplerDescriptorCount = 1024,
            .uavDescriptorCount = 1024,
            .srvDescriptorCount = 1024,
            .constantBufferDescriptorCount = 1024,
            .accelerationStructureDescriptorCount = 1024,
        },
        m_current_transient_resource_heap.writeRef()
    ));
    return m_current_transient_resource_heap;
}

CommandBuffer* Device::_begin_shared_command_buffer()
{
    CommandBuffer* command_buffer = m_open_command_buffer;
    if (!command_buffer) {
        m_shared_command_buffer = create_command_buffer();
        command_buffer = m_shared_command_buffer;
    }

    return command_buffer;
}

void Device::_end_shared_command_buffer(bool wait)
{
    SGL_ASSERT(m_open_command_buffer);

    uint64_t id = 0;
    if (m_shared_command_buffer) {
        m_shared_command_buffer->close();
        id = submit_command_buffer(m_shared_command_buffer);
        m_shared_command_buffer.reset();
    } else {
        if (wait) {
            CommandBuffer* command_buffer = m_open_command_buffer;
            command_buffer->close();
            id = submit_command_buffer(command_buffer);
            command_buffer->open();
        }
    }
    if (wait)
        wait_command_buffer(id);
}

uint64_t Device::submit_command_buffer(CommandBuffer* command_buffer, CommandQueueType queue)
{
    SGL_CHECK_NOT_NULL(command_buffer);
    SGL_CHECK(queue == CommandQueueType::graphics, "Only graphics queue is supported.");

    if (command_buffer->is_open())
        SGL_THROW("Cannot submit open command buffer.");

    // TODO make parameter
    void* cuda_stream = 0;

    if (m_supports_cuda_interop && command_buffer->m_cuda_interop_buffers.size() > 0) {
        for (const auto& buffer : command_buffer->m_cuda_interop_buffers)
            buffer->copy_from_cuda(cuda_stream);

        sync_to_cuda(cuda_stream);
    }

    uint64_t fence_value = m_global_fence->update_signaled_value();
    m_gfx_graphics_queue
        ->executeCommandBuffer(command_buffer->gfx_command_buffer(), m_global_fence->gfx_fence(), fence_value);

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
    m_gfx_graphics_queue->waitOnHost();
}

void Device::sync_to_cuda(void* cuda_stream)
{
    // Signal fence from CUDA, wait for it on graphics queue.
    SGL_CU_SCOPE(this);
    uint64_t signal_value = m_global_fence->update_signaled_value();
    m_cuda_semaphore->signal(signal_value, CUstream(cuda_stream));
    gfx::IFence* fence = m_global_fence->gfx_fence();
    m_gfx_graphics_queue->waitForFenceValuesOnDevice(1, &fence, &signal_value);
}

void Device::sync_to_device(void* cuda_stream)
{
    SGL_CU_SCOPE(this);
    m_cuda_semaphore->wait(m_global_fence->signaled_value(), CUstream(cuda_stream));
}

void Device::run_garbage_collection()
{
    uint64_t signaled_value = m_global_fence->signaled_value();

    // Finish current transient resource heap and push it to the in-flight queue.
    if (m_current_transient_resource_heap) {
        m_current_transient_resource_heap->finish();
        m_in_flight_transient_resource_heaps.push({m_current_transient_resource_heap, signaled_value});
        m_current_transient_resource_heap.setNull();
    }

    // Execute deferred releases on the upload and read-back heaps.
    m_upload_heap->execute_deferred_releases();
    m_read_back_heap->execute_deferred_releases();

    uint64_t current_value = m_global_fence->current_value();

    // Reset transient resource heaps that are no longer in use.
    while (m_in_flight_transient_resource_heaps.size()
           && m_in_flight_transient_resource_heaps.front().second <= current_value) {
        Slang::ComPtr<gfx::ITransientResourceHeap> transient_resource_heap
            = m_in_flight_transient_resource_heaps.front().first;
        m_in_flight_transient_resource_heaps.pop();
        transient_resource_heap->synchronizeAndReset();
        m_transient_resource_heap_pool.push(transient_resource_heap);
    }

    // Release deferred objects that are no longer in use.
    while (m_deferred_release_queue.size() && m_deferred_release_queue.front().fence_value <= current_value)
        m_deferred_release_queue.pop();

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

    // If command buffer is already open, only add the copy command, don't attempt
    // to immediately submit.
    if (m_shared_command_buffer) {
        m_shared_command_buffer->copy_buffer_region(buffer, offset, alloc->buffer, alloc->offset, size);
    } else {
        CommandBuffer* command_buffer = _begin_shared_command_buffer();
        command_buffer->copy_buffer_region(buffer, offset, alloc->buffer, alloc->offset, size);
        _end_shared_command_buffer(false);
    }
}

void Device::read_buffer_data(const Buffer* buffer, void* data, size_t size, size_t offset)
{
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK(offset + size <= buffer->size(), "Buffer read is out of bounds");
    SGL_CHECK_NOT_NULL(data);

    auto alloc = m_read_back_heap->allocate(size, TEXTURE_UPLOAD_ALIGNMENT);

    CommandBuffer* command_buffer = _begin_shared_command_buffer();
    command_buffer->copy_buffer_region(alloc->buffer, alloc->offset, buffer, offset, size);
    _end_shared_command_buffer(true);

    std::memcpy(data, alloc->data, size);
}

void Device::upload_texture_data(Texture* texture, uint32_t subresource, SubresourceData subresource_data)
{
    SGL_CHECK_NOT_NULL(texture);
    SGL_CHECK_LT(subresource, texture->subresource_count());

    CommandBuffer* command_buffer = _begin_shared_command_buffer();
    command_buffer->upload_texture_data(texture, subresource, subresource_data);
    _end_shared_command_buffer(false);
}

OwnedSubresourceData Device::read_texture_data(const Texture* texture, uint32_t subresource)
{
    SGL_CHECK_NOT_NULL(texture);
    SGL_CHECK_LT(subresource, texture->subresource_count());

    SubresourceLayout layout = texture->get_subresource_layout(subresource);

    size_t size = layout.total_size_aligned();
    auto alloc = m_read_back_heap->allocate(size, TEXTURE_UPLOAD_ALIGNMENT);

    CommandBuffer* command_buffer = _begin_shared_command_buffer();
    command_buffer->copy_texture_to_buffer(
        alloc->buffer,
        alloc->offset,
        alloc->size,
        layout.row_pitch_aligned,
        texture,
        subresource
    );
    _end_shared_command_buffer(true);

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

void Device::deferred_release(ISlangUnknown* object)
{
    // Skip deferred release when device is already closed (or in the process of being closed).
    if (m_closed)
        return;

    m_deferred_release_queue.push({
        .fence_value = m_global_fence ? m_global_fence->signaled_value() : 0,
        .object = Slang::ComPtr<ISlangUnknown>(object),
    });
}

NativeHandle Device::get_native_handle(uint32_t index) const
{
    gfx::IDevice::InteropHandles handles = {};
    SLANG_CALL(m_gfx_device->getNativeDeviceHandles(&handles));

#if SGL_HAS_D3D12
    if (type() == DeviceType::d3d12) {
        SGL_ASSERT(index == 0);
        if (index == 0)
            return NativeHandle(reinterpret_cast<ID3D12Device*>(handles.handles[0].handleValue));
    }
#endif
#if SGL_HAS_VULKAN
    if (type() == DeviceType::vulkan) {
        SGL_ASSERT(index < 3);
        if (index == 0)
            return NativeHandle(reinterpret_cast<VkInstance>(handles.handles[0].handleValue));
        else if (index == 1)
            return NativeHandle(reinterpret_cast<VkPhysicalDevice>(handles.handles[1].handleValue));
        else if (index == 2)
            return NativeHandle(reinterpret_cast<VkDevice>(handles.handles[2].handleValue));
    }
#endif
    return {};
}

NativeHandle Device::get_native_command_queue_handle(CommandQueueType queue) const
{
    SGL_CHECK(queue == CommandQueueType::graphics, "Only graphics queue is supported.");

    gfx::InteropHandle handle = {};
    SLANG_CALL(m_gfx_graphics_queue->getNativeHandle(&handle));
#if SGL_HAS_D3D12
    if (type() == DeviceType::d3d12)
        return NativeHandle(reinterpret_cast<ID3D12CommandQueue*>(handle.handleValue));
#endif
#if SGL_HAS_VULKAN
    if (type() == DeviceType::vulkan)
        return NativeHandle(reinterpret_cast<VkQueue>(handle.handleValue));
#endif
    return {};
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

    auto convert_luid = [](const gfx::AdapterLUID& gfx_luid) -> AdapterLUID
    {
        AdapterLUID luid;
        for (size_t i = 0; i < 16; ++i)
            luid[i] = gfx_luid.luid[i];
        return luid;
    };

    gfx::AdapterList gfx_adapters = gfx::gfxGetAdapters(gfx_device_type(type));

    std::vector<AdapterInfo> adapters(gfx_adapters.getCount());
    for (size_t i = 0; i < adapters.size(); ++i) {
        const auto& gfx_adapter = gfx_adapters.getAdapters()[i];
        adapters[i] = AdapterInfo{
            .name = gfx_adapter.name,
            .vendor_id = gfx_adapter.vendorID,
            .device_id = gfx_adapter.deviceID,
            .luid = convert_luid(gfx_adapter.luid),
        };
    }

    return adapters;
}

void Device::report_live_objects()
{
    gfx::gfxReportLiveObjects();
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
