#include "device.h"

#include "kali/device/swapchain.h"
#include "kali/device/resource.h"
#include "kali/device/sampler.h"
#include "kali/device/fence.h"
#include "kali/device/query.h"
#include "kali/device/input_layout.h"
#include "kali/device/shader.h"
#include "kali/device/pipeline.h"
#include "kali/device/memory_heap.h"
#include "kali/device/command.h"
#include "kali/device/helpers.h"
#include "kali/device/native_handle_traits.h"
#include "kali/device/agility_sdk.h"

#include "kali/core/config.h"
#include "kali/core/error.h"
#include "kali/core/window.h"
#include "kali/core/string.h"

#if KALI_HAS_D3D12
#include <dxgi.h>
#include <d3d12.h>
#include <comdef.h>
#endif

namespace kali {

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

private:
    ref<Logger> m_logger;
};

static DebugLogger& get_debug_logger()
{
    static DebugLogger debug_logger;
    return debug_logger;
}

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
    KALI_UNREACHABLE();
}

Device::Device(const DeviceDesc& desc)
    : m_desc(desc)
{
    inc_ref();

    SLANG_CALL(slang::createGlobalSession(m_global_session.writeRef()));

    gfx::gfxSetDebugCallback(&get_debug_logger());
    if (m_desc.enable_debug_layers) {
        gfx::gfxEnableDebugLayer();
    }

    if (m_desc.type == DeviceType::automatic) {
#if KALI_WINDOWS
        m_desc.type = DeviceType::d3d12;
#elif KALI_LINUX
        m_desc.type = DeviceType::vulkan;
#endif
    }

    gfx::IDevice::Desc gfx_desc{
        .deviceType = gfx_device_type(m_desc.type),
        .slang{
            .slangGlobalSession = m_global_session,
        },
    };
    if (SLANG_FAILED(gfx::gfxCreateDevice(&gfx_desc, m_gfx_device.writeRef())))
        KALI_THROW("Failed to create device!");

    // Get device info.
    const gfx::DeviceInfo& gfx_device_info = m_gfx_device->getDeviceInfo();
    m_info.type = m_desc.type;
    m_info.api_name = gfx_device_info.apiName;
    m_info.adapter_name = gfx_device_info.adapterName;
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

    // Set default shader model.
    m_default_shader_model = m_desc.default_shader_model;
    if (m_default_shader_model > m_supported_shader_model) {
        log_warn(
            "Shader model {} is not supported, falling back to {}.",
            m_default_shader_model,
            m_supported_shader_model
        );
        m_default_shader_model = m_supported_shader_model;
    }

    // Get features.
    const char* features[256];
    gfx::GfxCount feature_count = 0;
    SLANG_CALL(m_gfx_device->getFeatures(features, std::size(features), &feature_count));
    for (gfx::GfxCount i = 0; i < feature_count; ++i)
        m_features.push_back(features[i]);

    // Create per-frame data.
    // TODO hardcoded frame count, maybe add to device desc?
    m_frame_data.resize(3);
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

    m_frame_fence = create_fence({.shared = true});
    m_frame_fence->break_strong_reference_to_device();

    m_default_queue = create_command_queue({.type = CommandQueueType::graphics});
    m_default_queue->break_strong_reference_to_device();

    m_command_stream = create_command_stream();
    m_command_stream->break_strong_reference_to_device();

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

    dec_ref(false);
}

Device::~Device()
{
    m_read_back_heap.reset();
    m_upload_heap.reset();

    m_command_stream.reset();
    m_default_queue.reset();
    m_frame_fence.reset();

    m_gfx_device.setNull();
}

ref<Swapchain> Device::create_swapchain(SwapchainDesc desc, ref<Window> window)
{
    return make_ref<Swapchain>(std::move(desc), std::move(window), m_default_queue, ref<Device>(this));
}

ref<Buffer> Device::create_buffer(BufferDesc desc, const void* init_data)
{
    return make_ref<Buffer>(ref<Device>(this), std::move(desc), init_data);
}

ref<Buffer> Device::create_raw_buffer(size_t size, ResourceUsage usage, MemoryType memory_type, const void* init_data)
{
    BufferDesc desc{
        .size = size,
        .usage = usage,
        .memory_type = memory_type,
    };
    return create_buffer(desc, init_data);
}

ref<Buffer> Device::create_typed_buffer(
    Format format,
    size_t element_count,
    ResourceUsage usage,
    MemoryType memory_type,
    const void* init_data
)
{
    BufferDesc desc{
        .size = element_count * 1,
        .format = format,
        .usage = usage,
        .memory_type = memory_type,
    };
    return create_buffer(desc, init_data);
}

ref<Buffer> Device::create_structured_buffer(
    size_t struct_size,
    size_t element_count,
    ResourceUsage usage,
    MemoryType memory_type,
    const void* init_data
)
{
    BufferDesc desc{
        .size = element_count * struct_size,
        .struct_size = struct_size,
        .usage = usage,
        .memory_type = memory_type,
    };
    return create_buffer(desc, init_data);
}

ref<Texture> Device::create_texture(TextureDesc desc, const void* init_data)
{
    return make_ref<Texture>(ref<Device>(this), std::move(desc), init_data);
}

