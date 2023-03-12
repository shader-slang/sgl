#pragma once

#include "fwd.h"

#include "core/object.h"

#include <slang-gfx.h>

namespace kali {

class Window;

struct SwapchainDesc {
    // Format format;
    uint32_t width, height;
    uint32_t image_count;
    // ICommandQueue* queue;
    bool enable_vsync;
};

class Swapchain : public Object {
public:
    Swapchain(const SwapchainDesc& desc, ref<Window> window, ref<Device> device);
    ~Swapchain();

private:
    SwapchainDesc m_desc;
    ref<Window> m_window;
    ref<Device> m_device;
    Slang::ComPtr<gfx::ISwapchain> m_gfx_swapchain;
};
} // namespace kali
