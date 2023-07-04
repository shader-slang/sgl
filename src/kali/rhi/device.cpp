#include "device.h"
#include "swapchain.h"
#include "resource.h"
#include "sampler.h"
#include "program.h"
#include "helpers.h"

#include "core/error.h"
#include "core/window.h"

namespace kali {

class DebugLogger : public gfx::IDebugCallback {
public:
    virtual SLANG_NO_THROW void SLANG_MCALL
    handleMessage(gfx::DebugMessageType type, gfx::DebugMessageSource source, const char* message)
    {
        const char* source_str = "";
        switch (source) {
        case gfx::DebugMessageSource::Layer:
            source_str = "Layer";
        case gfx::DebugMessageSource::Driver:
            source_str = "Driver";
        case gfx::DebugMessageSource::Slang:
            source_str = "Slang";
        }
        switch (type) {
        case gfx::DebugMessageType::Info:
            log_info("{}: {}", source_str, message);
            break;
        case gfx::DebugMessageType::Warning:
            log_warn("{}: {}", source_str, message);
            break;
        case gfx::DebugMessageType::Error:
            log_error("{}: {}", source_str, message);
            break;
        }
    }
};

static DebugLogger s_debug_logger;


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
    KALI_THROW(RuntimeError("Invalid DeviceType"));
}

Device::Device(const DeviceDesc& desc)
    : m_desc(desc)
{
    if (m_desc.enable_debug_layers) {
        gfx::gfxEnableDebugLayer();
        gfx::gfxSetDebugCallback(&s_debug_logger);
    }

    m_type = m_desc.type;
    if (m_type == DeviceType::automatic) {
#if KALI_WINDOWS
        m_type = DeviceType::d3d12;
#elif KALI_LINUX
        m_type = DeviceType::vulkan;
#endif
    }

    gfx::IDevice::Desc gfx_desc{
        .deviceType = get_gfx_device_type(m_type),
    };
    if (SLANG_FAILED(gfx::gfxCreateDevice(&gfx_desc, m_gfx_device.writeRef())))
        KALI_THROW(RuntimeError("Failed to create device!"));

    gfx::ICommandQueue::Desc queue_desc{
        .type = gfx::ICommandQueue::QueueType::Graphics,
    };

    SLANG_CALL(m_gfx_device->createCommandQueue(queue_desc, m_gfx_queue.writeRef()));
    SLANG_CALL(slang::createGlobalSession(m_slang_session.writeRef()));

    m_program_manager = make_ref<ProgramManager>(this, m_slang_session);
}

Device::~Device()
{
    m_gfx_queue.setNull();
    m_gfx_device.setNull();
}

ref<Swapchain> Device::create_swapchain(const SwapchainDesc& desc, ref<Window> window)
{
    return make_ref<Swapchain>(desc, window, ref<Device>(this));
}

ref<Buffer> Device::create_buffer(const BufferDesc& desc, const void* init_data)
{
    return make_ref<Buffer>(desc, init_data, ref<Device>(this));
}

ref<Buffer> Device::create_raw_buffer(size_t size, ResourceUsage usage, CpuAccess cpu_access, const void* init_data)
{
    BufferDesc desc{
        .size = size,
        .usage = usage,
        .cpu_access = cpu_access,
    };
    return create_buffer(desc, init_data);
}

ref<Buffer> Device::create_typed_buffer(
    Format format,
    size_t element_count,
    ResourceUsage usage,
    CpuAccess cpu_access,
    const void* init_data
)
{
    BufferDesc desc{
        .size = element_count * 1,
        .format = format,
        .usage = usage,
        .cpu_access = cpu_access,
    };
    return create_buffer(desc, init_data);
}

ref<Buffer> Device::create_structured_buffer(
    size_t struct_size,
    size_t element_count,
    ResourceUsage usage,
    CpuAccess cpu_access,
    const void* init_data
)
{
    BufferDesc desc{
        .size = element_count * struct_size,
        .struct_size = struct_size,
        .usage = usage,
        .cpu_access = cpu_access,
    };
    return create_buffer(desc, init_data);
}

ref<Texture> Device::create_texture(const TextureDesc& desc, const void* init_data)
{
    return make_ref<Texture>(desc, init_data, ref<Device>(this));
}

ref<Sampler> Device::create_sampler(const SamplerDesc& desc)
{
    return make_ref<Sampler>(desc, ref<Device>(this));
}

ref<Program> Device::create_program(const ProgramDesc& desc)
{
    return m_program_manager->create_program(desc);
}

ProgramManager& Device::get_program_manager()
{
    return *m_program_manager;
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
