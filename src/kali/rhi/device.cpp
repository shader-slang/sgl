#include "device.h"
#include "swapchain.h"

#include "core/error.h"
#include "core/window.h"

namespace kali {

Device::Device(const DeviceDesc& desc)
{
    gfx::IDevice::Desc gfx_desc{
        .deviceType = gfx::DeviceType::DirectX12,
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

} // namespace kali
