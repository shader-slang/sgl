#include "swapchain.h"
#include "device.h"

#include "core/error.h"
#include "core/window.h"

namespace kali {

Swapchain::Swapchain(const SwapchainDesc& desc, WindowHandle window_handle, ref<Device> device)
    : m_desc(desc)
    , m_device(std::move(device))
{
    KALI_ASSERT(m_device);

    gfx::ISwapchain::Desc gfx_desc{
        .format = get_gfx_format(desc.format),
        .width = gfx::GfxCount(desc.width),
        .height = gfx::GfxCount(desc.height),
        .imageCount = gfx::GfxCount(desc.image_count),
        .queue = m_device->get_gfx_queue(),
        .enableVSync = desc.enable_vsync,
    };

#if KALI_WINDOWS
    gfx::WindowHandle gfx_window_handle = gfx::WindowHandle::FromHwnd(window_handle);
#elif KALI_LINUX
    gfx::WindowHandle gfx_window_handle = gfx::WindowHandle::FromXWindow(window_handle.xdisplay, window_handle.xwindow);
#endif

    if (SLANG_FAILED(
            m_device->get_gfx_device()->createSwapchain(gfx_desc, gfx_window_handle, m_gfx_swapchain.writeRef())))
        KALI_THROW(Exception("Failed to create swapchain!"));
}

Swapchain::Swapchain(const SwapchainDesc& desc, ref<Window> window, ref<Device> device)
    : Swapchain(desc, window->get_window_handle(), device)
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
