// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/formats.h"

#include "sgl/core/macros.h"
#include "sgl/core/platform.h"
#include "sgl/core/object.h"

#include <slang-gfx.h>

namespace sgl {

class Window;

struct SwapchainDesc {
    /// Format of the swapchain images.
    Format format{Format::bgra8_unorm_srgb};
    /// Width of the swapchain images in pixels.
    uint32_t width{0};
    /// Height of the swapchain images in pixels.
    uint32_t height{0};
    /// Number of swapchain images.
    uint32_t image_count{3};
    /// Enable/disable vertical synchronization.
    bool enable_vsync{false};
};

class SGL_API Swapchain : public Object {
    SGL_OBJECT(Swapchain)
public:
    Swapchain(SwapchainDesc desc, WindowHandle window_handle, ref<Device> device);
    Swapchain(SwapchainDesc desc, Window* window, ref<Device> device);
    ~Swapchain();

    /// Returns the swapchain description.
    const SwapchainDesc& desc() const { return m_desc; }

    /// Returns the back buffer images.
    const std::vector<ref<Texture>>& images() const { return m_images; }

    /// Returns the back buffer image at position `index`.
    const ref<Texture>& get_image(uint32_t index) const;

    /// Present the next image in the swapchain.
    void present();

    /// Returns the index of next back buffer image that will be presented in the next `present` call.
    /// Returns -1 if no image is available and the caller should skip the frame.
    int acquire_next_image();

    /// Resizes the back buffers of this swapchain. All render target views and framebuffers
    /// referencing the back buffer images must be freed before calling this method.
    void resize(uint32_t width, uint32_t height);

    /// Returns true if the window is occluded.
    bool is_occluded() const;

    bool fullscreen_mode() const { return m_fullscreen; }
    void set_fullscreen_mode(bool fullscreen);


private:
    void get_images();

    SwapchainDesc m_desc;
    ref<Device> m_device;
    Slang::ComPtr<gfx::ISwapchain> m_gfx_swapchain;
    bool m_fullscreen{false};
    std::vector<ref<Texture>> m_images;
};
} // namespace sgl
