// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/formats.h"

#include "sgl/core/object.h"
#include "sgl/core/macros.h"

#include <slang-gfx.h>

namespace sgl {

struct FramebufferAttachmentDesc {
    ref<Texture> texture;
    uint32_t mip_level{0};
    uint32_t base_array_layer{0};
    uint32_t layer_count{1};
};

struct FramebufferDesc {
    std::vector<FramebufferAttachmentDesc> render_targets;
    std::optional<FramebufferAttachmentDesc> depth_stencil;
};

class SGL_API Framebuffer : public DeviceResource {
    SGL_OBJECT(Framebuffer)
public:
    Framebuffer(ref<Device> device, FramebufferDesc desc);
    virtual ~Framebuffer();

    const FramebufferDesc& desc() const { return m_desc; }

    gfx::IFramebufferLayout* gfx_framebuffer_layout() const { return m_gfx_framebuffer_layout; }
    gfx::IFramebuffer* gfx_framebuffer() const { return m_gfx_framebuffer; }
    gfx::IRenderPassLayout* gfx_render_pass_layout() const { return m_gfx_render_pass_layout; }

    std::string to_string() const override;

private:
    FramebufferDesc m_desc;
    Slang::ComPtr<gfx::IFramebufferLayout> m_gfx_framebuffer_layout;
    Slang::ComPtr<gfx::IFramebuffer> m_gfx_framebuffer;
    Slang::ComPtr<gfx::IRenderPassLayout> m_gfx_render_pass_layout;
};

} // namespace sgl
