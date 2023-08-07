#include "swapchain.h"

#include "kali/rhi/device.h"
#include "kali/rhi/helpers.h"

#include "kali/core/error.h"
#include "kali/core/window.h"

namespace kali {

Swapchain::Swapchain(SwapchainDesc desc, WindowHandle window_handle, ref<Device> device)
    : m_desc(std::move(desc))
    , m_device(std::move(device))
{
    KALI_ASSERT(m_device);

    gfx::ISwapchain::Desc gfx_desc{
        .format = get_gfx_format(m_desc.format),
        .width = gfx::GfxCount(m_desc.width),
        .height = gfx::GfxCount(m_desc.height),
        .imageCount = gfx::GfxCount(m_desc.image_count),
        .queue = m_device->get_gfx_queue(),
        .enableVSync = m_desc.enable_vsync,
    };

#if KALI_WINDOWS
    gfx::WindowHandle gfx_window_handle = gfx::WindowHandle::FromHwnd(window_handle);
#elif KALI_LINUX
    gfx::WindowHandle gfx_window_handle = gfx::WindowHandle::FromXWindow(window_handle.xdisplay, window_handle.xwindow);
#endif

    SLANG_CALL(m_device->get_gfx_device()->createSwapchain(gfx_desc, gfx_window_handle, m_gfx_swapchain.writeRef()));
}

Swapchain::Swapchain(SwapchainDesc desc, ref<Window> window, ref<Device> device)
    : Swapchain(std::move(desc), window->get_window_handle(), std::move(device))
{
    m_window = window;
}

Swapchain::~Swapchain() { }

void Swapchain::resize(uint32_t width, uint32_t height)
{
    m_desc.width = width;
    m_desc.height = height;
    m_gfx_swapchain->resize(width, height);
}

} // namespace kali
