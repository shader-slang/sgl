#include "device.h"
#include "swapchain.h"

#include "core/error.h"
#include "core/window.h"

namespace kali {

Device::Device()
{
    gfx::IDevice::Desc gfx_desc{};
    gfx_desc.deviceType = gfx::DeviceType::DirectX12;
    if (SLANG_FAILED(gfx::gfxCreateDevice(&gfx_desc, m_gfx_device.writeRef())))
        KALI_THROW("Failed to create device!");
}

Device::~Device()
{
    m_gfx_device.setNull();
}

ref<Swapchain> Device::create_swapchain(const SwapchainDesc& desc, ref<Window> window)
{
    return new Swapchain(desc, window, this);
}

} // namespace kali
