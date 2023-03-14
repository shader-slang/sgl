#pragma once

#include "core/object.h"
#include "core/window.h"
#include "rhi/device.h"
#include "rhi/swapchain.h"

namespace kali {

class Renderer {
public:
    Renderer(ref<Window> window)
    {
        m_device = new Device();

        SwapchainDesc swapchain_desc{
            .format = Format::R8G8B8A8_UNORM_SRGB,
            .width = window->get_width(),
            .height = window->get_height(),
            .image_count = 3,
        };
        m_swapchain = m_device->create_swapchain(swapchain_desc, window);
    }

private:
    ref<Device> m_device;
    ref<Swapchain> m_swapchain;
};

} // namespace kali
