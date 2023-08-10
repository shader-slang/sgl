#include "device.h"

#include "kali/rhi/swapchain.h"
#include "kali/rhi/resource.h"
#include "kali/rhi/sampler.h"
#include "kali/rhi/fence.h"
#include "kali/rhi/program.h"
#include "kali/rhi/pipeline.h"
#include "kali/rhi/command_queue.h"
#include "kali/rhi/command_stream.h"
#include "kali/rhi/helpers.h"
#include "kali/rhi/native_handle_traits.h"

#include "kali/core/error.h"
#include "kali/core/window.h"

namespace kali {

class DebugLogger : public gfx::IDebugCallback {
public:
    DebugLogger()
    {
        m_logger = Logger::create(LogLevel::debug, "DEVICE", false);
        m_logger->use_same_outputs(Logger::global());
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

inline gfx::DeviceType get_gfx_device_type(DeviceType device_type)
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
    SLANG_CALL(slang::createGlobalSession(m_slang_session.writeRef()));

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
        .deviceType = get_gfx_device_type(m_desc.type),
        .slang{
            .slangGlobalSession = m_slang_session,
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

    m_program_manager = make_ref<ProgramManager>(this, m_slang_session);

    m_command_stream = create_command_stream({.queue_type = CommandQueueType::graphics});
}

Device::~Device()
{
    m_gfx_device.setNull();
}

ref<Swapchain> Device::create_swapchain(SwapchainDesc desc, ref<Window> window)
{
    return make_ref<Swapchain>(std::move(desc), window, ref<Device>(this));
}

ref<Buffer> Device::create_buffer(BufferDesc desc, const void* init_data)
{
    return make_ref<Buffer>(std::move(desc), init_data, ref<Device>(this));
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
    return make_ref<Texture>(std::move(desc), init_data, ref<Device>(this));
}

ref<Sampler> Device::create_sampler(SamplerDesc desc)
{
    return make_ref<Sampler>(std::move(desc), ref<Device>(this));
}

ref<Program> Device::create_program(ProgramDesc desc)
{
    return m_program_manager->create_program(std::move(desc));
}

ref<Fence> Device::create_fence(FenceDesc desc)
{
    return make_ref<Fence>(std::move(desc), ref<Device>(this));
}

ref<Fence> Device::create_fence(uint64_t initial_value, bool shared)
{
    return create_fence({
        .initial_value = initial_value,
        .shared = shared,
    });
}

ref<ComputePipelineState> Device::create_compute_pipeline_state(ComputePipelineStateDesc desc)
{
    return make_ref<ComputePipelineState>(std::move(desc), ref<Device>(this));
}

ref<ComputePipelineCache> Device::create_compute_pipeline_cache()
{
    return make_ref<ComputePipelineCache>(ref<Device>(this));
}

ref<GraphicsPipelineState> Device::create_graphics_pipeline_state(GraphicsPipelineStateDesc desc)
{
    return make_ref<GraphicsPipelineState>(std::move(desc), ref<Device>(this));
}

ref<CommandQueue> Device::create_command_queue(CommandQueueDesc desc)
{
    return make_ref<CommandQueue>(std::move(desc), ref<Device>(this));
}

ref<CommandStream> Device::create_command_stream(CommandStreamDesc desc)
{
    return make_ref<CommandStream>(std::move(desc), ref<Device>(this));
}

void Device::read_buffer(const Buffer* buffer, size_t offset, size_t size, void* out_data)
{
    Slang::ComPtr<ISlangBlob> blob;
    if (offset + size > buffer->get_size())
        KALI_THROW("Buffer read out of bounds");
    SLANG_CALL(m_gfx_device->readBufferResource(buffer->get_gfx_buffer_resource(), offset, size, blob.writeRef()));
    std::memcpy(out_data, blob->getBufferPointer(), size);
}

NativeHandle Device::get_native_handle(uint32_t index) const
{
    gfx::IDevice::InteropHandles handles = {};
    SLANG_CALL(m_gfx_device->getNativeDeviceHandles(&handles));

#if KALI_HAS_D3D12
    KALI_ASSERT(index == 0);
    if (get_type() == DeviceType::d3d12) {
        if (index == 0)
            return NativeHandle(reinterpret_cast<ID3D12Device*>(handles.handles[0].handleValue));
    }
#endif
#if KALI_HAS_VULKAN
    KALI_ASSERT(index < 3);
    if (get_type() == DeviceType::vulkan) {
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

    gfx::AdapterList gfx_adapters = gfx::gfxGetAdapters(get_gfx_device_type(type));

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

} // namespace kali
