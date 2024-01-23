#pragma once

#include "kali/device/fwd.h"
#include "kali/device/device_resource.h"
#include "kali/device/formats.h"

#include "kali/core/object.h"
#include "kali/core/macros.h"

#include <slang-gfx.h>

namespace kali {

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

class KALI_API Framebuffer : public DeviceResource {
    KALI_OBJECT(Framebuffer)
public:
    Framebuffer(ref<Device> device, FramebufferDesc desc);
    virtual ~Framebuffer() = default;

    const FramebufferDesc& desc() const { return m_desc; }

    gfx::IFramebufferLayout* gfx_framebuffer_layout() const { return m_gfx_framebuffer_layout; }
    gfx::IFramebuffer* gfx_framebuffer() const { return m_gfx_framebuffer; }

    std::string to_string() const override;

private:
    FramebufferDesc m_desc;
    Slang::ComPtr<gfx::IFramebufferLayout> m_gfx_framebuffer_layout;
    Slang::ComPtr<gfx::IFramebuffer> m_gfx_framebuffer;
};


} // namespace kali
