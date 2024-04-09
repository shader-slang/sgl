// SPDX-License-Identifier: Apache-2.0

#include "framebuffer.h"

#include "sgl/device/device.h"
#include "sgl/device/helpers.h"

#include "sgl/core/short_vector.h"
#include "sgl/core/type_utils.h"
#include "sgl/core/string.h"

namespace sgl {

// ----------------------------------------------------------------------------
// FramebufferLayout
// ----------------------------------------------------------------------------

FramebufferLayout::FramebufferLayout(ref<Device> device, FramebufferLayoutDesc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    short_vector<gfx::IFramebufferLayout::TargetLayout, 16> gfx_render_targets;
    for (const auto& render_target : m_desc.render_targets) {
        gfx_render_targets.push_back({
            .format = static_cast<gfx::Format>(render_target.format),
            .sampleCount = narrow_cast<gfx::GfxCount>(render_target.sample_count),
        });
    }

    gfx::IFramebufferLayout::TargetLayout gfx_depth_stencil;
    if (m_desc.depth_stencil) {
        const auto& depth_stencil = *m_desc.depth_stencil;
        gfx_depth_stencil = {
            .format = static_cast<gfx::Format>(depth_stencil.format),
            .sampleCount = narrow_cast<gfx::GfxCount>(depth_stencil.sample_count),
        };
    }

    gfx::IFramebufferLayout::Desc gfx_framebuffer_layout_desc{
        .renderTargetCount = static_cast<gfx::GfxCount>(gfx_render_targets.size()),
        .renderTargets = gfx_render_targets.data(),
        .depthStencil = m_desc.depth_stencil ? &gfx_depth_stencil : nullptr,
    };
    SLANG_CALL(m_device->gfx_device()
                   ->createFramebufferLayout(gfx_framebuffer_layout_desc, m_gfx_framebuffer_layout.writeRef()));
}

FramebufferLayout::~FramebufferLayout() { }

inline std::string to_string(const FramebufferLayoutTargetDesc& desc)
{
    return fmt::format("(format={}, sample_count={})", desc.format, desc.sample_count);
}

std::string FramebufferLayout::to_string() const
{
    return fmt::format(
        "FramebufferLayout(\n"
        "  render_targets = {},\n"
        "  depth_stencil = {}\n"
        ")",
        string::indent(string::list_to_string(m_desc.render_targets)),
        m_desc.depth_stencil ? sgl::to_string(*m_desc.depth_stencil) : "null"
    );
}

// ----------------------------------------------------------------------------
// Framebuffer
// ----------------------------------------------------------------------------

Framebuffer::Framebuffer(ref<Device> device, FramebufferDesc desc)
    : DeviceResource(std::move(device))
    , m_desc(std::move(desc))
{
    // If no layout is provided, derive one from the provided render targets.
    if (!m_desc.layout) {
        FramebufferLayoutDesc layout_desc;
        for (const auto& render_target : m_desc.render_targets) {
            Texture* texture = render_target->resource()->as_texture();
            layout_desc.render_targets.push_back({
                .format = texture->format(),
                .sample_count = texture->desc().sample_count,
            });
        }
        if (m_desc.depth_stencil) {
            Texture* texture = m_desc.depth_stencil->resource()->as_texture();
            layout_desc.depth_stencil = {
                .format = texture->format(),
                .sample_count = texture->desc().sample_count,
            };
        }
        m_desc.layout = make_ref<FramebufferLayout>(m_device, layout_desc);
    }

    short_vector<gfx::IResourceView*, 16> gfx_render_target_views;
    short_vector<gfx::IRenderPassLayout::TargetAccessDesc, 16> gfx_render_target_access;
    for (size_t i = 0; i < m_desc.render_targets.size(); ++i) {
        const ResourceView* render_target = m_desc.render_targets[i];
        SGL_CHECK(render_target->type() == ResourceViewType::render_target, "Resource view is not render target");
        gfx_render_target_views.push_back(render_target->gfx_resource_view());
        gfx_render_target_access.push_back({
            .loadOp = gfx::IRenderPassLayout::TargetLoadOp::Load,
            .stencilLoadOp = gfx::IRenderPassLayout::TargetLoadOp::DontCare,
            .storeOp = gfx::IRenderPassLayout::TargetStoreOp::Store,
            .stencilStoreOp = gfx::IRenderPassLayout::TargetStoreOp::DontCare,
            .initialState = gfx::ResourceState::RenderTarget,
            .finalState = gfx::ResourceState::RenderTarget,
        });
    }

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
        const ResourceView* depth_stencil = m_desc.depth_stencil;
        SGL_CHECK(depth_stencil->type() == ResourceViewType::depth_stencil, "Resource view is not depth stencil");
        gfx_depth_stencil_view = depth_stencil->gfx_resource_view();
    }

    // Create framebuffer
    gfx::IFramebuffer::Desc gfx_framebuffer_desc{
        .renderTargetCount = static_cast<gfx::GfxCount>(gfx_render_target_views.size()),
        .renderTargetViews = gfx_render_target_views.data(),
        .depthStencilView = gfx_depth_stencil_view,
        .layout = m_desc.layout->gfx_framebuffer_layout(),
    };
    SLANG_CALL(m_device->gfx_device()->createFramebuffer(gfx_framebuffer_desc, m_gfx_framebuffer.writeRef()));

    // Create render pass layout
    gfx::IRenderPassLayout::Desc gfx_render_pass_layout_desc{
        .framebufferLayout = m_desc.layout->gfx_framebuffer_layout(),
        .renderTargetCount = static_cast<gfx::GfxCount>(gfx_render_target_views.size()),
        .renderTargetAccess = gfx_render_target_access.data(),
        .depthStencilAccess = gfx_depth_stencil_view ? &gfx_depth_stencil_access : nullptr,
    };
    SLANG_CALL(
        m_device->gfx_device()->createRenderPassLayout(gfx_render_pass_layout_desc, m_gfx_render_pass_layout.writeRef())
    );
}

Framebuffer::~Framebuffer()
{
    m_device->deferred_release(std::move(m_gfx_framebuffer));
    m_device->deferred_release(std::move(m_gfx_render_pass_layout));
}

std::string Framebuffer::to_string() const
{
    return fmt::format(
        "Framebuffer(\n"
        "  layout = {},\n"
        "  render_targets = {},\n"
        "  depth_stencil = {}\n"
        ")",
        string::indent(m_desc.layout->to_string()),
        string::indent(string::list_to_string(std::span{m_desc.render_targets})),
        m_desc.depth_stencil ? m_desc.depth_stencil->to_string() : "null"
    );
}

} // namespace sgl
