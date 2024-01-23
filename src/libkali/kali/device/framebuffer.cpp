#include "framebuffer.h"

#include "kali/device/device.h"
#include "kali/device/helpers.h"

#include "kali/core/short_vector.h"
#include "kali/core/type_utils.h"
#include "kali/core/string.h"

namespace kali {

inline std::string to_string(const FramebufferAttachmentDesc& desc)
{
    return fmt::format(
        "(format={}, sample_count={}, mip_level={}, base_array_layer={}, layer_count={})",
        desc.texture->format(),
        desc.texture->desc().sample_count,
        desc.mip_level,
        desc.base_array_layer,
        desc.layer_count
    );
}

Framebuffer::Framebuffer(ref<Device> device, FramebufferDesc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    short_vector<gfx::IFramebufferLayout::TargetLayout, 16> gfx_render_targets;
    short_vector<gfx::IResourceView*, 16> gfx_render_target_views;
    short_vector<gfx::IRenderPassLayout::TargetAccessDesc, 16> gfx_render_target_access;
    for (size_t i = 0; i < m_desc.render_targets.size(); ++i) {
        const FramebufferAttachmentDesc& attachment = m_desc.render_targets[i];
        KALI_CHECK(attachment.texture, "Render target texture is null");
        KALI_CHECK(
            is_set(attachment.texture->desc().usage, ResourceUsage::render_target),
            "Texture is not render target"
        );
        gfx_render_targets.push_back({
            .format = static_cast<gfx::Format>(attachment.texture->format()),
            .sampleCount = narrow_cast<gfx::GfxCount>(attachment.texture->desc().sample_count),
        });
        gfx_render_target_views.push_back(
            attachment.texture->get_rtv(attachment.mip_level, attachment.base_array_layer, attachment.layer_count)
                ->gfx_resource_view()
        );
        gfx_render_target_access.push_back({
            .loadOp = gfx::IRenderPassLayout::TargetLoadOp::Load,
            .stencilLoadOp = gfx::IRenderPassLayout::TargetLoadOp::DontCare,
            .storeOp = gfx::IRenderPassLayout::TargetStoreOp::Store,
            .stencilStoreOp = gfx::IRenderPassLayout::TargetStoreOp::DontCare,
            .initialState = gfx::ResourceState::RenderTarget,
            .finalState = gfx::ResourceState::RenderTarget,
        });
    }

    gfx::IFramebufferLayout::TargetLayout gfx_depth_stencil;
    gfx::IResourceView* gfx_depth_stencil_view{nullptr};
    gfx::IRenderPassLayout::TargetAccessDesc gfx_depth_stencil_access{
        .loadOp = gfx::IRenderPassLayout::TargetLoadOp::Load,
        .stencilLoadOp = gfx::IRenderPassLayout::TargetLoadOp::Load,
        .storeOp = gfx::IRenderPassLayout::TargetStoreOp::Store,
        .stencilStoreOp = gfx::IRenderPassLayout::TargetStoreOp::Store,
        .initialState = gfx::ResourceState::DepthWrite,
        .finalState = gfx::ResourceState::DepthWrite,
    };
    if (m_desc.depth_stencil) {
        const FramebufferAttachmentDesc& attachment = *m_desc.depth_stencil;
        KALI_CHECK(attachment.texture, "Render target texture is null");
        KALI_CHECK(
            is_set(attachment.texture->desc().usage, ResourceUsage::depth_stencil),
            "Texture is not depth stencil"
        );
        gfx_depth_stencil = {
            .format = static_cast<gfx::Format>(attachment.texture->format()),
            .sampleCount = narrow_cast<gfx::GfxCount>(attachment.texture->desc().sample_count),
        };
        gfx_depth_stencil_view
            = attachment.texture->get_dsv(attachment.mip_level, attachment.base_array_layer, attachment.layer_count)
                  ->gfx_resource_view();
    }

    // Create framebuffer layout
    gfx::IFramebufferLayout::Desc gfx_framebuffer_layout_desc{
        .renderTargetCount = static_cast<gfx::GfxCount>(gfx_render_targets.size()),
        .renderTargets = gfx_render_targets.data(),
        .depthStencil = m_desc.depth_stencil ? &gfx_depth_stencil : nullptr,
    };
    SLANG_CALL(m_device->gfx_device()
                   ->createFramebufferLayout(gfx_framebuffer_layout_desc, m_gfx_framebuffer_layout.writeRef()));

    // Create framebuffer
    gfx::IFramebuffer::Desc gfx_framebuffer_desc{
        .renderTargetCount = static_cast<gfx::GfxCount>(gfx_render_target_views.size()),
        .renderTargetViews = gfx_render_target_views.data(),
        .depthStencilView = gfx_depth_stencil_view,
        .layout = m_gfx_framebuffer_layout,
    };
    SLANG_CALL(m_device->gfx_device()->createFramebuffer(gfx_framebuffer_desc, m_gfx_framebuffer.writeRef()));

    // Create render pass layout
    gfx::IRenderPassLayout::Desc gfx_render_pass_layout_desc{
        .framebufferLayout = m_gfx_framebuffer_layout,
        .renderTargetCount = static_cast<gfx::GfxCount>(gfx_render_targets.size()),
        .renderTargetAccess = gfx_render_target_access.data(),
        .depthStencilAccess = m_desc.depth_stencil ? &gfx_depth_stencil_access : nullptr,
    };
    SLANG_CALL(
        m_device->gfx_device()->createRenderPassLayout(gfx_render_pass_layout_desc, m_gfx_render_pass_layout.writeRef())
    );
}

std::string Framebuffer::to_string() const
{
    return fmt::format(
        "Framebuffer(\n"
        "  render_targets={},\n"
        "  depth_stencil={}\n"
        ")",
        string::indent(string::list_to_string(std::span{m_desc.render_targets})),
        m_desc.depth_stencil ? string::indent(kali::to_string(*m_desc.depth_stencil)) : "null"
    );
}

} // namespace kali
