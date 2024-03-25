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
#include "sgl/device/print.h"

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

namespace sgl {

static constexpr uint32_t IN_FLIGHT_FRAME_COUNT = 3;

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

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) override
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
    inc_ref();

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

    // Create default slang session.
    m_slang_session = create_slang_session({
        .compiler_options = m_desc.compiler_options,
        .add_default_include_paths = true,
        .cache_path = m_shader_cache_enabled ? std::optional(m_shader_cache_path) : std::nullopt,
    });
    m_slang_session->break_strong_reference_to_device();

    // Create per-frame data.
    m_frame_data.resize(IN_FLIGHT_FRAME_COUNT);
    for (auto& frame_data : m_frame_data) {
        m_gfx_device->createTransientResourceHeap(
            gfx::ITransientResourceHeap::Desc{
                .flags = gfx::ITransientResourceHeap::Flags::AllowResizing,
                .constantBufferSize = 1024 * 1024 * 4,
                .samplerDescriptorCount = 1024,
                .uavDescriptorCount = 1024,
                .srvDescriptorCount = 1024,
                .constantBufferDescriptorCount = 1024,
                .accelerationStructureDescriptorCount = 1024,
            },
            frame_data.transient_resource_heap.writeRef()
        );
    }

    // Create CUDA device before creating the graphics queue, which internally
    // allocates a shared semaphore for synchronization.
    if (m_desc.enable_cuda_interop) {
        SGL_CHECK(sgl_cuda_api_init(), "Failed to initialize CUDA driver API.");
        SGL_CHECK(m_desc.type == DeviceType::d3d12, "CUDA interop is currently only supported on D3D12 devices");
        m_cuda_device = make_ref<cuda::Device>(this);
        m_supports_cuda_interop = true;
    }

    m_frame_fence = create_fence({});
    m_frame_fence->break_strong_reference_to_device();

    m_graphics_queue = make_ref<CommandQueue>(ref<Device>(this), CommandQueueDesc{.type = CommandQueueType::graphics});
    m_graphics_queue->break_strong_reference_to_device();

    m_upload_heap = create_memory_heap(
        {.memory_type = MemoryType::upload,
         .usage = ResourceUsage::none,
         .page_size = 1024 * 1024 * 4,
         .debug_name = "default_upload_heap"}
    );
    m_upload_heap->break_strong_reference_to_device();

    m_read_back_heap = create_memory_heap(
        {.memory_type = MemoryType::read_back,
         .usage = ResourceUsage::none,
         .page_size = 1024 * 1024 * 4,
         .debug_name = "default_read_back_heap"}
    );
    m_read_back_heap->break_strong_reference_to_device();

    if (m_desc.enable_print)
        m_debug_printer = std::make_unique<DebugPrinter>(this);

    dec_ref(false);
}

Device::~Device()
{
    wait();

    m_debug_printer.reset();

    m_read_back_heap.reset();
    m_upload_heap.reset();

    m_graphics_queue.reset();
    m_frame_data.clear();
    m_frame_fence.reset();

    m_deferred_release_queue = {};

    m_slang_session.reset();

    m_gfx_device.setNull();

#if SGL_HAS_NVAPI
    m_api_dispatcher.reset();
#endif
}

ShaderCacheStats Device::shader_cache_stats() const
{
    Slang::ComPtr<gfx::IShaderCache> gfx_shader_cache;
    SLANG_CALL(m_gfx_device->queryInterface(SLANG_UUID_IShaderCache, (void**)gfx_shader_cache.writeRef()));
    gfx::ShaderCacheStats gfx_stats;
    SLANG_CALL(gfx_shader_cache->getShaderCacheStats(&gfx_stats));
    return {
        .entry_count = static_cast<size_t>(gfx_stats.entryCount),
        .hit_count = static_cast<size_t>(gfx_stats.hitCount),
        .miss_count = static_cast<size_t>(gfx_stats.missCount),
    };
}

ref<Swapchain> Device::create_swapchain(SwapchainDesc desc, Window* window)
{
    return make_ref<Swapchain>(std::move(desc), window, m_graphics_queue, ref<Device>(this));
}

ref<Buffer> Device::create_buffer(BufferDesc desc)
{
    return make_ref<Buffer>(ref<Device>(this), std::move(desc));
}

ref<Buffer> Device::create_raw_buffer(RawBufferDesc desc)
{
    return make_ref<Buffer>(ref<Device>(this), std::move(desc));
}

ref<Buffer> Device::create_structured_buffer(StructuredBufferDesc desc)
{
    return make_ref<Buffer>(ref<Device>(this), std::move(desc));
}

ref<Buffer> Device::create_typed_buffer(TypedBufferDesc desc)
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

ref<ShaderTable> Device::create_shader_table(ShaderTableDesc desc)
{
    return make_ref<ShaderTable>(ref<Device>(this), std::move(desc));
}

