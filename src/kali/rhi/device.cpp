#include "device.h"
#include "swapchain.h"
#include "resource.h"

#include "core/error.h"
#include "core/window.h"

namespace kali {

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
    KALI_THROW(Exception("Invalid DeviceType"));
}

Device::Device(const DeviceDesc& desc)
{
    gfx::IDevice::Desc gfx_desc{
        .deviceType = get_gfx_device_type(desc.type),
    };
    if (SLANG_FAILED(gfx::gfxCreateDevice(&gfx_desc, m_gfx_device.writeRef())))
        KALI_THROW(Exception("Failed to create device!"));

    gfx::ICommandQueue::Desc queue_desc{
        .type = gfx::ICommandQueue::QueueType::Graphics,
    };
    if (SLANG_FAILED(m_gfx_device->createCommandQueue(queue_desc, m_gfx_queue.writeRef())))
        KALI_THROW(Exception("Failed to create graphics queue!"));
}

Device::~Device()
{
    m_gfx_queue.setNull();
    m_gfx_device.setNull();
}

ref<Swapchain> Device::create_swapchain(const SwapchainDesc& desc, ref<Window> window)
{
    return new Swapchain(desc, window, this);
}

ref<Buffer> Device::create_buffer(const BufferDesc& desc, void* init_data)
{
    return new Buffer(desc, init_data, this);
}


} // namespace kali
