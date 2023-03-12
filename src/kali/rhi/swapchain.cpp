#include "swapchain.h"
#include "device.h"

#include "core/error.h"
#include "core/window.h"

namespace kali {

Swapchain::Swapchain(const SwapchainDesc& desc, ref<Window> window, ref<Device> device)
    : m_desc(desc)
    , m_device(std::move(device))
    , m_window(window)
{
    gfx::ISwapchain::Desc gfx_desc{};
    gfx::WindowHandle gfx_window_handle{};

    if (SLANG_FAILED(
            m_device->get_gfx_device()->createSwapchain(gfx_desc, gfx_window_handle, m_gfx_swapchain.writeRef())))
        KALI_THROW("Failed to create swapchain!");
}

Swapchain::~Swapchain() { }

} // namespace kali
