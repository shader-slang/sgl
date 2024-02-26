// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/core/object.h"
#include "kali/core/window.h"
#include "kali/device/device.h"
#include "kali/device/swapchain.h"

namespace kali {

class Renderer {
public:
    Renderer(ref<Window> window)
    {
        m_device = Device::create();

        SwapchainDesc swapchain_desc{
            .format = Format::rgba8_unorm_srgb,
            .width = window->width(),
            .height = window->height(),
            .image_count = 3,
        };
        m_swapchain = m_device->create_swapchain(swapchain_desc, window);
    }

private:
    ref<Device> m_device;
    ref<Swapchain> m_swapchain;
};

} // namespace kali