ref<Texture> Device::create_texture_from_resource(TextureDesc desc, gfx::ITextureResource* resource)
{
    return make_ref<Texture>(ref<Device>(this), std::move(desc), resource);
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

ref<AccelerationStructure> Device::create_acceleration_structure(AccelerationStructure::Desc desc)
{
    return make_ref<AccelerationStructure>(ref<Device>(this), std::move(desc));
}

ref<SlangSession> Device::create_slang_session(SlangSessionDesc desc)
{
    return make_ref<SlangSession>(ref<Device>(this), std::move(desc));
}

ref<SlangModule> Device::load_module(const std::filesystem::path& path)
{
    ref<SlangSession> session = create_slang_session({});
    return session->load_module(path);
}

ref<SlangModule> Device::load_module_from_source(const std::string& source)
{
    ref<SlangSession> session = create_slang_session({});
    return session->load_module_from_source(source);
}

ref<ComputePipelineState> Device::create_compute_pipeline_state(ComputePipelineStateDesc desc)
{
    return make_ref<ComputePipelineState>(ref<Device>(this), std::move(desc));
}

ref<GraphicsPipelineState> Device::create_graphics_pipeline_state(GraphicsPipelineStateDesc desc)
{
    return make_ref<GraphicsPipelineState>(ref<Device>(this), std::move(desc));
}

ref<RayTracingPipelineState> Device::create_ray_tracing_pipeline_state(RayTracingPipelineStateDesc desc)
{
    return make_ref<RayTracingPipelineState>(ref<Device>(this), std::move(desc));
}

ref<CommandQueue> Device::create_command_queue(CommandQueueDesc desc)
{
    return make_ref<CommandQueue>(ref<Device>(this), std::move(desc));
}

ref<CommandBuffer> Device::create_command_buffer()
{
    // TODO increment frame
    return make_ref<CommandBuffer>(ref<Device>(this), m_frame_data[0].transient_resource_heap->createCommandBuffer());
}

ref<CommandStream> Device::create_command_stream()
{
    return make_ref<CommandStream>(ref<Device>(this), m_default_queue);
}

ref<MemoryHeap> Device::create_memory_heap(MemoryHeapDesc desc)
{
    return make_ref<MemoryHeap>(ref<Device>(this), m_frame_fence, std::move(desc));
}

void Device::wait()
{
    m_command_stream->submit();
    // TODO maybe we should keep track of all queues and wait for all of them?
    m_default_queue->wait();
}

void Device::read_buffer(const Buffer* buffer, size_t offset, size_t size, void* out_data)
{
    m_command_stream->buffer_barrier(buffer, ResourceState::copy_source);
    m_command_stream->submit();

    Slang::ComPtr<ISlangBlob> blob;
    if (offset + size > buffer->size())
        KALI_THROW("Buffer read out of bounds");
    SLANG_CALL(m_gfx_device->readBufferResource(buffer->gfx_buffer_resource(), offset, size, blob.writeRef()));
    std::memcpy(out_data, blob->getBufferPointer(), size);
}

void Device::read_texture(
    const Texture* texture,
    size_t size,
    void* out_data,
    size_t* out_row_pitch,
    size_t* out_pixel_size
)
{
    m_command_stream->texture_barrier(texture, ResourceState::copy_source);
    m_command_stream->submit();

    Slang::ComPtr<ISlangBlob> blob;
    SLANG_CALL(m_gfx_device->readTextureResource(
        texture->gfx_texture_resource(),
        gfx::ResourceState::CopySource,
        blob.writeRef(),
        out_row_pitch,
        out_pixel_size
    ));
    if (size > blob->getBufferSize())
        KALI_THROW("Texture read out of bounds");
    std::memcpy(out_data, blob->getBufferPointer(), size);
}

NativeHandle Device::get_native_handle(uint32_t index) const
{
    gfx::IDevice::InteropHandles handles = {};
    SLANG_CALL(m_gfx_device->getNativeDeviceHandles(&handles));

#if KALI_HAS_D3D12
    KALI_ASSERT(index == 0);
    if (type() == DeviceType::d3d12) {
        if (index == 0)
            return NativeHandle(reinterpret_cast<ID3D12Device*>(handles.handles[0].handleValue));
    }
#endif
#if KALI_HAS_VULKAN
    KALI_ASSERT(index < 3);
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
#if KALI_WINDOWS
        type = DeviceType::d3d12;
#elif KALI_LINUX
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
#if KALI_HAS_D3D12 && KALI_HAS_AGILITY_SDK
    std::filesystem::path exe_dir = platform::executable_directory();
    std::filesystem::path sdk_dir = platform::runtime_directory() / KALI_AGILITY_SDK_PATH;

    // Agility SDK can only be loaded from a relative path to the executable. Make sure both paths use the same driver
    // letter.
    if (std::tolower(exe_dir.string()[0]) != std::tolower(sdk_dir.string()[0])) {
        log_warn(
            "Cannot enable D3D12 Agility SDK: "
            "Executable directory '{}' is not on the same drive as the SDK directory '{}'.",
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
    if (!SUCCEEDED(pD3D12SDKConfiguration->SetSDKVersion(KALI_AGILITY_SDK_VERSION, rel_path.string().c_str()))) {
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
        "  type={},\n"
        "  enable_debug_layers={},\n"
        "  adapter_luid={},\n"
        "  shader_cache_path=\"{}\"\n"
        "  default_shader_model={},\n"
        "  supported_shader_model={}\n"
        ")",
        m_desc.type,
        m_desc.enable_debug_layers,
        m_desc.adapter_luid ? fmt::format("{}", *m_desc.adapter_luid) : "null",
        m_desc.shader_cache_path,
        m_default_shader_model,
        m_supported_shader_model
    );
}

} // namespace kali
