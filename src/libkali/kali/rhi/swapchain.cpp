#include "swapchain.h"

#include "kali/rhi/device.h"
#include "kali/rhi/command_queue.h"
#include "kali/rhi/helpers.h"

#include "kali/core/error.h"
#include "kali/core/window.h"

namespace kali {

Swapchain::Swapchain(SwapchainDesc desc, WindowHandle window_handle, ref<CommandQueue> queue, ref<Device> device)
    : m_desc(std::move(desc))
    , m_queue(std::move(queue))
    , m_device(std::move(device))
{
    KALI_ASSERT(m_device);

    gfx::ISwapchain::Desc gfx_desc{
        .format = get_gfx_format(m_desc.format),
        .width = gfx::GfxCount(m_desc.width),
        .height = gfx::GfxCount(m_desc.height),
        .imageCount = gfx::GfxCount(m_desc.image_count),
        .queue = m_queue->get_gfx_command_queue(),
        .enableVSync = m_desc.enable_vsync,
    };

#if KALI_WINDOWS
    gfx::WindowHandle gfx_window_handle = gfx::WindowHandle::FromHwnd(window_handle);
#elif KALI_LINUX
    gfx::WindowHandle gfx_window_handle = gfx::WindowHandle::FromXWindow(window_handle.xdisplay, window_handle.xwindow);
#endif

    SLANG_CALL(m_device->get_gfx_device()->createSwapchain(gfx_desc, gfx_window_handle, m_gfx_swapchain.writeRef()));

    get_images();
}

Swapchain::Swapchain(SwapchainDesc desc, ref<Window> window, ref<CommandQueue> queue, ref<Device> device)
    : Swapchain(std::move(desc), window->get_window_handle(), std::move(queue), std::move(device))
{
    m_window = window;
}

Swapchain::~Swapchain() { }

const ref<Texture>& Swapchain::get_image(uint32_t index) const
{
    KALI_ASSERT(index < m_desc.image_count);
    return m_images[index];
}

void Swapchain::present()
{
    SLANG_CALL(m_gfx_swapchain->present());
}

uint32_t Swapchain::acquire_next_image()
{
    int index = m_gfx_swapchain->acquireNextImage();
    if (index < 0)
        KALI_THROW("Failed to acquire next image");
    return index;
}

void Swapchain::resize(uint32_t width, uint32_t height)
{
    m_desc.width = width;
    m_desc.height = height;
    SLANG_CALL(m_gfx_swapchain->resize(width, height));
    get_images();
}

bool Swapchain::is_occluded() const
{
    return m_gfx_swapchain->isOccluded();
}

void Swapchain::set_fullscreen_mode(bool fullscreen)
{
    if (m_fullscreen != fullscreen) {
        SLANG_CALL(m_gfx_swapchain->setFullScreenMode(fullscreen));
        m_fullscreen = fullscreen;
    }
}

void Swapchain::get_images()
{
    m_images.clear();
    for (uint32_t i = 0; i < m_desc.image_count; ++i) {
        Slang::ComPtr<gfx::ITextureResource> resource;
        SLANG_CALL(m_gfx_swapchain->getImage(i, resource.writeRef()));

        m_images.push_back(m_device->create_texture_from_resource(
            {
                .type = TextureType::texture_2d,
                .format = m_desc.format,
                .width = m_desc.width,
                .height = m_desc.height,
                .mip_count = 1,
                .usage = ResourceUsage::render_target,
            },
            resource
        ));
    }
    KALI_ASSERT(m_images.size() == m_desc.image_count);
}


} // namespace kali
