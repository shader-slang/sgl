// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/formats.h"

#include "sgl/core/object.h"
#include "sgl/core/macros.h"

#include <slang-gfx.h>

namespace sgl {

struct FramebufferLayoutTargetDesc {
    Format format{Format::unknown};
    uint32_t sample_count{0};

    auto operator<=>(const FramebufferLayoutTargetDesc&) const = default;
};

struct FramebufferLayoutDesc {
    std::vector<FramebufferLayoutTargetDesc> render_targets;
    std::optional<FramebufferLayoutTargetDesc> depth_stencil;

#if SGL_MACOS
    // macOS clang stdc++ doesn't support C++20 <=> operator for standard containers yet.
    bool operator<(const FramebufferLayoutDesc& other) const
    {
        return std::tie(render_targets, depth_stencil) < std::tie(other.render_targets, other.depth_stencil);
    }
#else
    auto operator<=>(const FramebufferLayoutDesc&) const = default;
#endif
};

class SGL_API FramebufferLayout : public DeviceResource {
    SGL_OBJECT(FramebufferLayout)
public:
    FramebufferLayout(ref<Device> device, FramebufferLayoutDesc desc);
    virtual ~FramebufferLayout();

    const FramebufferLayoutDesc& desc() const { return m_desc; }

    gfx::IFramebufferLayout* gfx_framebuffer_layout() const { return m_gfx_framebuffer_layout; }

    std::string to_string() const override;

private:
    FramebufferLayoutDesc m_desc;
    Slang::ComPtr<gfx::IFramebufferLayout> m_gfx_framebuffer_layout;
};

struct FramebufferDesc {
    /// List of render targets.
    std::vector<ref<ResourceView>> render_targets;
    /// Depth-stencil target (optional).
    ref<ResourceView> depth_stencil;
    /// Framebuffer layout (optional). If not provided, framebuffer layout is determined from the render targets.
    ref<FramebufferLayout> layout;
};

class SGL_API Framebuffer : public DeviceResource {
    SGL_OBJECT(Framebuffer)
public:
    Framebuffer(ref<Device> device, FramebufferDesc desc);
    virtual ~Framebuffer();

    const FramebufferDesc& desc() const { return m_desc; }

    const ref<FramebufferLayout>& layout() const { return m_desc.layout; }

    gfx::IFramebuffer* gfx_framebuffer() const { return m_gfx_framebuffer; }
    gfx::IRenderPassLayout* gfx_render_pass_layout() const { return m_gfx_render_pass_layout; }

    std::string to_string() const override;

private:
    FramebufferDesc m_desc;
    Slang::ComPtr<gfx::IFramebuffer> m_gfx_framebuffer;
    Slang::ComPtr<gfx::IRenderPassLayout> m_gfx_render_pass_layout;
};

} // namespace sgl