ref<SlangSession> Device::create_slang_session(SlangSessionDesc desc)
{
    return make_ref<SlangSession>(ref<Device>(this), std::move(desc));
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
    return create_mutable_shader_object(cursor.type_layout());
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
    return make_ref<CommandBuffer>(
        ref<Device>(this),
        m_frame_data[m_current_frame_index].transient_resource_heap->createCommandBuffer()
    );
}

ref<MemoryHeap> Device::create_memory_heap(MemoryHeapDesc desc)
{
    return make_ref<MemoryHeap>(ref<Device>(this), m_frame_fence, std::move(desc));
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

void Device::end_frame()
{
    if (m_frame_fence->signaled_value() > IN_FLIGHT_FRAME_COUNT)
        m_frame_fence->wait(m_frame_fence->signaled_value() - IN_FLIGHT_FRAME_COUNT);

    m_frame_data[m_current_frame_index].transient_resource_heap->finish();
    m_current_frame_index = (m_current_frame_index + 1) % IN_FLIGHT_FRAME_COUNT;
    m_frame_data[m_current_frame_index].transient_resource_heap->synchronizeAndReset();

    m_graphics_queue->signal(m_frame_fence);

    execute_deferred_releases();

    flush_print();
}

void Device::wait()
{
    m_graphics_queue->wait();
}

void Device::upload_buffer_data(Buffer* buffer, const void* data, size_t size, size_t offset)
{
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK(offset + size <= buffer->size(), "Buffer write is out of bounds");
    SGL_CHECK_NOT_NULL(data);

    auto alloc = m_upload_heap->allocate(size);

    std::memcpy(alloc->data, data, size);

    // TODO we want to use existing command buffer.
    ref<CommandBuffer> command_buffer = create_command_buffer();
    command_buffer->buffer_barrier(buffer, ResourceState::copy_destination);
    command_buffer->copy_buffer_region(buffer, offset, alloc->buffer, alloc->offset, size);
    command_buffer->submit();
}

void Device::read_buffer_data(const Buffer* buffer, void* data, size_t size, size_t offset)
{
    SGL_CHECK_NOT_NULL(buffer);
    SGL_CHECK(offset + size <= buffer->size(), "Buffer read is out of bounds");
    SGL_CHECK_NOT_NULL(data);

    auto alloc = m_read_back_heap->allocate(size);

    // TODO we want to use existing command buffer.
    ref<CommandBuffer> command_buffer = create_command_buffer();
    command_buffer->buffer_barrier(buffer, ResourceState::copy_source);
    command_buffer->copy_buffer_region(alloc->buffer, alloc->offset, buffer, offset, size);
    command_buffer->submit();
    m_graphics_queue->wait();

    std::memcpy(data, alloc->data, size);
}

void Device::read_texture(
    const Texture* texture,
    size_t size,
    void* out_data,
    size_t* out_row_pitch,
    size_t* out_pixel_size
)
{
    // TODO move this to CommandBuffer
    ref<CommandBuffer> command_buffer = create_command_buffer();
    command_buffer->texture_barrier(texture, ResourceState::copy_source);
    command_buffer->submit();

    Slang::ComPtr<ISlangBlob> blob;
    SLANG_CALL(m_gfx_device->readTextureResource(
        texture->gfx_texture_resource(),
        gfx::ResourceState::CopySource,
        blob.writeRef(),
        out_row_pitch,
        out_pixel_size
    ));
    if (size > blob->getBufferSize())
        SGL_THROW("Texture read out of bounds");
    std::memcpy(out_data, blob->getBufferPointer(), size);
}

void Device::deferred_release(ISlangUnknown* object)
{
    m_deferred_release_queue.push({
        .fence_value = m_frame_fence ? m_frame_fence->signaled_value() : 0,
        .object = Slang::ComPtr<ISlangUnknown>(object),
    });
}

void Device::execute_deferred_releases()
{
    m_upload_heap->execute_deferred_releases();
    m_read_back_heap->execute_deferred_releases();

    uint64_t current_value = m_frame_fence->current_value();
    while (m_deferred_release_queue.size() && m_deferred_release_queue.front().fence_value < current_value)
        m_deferred_release_queue.pop();
}

NativeHandle Device::get_native_handle(uint32_t index) const
{
    gfx::IDevice::InteropHandles handles = {};
    SLANG_CALL(m_gfx_device->getNativeDeviceHandles(&handles));

#if SGL_HAS_D3D12
    SGL_ASSERT(index == 0);
    if (type() == DeviceType::d3d12) {
        if (index == 0)
            return NativeHandle(reinterpret_cast<ID3D12Device*>(handles.handles[0].handleValue));
    }
#endif
#if SGL_HAS_VULKAN
    SGL_ASSERT(index < 3);
    if (type() == DeviceType::vulkan) {
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

std::vector<AdapterInfo> Device::enumerate_adapters(DeviceType type)
{
    if (type == DeviceType::automatic) {
#if SGL_WINDOWS
        type = DeviceType::d3d12;
#elif SGL_LINUX
        type = DeviceType::vulkan;
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

    // Agility SDK can only be loaded from a relative path to the executable. Make sure both paths use the same driver
    // letter.
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
        "  shader_cache_path = \"{}\",\n"
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

} // namespace sgl