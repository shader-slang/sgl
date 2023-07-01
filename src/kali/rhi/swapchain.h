#pragma once

#include "fwd.h"
#include "formats.h"

#include "core/platform.h"
#include "core/object.h"

#include <slang-gfx.h>

namespace kali {

class Window;

struct SwapchainDesc {
    Format format;
    uint32_t width, height;
    uint32_t image_count;
    bool enable_vsync;
};

class KALI_API Swapchain : public Object {
    KALI_OBJECT(Swapchain)
public:
    Swapchain(const SwapchainDesc& desc, WindowHandle window_handle, ref<Device> device);
    Swapchain(const SwapchainDesc& desc, ref<Window> window, ref<Device> device);
    ~Swapchain();

    const SwapchainDesc& get_desc() const { return m_desc; }

    void resize(uint32_t width, uint32_t height);

private:
    SwapchainDesc m_desc;
    ref<Window> m_window;
    ref<Device> m_device;
    Slang::ComPtr<gfx::ISwapchain> m_gfx_swapchain;
};
} // namespace kali
