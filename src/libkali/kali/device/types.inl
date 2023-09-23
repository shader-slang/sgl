#include "types.h"

#include "kali/core/error.h"

#include <slang.h>
#include <slang-gfx.h>

namespace kali {

[[nodiscard]] inline constexpr SlangStage get_gfx_slang_stage(ShaderStage shader_stage)
{
    static_assert(uint32_t(ShaderStage::none) == SLANG_STAGE_NONE);
    static_assert(uint32_t(ShaderStage::vertex) == SLANG_STAGE_VERTEX);
    static_assert(uint32_t(ShaderStage::hull) == SLANG_STAGE_HULL);
    static_assert(uint32_t(ShaderStage::domain) == SLANG_STAGE_DOMAIN);
    static_assert(uint32_t(ShaderStage::geometry) == SLANG_STAGE_GEOMETRY);
    static_assert(uint32_t(ShaderStage::fragment) == SLANG_STAGE_FRAGMENT);
    static_assert(uint32_t(ShaderStage::compute) == SLANG_STAGE_COMPUTE);
    static_assert(uint32_t(ShaderStage::ray_generation) == SLANG_STAGE_RAY_GENERATION);
    static_assert(uint32_t(ShaderStage::intersection) == SLANG_STAGE_INTERSECTION);
    static_assert(uint32_t(ShaderStage::any_hit) == SLANG_STAGE_ANY_HIT);
    static_assert(uint32_t(ShaderStage::closest_hit) == SLANG_STAGE_CLOSEST_HIT);
    static_assert(uint32_t(ShaderStage::miss) == SLANG_STAGE_MISS);
    static_assert(uint32_t(ShaderStage::callable) == SLANG_STAGE_CALLABLE);
    static_assert(uint32_t(ShaderStage::mesh) == SLANG_STAGE_MESH);
    static_assert(uint32_t(ShaderStage::amplification) == SLANG_STAGE_AMPLIFICATION);
    KALI_ASSERT(uint32_t(shader_stage) <= uint32_t(ShaderStage::amplification));
    return SlangStage(shader_stage);
}

[[nodiscard]] inline constexpr ShaderStage get_shader_stage(SlangStage stage)
{
    KALI_ASSERT(uint32_t(stage) <= SLANG_STAGE_AMPLIFICATION);
    return ShaderStage(stage);
}

[[nodiscard]] inline constexpr gfx::ComparisonFunc get_gfx_comparison_func(ComparisonFunc comparison_func)
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

[[nodiscard]] inline constexpr gfx::TextureFilteringMode get_gfx_texture_filtering_mode(TextureFilteringMode texture_filtering_mode)
{
    static_assert(uint32_t(TextureFilteringMode::point) == uint32_t(gfx::TextureFilteringMode::Point));
    static_assert(uint32_t(TextureFilteringMode::linear) == uint32_t(gfx::TextureFilteringMode::Linear));
    KALI_ASSERT(uint32_t(texture_filtering_mode) <= uint32_t(TextureFilteringMode::linear));
    return gfx::TextureFilteringMode(texture_filtering_mode);
}

[[nodiscard]] inline constexpr gfx::TextureAddressingMode get_gfx_texture_addressing_mode(TextureAddressingMode texture_addressing_mode)
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

[[nodiscard]] inline constexpr gfx::TextureReductionOp get_gfx_texture_reduction_op(TextureReductionOp texture_reduction_op)
{
    static_assert(uint32_t(TextureReductionOp::average) == uint32_t(gfx::TextureReductionOp::Average));
    static_assert(uint32_t(TextureReductionOp::comparison) == uint32_t(gfx::TextureReductionOp::Comparison));
    static_assert(uint32_t(TextureReductionOp::minimum) == uint32_t(gfx::TextureReductionOp::Minimum));
    static_assert(uint32_t(TextureReductionOp::maximum) == uint32_t(gfx::TextureReductionOp::Maximum));
    KALI_ASSERT(uint32_t(texture_reduction_op) <= uint32_t(TextureReductionOp::maximum));
    return gfx::TextureReductionOp(texture_reduction_op);
}


} // namespace kali
