// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/formats.h"
#include "sgl/device/resource.h"

#include "sgl/core/macros.h"
#include "sgl/core/platform.h"
#include "sgl/core/object.h"

#include <slang-rhi.h>

namespace sgl {

class Window;

struct SurfaceInfo {
    /// Preferred format for the surface.
    Format preferred_format;
    /// Supported texture usages.
    TextureUsage supported_usage;
    /// Supported texture formats.
    std::vector<Format> formats;
};

struct SurfaceConfig {
    /// Surface texture format.
    Format format = Format::undefined;
    /// Surface texture usage.
    TextureUsage usage = TextureUsage::render_target;
    /// Surface texture width.
    uint32_t width = 0;
    /// Surface texture height.
    uint32_t height = 0;
    /// Desired number of images.
    uint32_t desired_image_count = 3;
    /// Enable/disable vertical synchronization.
    bool vsync = true;
};

class SGL_API Surface : public Object {
    SGL_OBJECT(Surface)
public:
    Surface(WindowHandle window_handle, ref<Device> device);
    Surface(Window* window, ref<Device> device);
    ~Surface();

    /// Returns the surface info.
    const SurfaceInfo& info() const { return m_info; };

    /// Returns the surface config.
    const SurfaceConfig& config() const { return m_config; };

    /// Configure the surface.
    void configure(const SurfaceConfig& config);

    /// Returns the current surface image texture.
    ref<Texture> get_current_texture();

    /// Present the current surface image texture.
    void present();

private:
    void get_images();

    SurfaceInfo m_info;
    SurfaceConfig m_config;
    ref<Device> m_device;
    Slang::ComPtr<rhi::ISurface> m_rhi_surface;
};
} // namespace sgl
