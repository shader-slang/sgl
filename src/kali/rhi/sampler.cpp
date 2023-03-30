#include "sampler.h"
#include "device.h"
#include "helpers.h"

#include "core/assert.h"


namespace kali {

inline gfx::ComparisonFunc get_gfx_comparison_func(ComparisonFunc comparison_func)
{
    static_assert(uint32_t(ComparisonFunc::never) == uint32_t(gfx::ComparisonFunc::Never));
    static_assert(uint32_t(ComparisonFunc::less) == uint32_t(gfx::ComparisonFunc::Less));
    static_assert(uint32_t(ComparisonFunc::equal) == uint32_t(gfx::ComparisonFunc::Equal));
    static_assert(uint32_t(ComparisonFunc::less_equal) == uint32_t(gfx::ComparisonFunc::LessEqual));
    static_assert(uint32_t(ComparisonFunc::greater) == uint32_t(gfx::ComparisonFunc::Greater));
    static_assert(uint32_t(ComparisonFunc::not_equal) == uint32_t(gfx::ComparisonFunc::NotEqual));
    static_assert(uint32_t(ComparisonFunc::greater_equal) == uint32_t(gfx::ComparisonFunc::GreaterEqual));
    static_assert(uint32_t(ComparisonFunc::always) == uint32_t(gfx::ComparisonFunc::Always));
    KALI_ASSERT(uint32_t(comparison_func) <= uint32_t(ComparisonFunc::always));
    return gfx::ComparisonFunc(comparison_func);
}

inline gfx::TextureFilteringMode get_gfx_texture_filtering_mode(TextureFilteringMode texture_filtering_mode)
{
    static_assert(uint32_t(TextureFilteringMode::point) == uint32_t(gfx::TextureFilteringMode::Point));
    static_assert(uint32_t(TextureFilteringMode::linear) == uint32_t(gfx::TextureFilteringMode::Linear));
    KALI_ASSERT(uint32_t(texture_filtering_mode) <= uint32_t(TextureFilteringMode::linear));
    return gfx::TextureFilteringMode(texture_filtering_mode);
}

inline gfx::TextureAddressingMode get_gfx_texture_addressing_mode(TextureAddressingMode texture_addressing_mode)
{
    // clang-format off
    static_assert(uint32_t(TextureAddressingMode::wrap) == uint32_t(gfx::TextureAddressingMode::Wrap));
    static_assert(uint32_t(TextureAddressingMode::clamp_to_edge) == uint32_t(gfx::TextureAddressingMode::ClampToEdge));
    static_assert(uint32_t(TextureAddressingMode::clamp_to_border) == uint32_t(gfx::TextureAddressingMode::ClampToBorder));
    static_assert(uint32_t(TextureAddressingMode::mirror_repeat) == uint32_t(gfx::TextureAddressingMode::MirrorRepeat));
    static_assert(uint32_t(TextureAddressingMode::mirror_once) == uint32_t(gfx::TextureAddressingMode::MirrorOnce));
    // clang-format on
    KALI_ASSERT(uint32_t(texture_addressing_mode) <= uint32_t(TextureAddressingMode::mirror_once));
    return gfx::TextureAddressingMode(texture_addressing_mode);
}

inline gfx::TextureReductionOp get_gfx_texture_reduction_op(TextureReductionOp texture_reduction_op)
{
    static_assert(uint32_t(TextureReductionOp::average) == uint32_t(gfx::TextureReductionOp::Average));
    static_assert(uint32_t(TextureReductionOp::comparison) == uint32_t(gfx::TextureReductionOp::Comparison));
    static_assert(uint32_t(TextureReductionOp::minimum) == uint32_t(gfx::TextureReductionOp::Minimum));
    static_assert(uint32_t(TextureReductionOp::maximum) == uint32_t(gfx::TextureReductionOp::Maximum));
    KALI_ASSERT(uint32_t(texture_reduction_op) <= uint32_t(TextureReductionOp::maximum));
    return gfx::TextureReductionOp(texture_reduction_op);
}

Sampler::Sampler(const SamplerDesc& desc, ref<Device> device)
    : m_desc(desc)
    , m_device(device)
{
    gfx::ISamplerState::Desc gfx_desc{
        .minFilter = get_gfx_texture_filtering_mode(m_desc.min_filter),
        .magFilter = get_gfx_texture_filtering_mode(m_desc.mag_filter),
        .mipFilter = get_gfx_texture_filtering_mode(m_desc.mip_filter),
        .reductionOp = get_gfx_texture_reduction_op(m_desc.reduction_op),
        .addressU = get_gfx_texture_addressing_mode(m_desc.address_u),
        .addressV = get_gfx_texture_addressing_mode(m_desc.address_v),
        .addressW = get_gfx_texture_addressing_mode(m_desc.address_w),
        .mipLODBias = m_desc.mip_lod_bias,
        .maxAnisotropy = m_desc.max_anisotropy,
        .comparisonFunc = get_gfx_comparison_func(m_desc.comparison_func),
        .borderColor = {m_desc.border_color.x, m_desc.border_color.y, m_desc.border_color.z, m_desc.border_color.w},
        .minLOD = m_desc.min_lod,
        .maxLOD = m_desc.max_lod,
    };
    SLANG_CALL(m_device->get_gfx_device()->createSamplerState(gfx_desc, m_gfx_sampler.writeRef()));
}

} // namespace kali
