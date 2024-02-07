#include "device.h"

#include "kali/device/swapchain.h"
#include "kali/device/resource.h"
#include "kali/device/sampler.h"
#include "kali/device/fence.h"
#include "kali/device/query.h"
#include "kali/device/input_layout.h"
#include "kali/device/framebuffer.h"
#include "kali/device/shader.h"
#include "kali/device/shader_object.h"
#include "kali/device/pipeline.h"
#include "kali/device/raytracing.h"
#include "kali/device/memory_heap.h"
#include "kali/device/command.h"
#include "kali/device/helpers.h"
#include "kali/device/native_handle_traits.h"
#include "kali/device/agility_sdk.h"
#include "kali/device/cuda_utils.h"
#include "kali/device/print.h"

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

    // Setup path for slang's downstream compilers.
    for (SlangPassThrough pass_through : {SLANG_PASS_THROUGH_DXC, SLANG_PASS_THROUGH_GLSLANG}) {
        m_global_session->setDownstreamCompilerPath(pass_through, platform::runtime_directory().string().c_str());
    }

    gfx::gfxSetDebugCallback(&get_debug_logger());
    if (m_desc.enable_debug_layers) {
        gfx::gfxEnableDebugLayer();
    }

    if (m_desc.type == DeviceType::automatic) {
#if KALI_WINDOWS
        m_desc.type = DeviceType::d3d12;
#elif KALI_LINUX || KALI_MACOS
        m_desc.type = DeviceType::vulkan;
#endif
    }

    gfx::IDevice::Desc gfx_desc{
        .deviceType = gfx_device_type(m_desc.type),
        .adapterLUID
        = m_desc.adapter_luid ? reinterpret_cast<const gfx::AdapterLUID*>(m_desc.adapter_luid->data()) : nullptr,
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
#if KALI_HAS_CUDA
    if (m_desc.enable_cuda_interop) {
        KALI_CHECK(m_desc.type == DeviceType::d3d12, "CUDA interop is only supported on D3D12 devices");
        m_cuda_device = make_ref<cuda::Device>(this);
        m_supports_cuda_interop = true;
    }
#else
    if (m_desc.enable_cuda_interop) {
        KALI_THROW("CUDA interop is not supported");
    }
#endif

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
    m_frame_fence.reset();

    m_deferred_release_queue = {};

    m_gfx_device.setNull();
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

ref<SlangModule> Device::load_module(
    const std::filesystem::path& path,
    const DefineList& defines,
    const SlangCompilerOptions& compiler_options
)
{
    ref<SlangSession> session = create_slang_session({.compiler_options = compiler_options});
    return session->load_module(path, defines);
}

ref<SlangModule> Device::load_module_from_source(
    const std::string& source,
    const DefineList& defines,
    const SlangCompilerOptions& compiler_options
)
{
    ref<SlangSession> session = create_slang_session({.compiler_options = compiler_options});
    return session->load_module_from_source(source, {}, {}, defines);
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
    KALI_CHECK(cursor.is_valid(), "Invalid reflection cursor");
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

    flush_print();
}

void Device::wait()
{
    m_graphics_queue->wait();
}

void Device::read_buffer(const Buffer* buffer, size_t offset, size_t size, void* out_data)
{
    // TODO move this to CommandBuffer
    ref<CommandBuffer> command_buffer = create_command_buffer();
    command_buffer->buffer_barrier(buffer, ResourceState::copy_source);
    command_buffer->submit();

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
        KALI_THROW("Texture read out of bounds");
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
        "  type = {},\n"
        "  enable_debug_layers = {},\n"
        "  adapter_luid = {},\n"
        "  shader_cache_path = \"{}\"\n"
        "  default_shader_model = {},\n"
        "  supported_shader_model = {}\n"
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
