#pragma once

#include "kali/device/fwd.h"
#include "kali/device/formats.h"

#include "kali/core/macros.h"
#include "kali/core/platform.h"
#include "kali/core/object.h"

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
    Swapchain(SwapchainDesc desc, WindowHandle window_handle, ref<CommandQueue> queue, ref<Device> device);
    Swapchain(SwapchainDesc desc, ref<Window> window, ref<CommandQueue> queue, ref<Device> device);
    ~Swapchain();

    /// Returns the swapchain description.
    const SwapchainDesc& get_desc() const { return m_desc; }

    /// Returns the back buffer image at position `index`.
    const ref<Texture>& get_image(uint32_t index) const;

    /// Present the next image in the swapchain.
    void present();

    /// Returns the index of next back buffer image that will be presented in the next `present` call.
    /// If the swapchain is invalid/out-of-date, an error is thrown.
    uint32_t acquire_next_image();

    /// Resizes the back buffers of this swapchain. All render target views and framebuffers
    /// referencing the back buffer images must be freed before calling this method.
    void resize(uint32_t width, uint32_t height);

    /// Returns true if the window is occluded.
    bool is_occluded() const;

    bool get_fullscreen_mode() const { return m_fullscreen; }
    void set_fullscreen_mode(bool fullscreen);


private:
    void get_images();

    SwapchainDesc m_desc;
    ref<Window> m_window;
    ref<CommandQueue> m_queue;
    ref<Device> m_device;
    Slang::ComPtr<gfx::ISwapchain> m_gfx_swapchain;
    bool m_fullscreen{false};
    std::vector<ref<Texture>> m_images;
};
} // namespace kali