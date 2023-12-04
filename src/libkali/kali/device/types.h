#pragma once

#include "kali/core/macros.h"
#include "kali/core/enum.h"

#include <slang-gfx.h>

namespace kali {

enum class ShaderModel : uint32_t {
    unknown = 0,
    sm_6_0 = 60,
    sm_6_1 = 61,
    sm_6_2 = 62,
    sm_6_3 = 63,
    sm_6_4 = 64,
    sm_6_5 = 65,
    sm_6_6 = 66,
    sm_6_7 = 67,
};
KALI_ENUM_INFO(
    ShaderModel,
    {
        {ShaderModel::unknown, "unknown"},
        {ShaderModel::sm_6_0, "sm_6_0"},
        {ShaderModel::sm_6_1, "sm_6_1"},
        {ShaderModel::sm_6_2, "sm_6_2"},
        {ShaderModel::sm_6_3, "sm_6_3"},
        {ShaderModel::sm_6_4, "sm_6_4"},
        {ShaderModel::sm_6_5, "sm_6_5"},
        {ShaderModel::sm_6_6, "sm_6_6"},
        {ShaderModel::sm_6_7, "sm_6_7"},
    }
);
KALI_ENUM_REGISTER(ShaderModel);

inline uint32_t get_shader_model_major_version(ShaderModel sm)
{
    return static_cast<uint32_t>(sm) / 10;
}

inline uint32_t get_shader_model_minor_version(ShaderModel sm)
{
    return static_cast<uint32_t>(sm) % 10;
}

enum class ShaderStage : uint32_t {
    none = SLANG_STAGE_NONE,
    vertex = SLANG_STAGE_VERTEX,
    hull = SLANG_STAGE_HULL,
    domain = SLANG_STAGE_DOMAIN,
    geometry = SLANG_STAGE_GEOMETRY,
    fragment = SLANG_STAGE_FRAGMENT,
    compute = SLANG_STAGE_COMPUTE,
    ray_generation = SLANG_STAGE_RAY_GENERATION,
    intersection = SLANG_STAGE_INTERSECTION,
    any_hit = SLANG_STAGE_ANY_HIT,
    closest_hit = SLANG_STAGE_CLOSEST_HIT,
    miss = SLANG_STAGE_MISS,
    callable = SLANG_STAGE_CALLABLE,
    mesh = SLANG_STAGE_MESH,
    amplification = SLANG_STAGE_AMPLIFICATION,
};

KALI_ENUM_INFO(
    ShaderStage,
    {
        {ShaderStage::none, "none"},
        {ShaderStage::vertex, "vertex"},
        {ShaderStage::hull, "hull"},
        {ShaderStage::domain, "domain"},
        {ShaderStage::geometry, "geometry"},
        {ShaderStage::fragment, "fragment"},
        {ShaderStage::compute, "compute"},
        {ShaderStage::ray_generation, "ray_generation"},
        {ShaderStage::intersection, "intersection"},
        {ShaderStage::any_hit, "any_hit"},
        {ShaderStage::closest_hit, "closest_hit"},
        {ShaderStage::miss, "miss"},
        {ShaderStage::callable, "callable"},
        {ShaderStage::mesh, "mesh"},
        {ShaderStage::amplification, "amplification"},
    }
);
KALI_ENUM_REGISTER(ShaderStage);

enum class ComparisonFunc : uint32_t {
    never = static_cast<uint32_t>(gfx::ComparisonFunc::Never),
    less = static_cast<uint32_t>(gfx::ComparisonFunc::Less),
    equal = static_cast<uint32_t>(gfx::ComparisonFunc::Equal),
    less_equal = static_cast<uint32_t>(gfx::ComparisonFunc::LessEqual),
    greater = static_cast<uint32_t>(gfx::ComparisonFunc::Greater),
    not_equal = static_cast<uint32_t>(gfx::ComparisonFunc::NotEqual),
    greater_equal = static_cast<uint32_t>(gfx::ComparisonFunc::GreaterEqual),
    always = static_cast<uint32_t>(gfx::ComparisonFunc::Always),
};

KALI_ENUM_INFO(
    ComparisonFunc,
    {
        {ComparisonFunc::never, "never"},
        {ComparisonFunc::less, "less"},
        {ComparisonFunc::equal, "equal"},
        {ComparisonFunc::less_equal, "less_equal"},
        {ComparisonFunc::greater, "greater"},
        {ComparisonFunc::not_equal, "not_equal"},
        {ComparisonFunc::greater_equal, "greater_equal"},
        {ComparisonFunc::always, "always"},
    }
);
KALI_ENUM_REGISTER(ComparisonFunc);

enum class TextureFilteringMode : uint32_t {
    point = static_cast<uint32_t>(gfx::TextureFilteringMode::Point),
    linear = static_cast<uint32_t>(gfx::TextureFilteringMode::Linear),
};

KALI_ENUM_INFO(
    TextureFilteringMode,
    {
        {TextureFilteringMode::point, "point"},
        {TextureFilteringMode::linear, "linear"},
    }
);
KALI_ENUM_REGISTER(TextureFilteringMode);

enum class TextureAddressingMode : uint32_t {
    wrap = static_cast<uint32_t>(gfx::TextureAddressingMode::Wrap),
    clamp_to_edge = static_cast<uint32_t>(gfx::TextureAddressingMode::ClampToEdge),
    clamp_to_border = static_cast<uint32_t>(gfx::TextureAddressingMode::ClampToBorder),
    mirror_repeat = static_cast<uint32_t>(gfx::TextureAddressingMode::MirrorRepeat),
    mirror_once = static_cast<uint32_t>(gfx::TextureAddressingMode::MirrorOnce),
};

KALI_ENUM_INFO(
    TextureAddressingMode,
    {
        {TextureAddressingMode::wrap, "wrap"},
        {TextureAddressingMode::clamp_to_edge, "clamp_to_edge"},
        {TextureAddressingMode::clamp_to_border, "clamp_to_border"},
        {TextureAddressingMode::mirror_repeat, "mirror_repeat"},
        {TextureAddressingMode::mirror_once, "mirror_once"},
    }
);
KALI_ENUM_REGISTER(TextureAddressingMode);

enum class TextureReductionOp : uint32_t {
    average = static_cast<uint32_t>(gfx::TextureReductionOp::Average),
    comparison = static_cast<uint32_t>(gfx::TextureReductionOp::Comparison),
    minimum = static_cast<uint32_t>(gfx::TextureReductionOp::Minimum),
    maximum = static_cast<uint32_t>(gfx::TextureReductionOp::Maximum),
};

KALI_ENUM_INFO(
    TextureReductionOp,
    {
        {TextureReductionOp::average, "average"},
        {TextureReductionOp::comparison, "comparison"},
        {TextureReductionOp::minimum, "minimum"},
        {TextureReductionOp::maximum, "maximum"},
    }
);
KALI_ENUM_REGISTER(TextureReductionOp);

enum class PrimitiveType : uint8_t {
    point = static_cast<uint8_t>(gfx::PrimitiveType::Point),
    line = static_cast<uint8_t>(gfx::PrimitiveType::Line),
    triangle = static_cast<uint8_t>(gfx::PrimitiveType::Triangle),
    patch = static_cast<uint8_t>(gfx::PrimitiveType::Patch),
};

KALI_ENUM_INFO(
    PrimitiveType,
    {
        {PrimitiveType::point, "point"},
        {PrimitiveType::line, "line"},
        {PrimitiveType::triangle, "triangle"},
        {PrimitiveType::patch, "patch"},
    }
);
KALI_ENUM_REGISTER(PrimitiveType);

enum class StencilOp : uint8_t {
    keep = static_cast<uint8_t>(gfx::StencilOp::Keep),
    zero = static_cast<uint8_t>(gfx::StencilOp::Keep),
    replace = static_cast<uint8_t>(gfx::StencilOp::Keep),
    increment_saturate = static_cast<uint8_t>(gfx::StencilOp::Keep),
    decrement_saturate = static_cast<uint8_t>(gfx::StencilOp::Keep),
    invert = static_cast<uint8_t>(gfx::StencilOp::Keep),
    increment_wrap = static_cast<uint8_t>(gfx::StencilOp::Keep),
    decrement_wrap = static_cast<uint8_t>(gfx::StencilOp::Keep),
};

KALI_ENUM_INFO(
    StencilOp,
    {
        {StencilOp::keep, "keep"},
        {StencilOp::zero, "zero"},
        {StencilOp::replace, "replace"},
        {StencilOp::increment_saturate, "increment_saturate"},
        {StencilOp::decrement_saturate, "decrement_saturate"},
        {StencilOp::invert, "invert"},
        {StencilOp::increment_wrap, "increment_wrap"},
        {StencilOp::decrement_wrap, "decrement_wrap"},
    }
);
KALI_ENUM_REGISTER(StencilOp);

enum class FillMode : uint8_t {
    solid = static_cast<uint8_t>(gfx::FillMode::Solid),
    wireframe = static_cast<uint8_t>(gfx::FillMode::Wireframe),
};

KALI_ENUM_INFO(
    FillMode,
    {
        {FillMode::solid, "solid"},
        {FillMode::wireframe, "wireframe"},
    }
);
KALI_ENUM_REGISTER(FillMode);

enum class CullMode : uint8_t {
    none = static_cast<uint8_t>(gfx::CullMode::None),
    front = static_cast<uint8_t>(gfx::CullMode::Front),
    back = static_cast<uint8_t>(gfx::CullMode::Back),
};

KALI_ENUM_INFO(
    CullMode,
    {
        {CullMode::none, "none"},
        {CullMode::front, "front"},
        {CullMode::back, "back"},
    }
);
KALI_ENUM_REGISTER(CullMode);

enum class FrontFaceMode : uint8_t {
    counter_clockwise = static_cast<uint8_t>(gfx::FrontFaceMode::CounterClockwise),
    clockwise = static_cast<uint8_t>(gfx::FrontFaceMode::Clockwise),
};

KALI_ENUM_INFO(
    FrontFaceMode,
    {
        {FrontFaceMode::counter_clockwise, "counter_clockwise"},
        {FrontFaceMode::clockwise, "clockwise"},
    }
);
KALI_ENUM_REGISTER(FrontFaceMode);

struct DepthStencilOpDesc {
    StencilOp stencil_fail_op{StencilOp::keep};
    StencilOp stencil_depth_fail_op{StencilOp::keep};
    StencilOp stencil_pass_op{StencilOp::keep};
    ComparisonFunc stencil_func{ComparisonFunc::always};
};

struct DepthStencilDesc {
    bool depth_test_enable{false};
    bool depth_write_enable{true};
    ComparisonFunc depth_func = ComparisonFunc::less;

    bool stencil_enable{false};
    uint32_t stencil_read_mask{0xffffffff};
    uint32_t stencil_write_mask{0xffffffff};
    DepthStencilOpDesc front_face;
    DepthStencilOpDesc back_face;

    uint32_t stencil_ref = 0;
};

struct RasterizerDesc {
    FillMode fill_mode{FillMode::solid};
    CullMode cull_mode{CullMode::none};
    FrontFaceMode front_face{FrontFaceMode::counter_clockwise};
    int32_t depth_bias{0};
    float depth_bias_clamp{0.0f};
    float slope_scaled_depth_bias{0.0f};
    bool depth_clip_enable{true};
    bool scissor_enable{false};
    bool multisample_enable{false};
    bool antialiased_line_enable{false};
    bool enable_conservative_rasterization{false};
    uint32_t forced_sample_count{0};
};

enum class LogicOp : uint8_t {
    no_op = static_cast<uint8_t>(gfx::LogicOp::NoOp),
};

KALI_ENUM_INFO(
    LogicOp,
    {
        {LogicOp::no_op, "no_op"},
    }
);
KALI_ENUM_REGISTER(LogicOp);

enum class BlendOp : uint8_t {
    add = static_cast<uint8_t>(gfx::BlendOp::Add),
    subtract = static_cast<uint8_t>(gfx::BlendOp::Subtract),
    reverse_subtract = static_cast<uint8_t>(gfx::BlendOp::ReverseSubtract),
    min = static_cast<uint8_t>(gfx::BlendOp::Min),
    max = static_cast<uint8_t>(gfx::BlendOp::Max),
};

KALI_ENUM_INFO(
    BlendOp,
    {
        {BlendOp::add, "add"},
        {BlendOp::subtract, "subtract"},
        {BlendOp::reverse_subtract, "reverse_subtract"},
        {BlendOp::min, "min"},
        {BlendOp::max, "max"},
    }
);
KALI_ENUM_REGISTER(BlendOp);

enum class BlendFactor : uint8_t {
    zero = static_cast<uint8_t>(gfx::BlendFactor::Zero),
    one = static_cast<uint8_t>(gfx::BlendFactor::One),
    src_color = static_cast<uint8_t>(gfx::BlendFactor::SrcColor),
    inv_src_color = static_cast<uint8_t>(gfx::BlendFactor::InvSrcColor),
    src_alpha = static_cast<uint8_t>(gfx::BlendFactor::SrcAlpha),
    inv_src_alpha = static_cast<uint8_t>(gfx::BlendFactor::InvSrcAlpha),
    dest_alpha = static_cast<uint8_t>(gfx::BlendFactor::DestAlpha),
    inv_dest_alpha = static_cast<uint8_t>(gfx::BlendFactor::InvDestAlpha),
    dest_color = static_cast<uint8_t>(gfx::BlendFactor::DestColor),
    inv_dest_color = static_cast<uint8_t>(gfx::BlendFactor::InvDestColor),
    src_alpha_saturate = static_cast<uint8_t>(gfx::BlendFactor::SrcAlphaSaturate),
    blend_color = static_cast<uint8_t>(gfx::BlendFactor::BlendColor),
    inv_blend_color = static_cast<uint8_t>(gfx::BlendFactor::InvBlendColor),
    secondary_src_color = static_cast<uint8_t>(gfx::BlendFactor::SecondarySrcColor),
    inv_secondary_src_color = static_cast<uint8_t>(gfx::BlendFactor::InvSecondarySrcColor),
    secondary_src_alpha = static_cast<uint8_t>(gfx::BlendFactor::SecondarySrcAlpha),
    inv_secondary_src_alpha = static_cast<uint8_t>(gfx::BlendFactor::InvSecondarySrcAlpha),
};

KALI_ENUM_INFO(
    BlendFactor,
    {
        {BlendFactor::zero, "zero"},
        {BlendFactor::one, "one"},
        {BlendFactor::src_color, "src_color"},
        {BlendFactor::inv_src_color, "inv_src_color"},
        {BlendFactor::src_alpha, "src_alpha"},
        {BlendFactor::inv_src_alpha, "inv_src_alpha"},
        {BlendFactor::dest_alpha, "dest_alpha"},
        {BlendFactor::inv_dest_alpha, "inv_dest_alpha"},
        {BlendFactor::dest_color, "dest_color"},
        {BlendFactor::inv_dest_color, "inv_dest_color"},
        {BlendFactor::src_alpha_saturate, "src_alpha_saturate"},
        {BlendFactor::blend_color, "blend_color"},
        {BlendFactor::inv_blend_color, "inv_blend_color"},
        {BlendFactor::secondary_src_color, "secondary_src_color"},
        {BlendFactor::inv_secondary_src_color, "inv_secondary_src_color"},
        {BlendFactor::secondary_src_alpha, "secondary_src_alpha"},
        {BlendFactor::inv_secondary_src_alpha, "inv_secondary_src_alpha"},
    }
);
KALI_ENUM_REGISTER(BlendFactor);

enum class RenderTargetWriteMask : uint8_t {
    enable_none = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableNone),
    enable_red = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableRed),
    enable_green = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableGreen),
    enable_blue = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableBlue),
    enable_alpha = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableAlpha),
    enable_all = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableAll),
};

KALI_ENUM_CLASS_OPERATORS(RenderTargetWriteMask);
KALI_ENUM_INFO(
    RenderTargetWriteMask,
    {
        {RenderTargetWriteMask::enable_none, "enable_none"},
        {RenderTargetWriteMask::enable_red, "enable_red"},
        {RenderTargetWriteMask::enable_green, "enable_green"},
        {RenderTargetWriteMask::enable_blue, "enable_blue"},
        {RenderTargetWriteMask::enable_alpha, "enable_alpha"},
        {RenderTargetWriteMask::enable_all, "enable_all"},
    }
);
KALI_ENUM_REGISTER(RenderTargetWriteMask);

struct AspectBlendDesc {
    BlendFactor src_factor{BlendFactor::one};
    BlendFactor dst_factor{BlendFactor::zero};
    BlendOp op{BlendOp::add};
};

struct TargetBlendDesc {
    AspectBlendDesc color;
    AspectBlendDesc alpha;
    bool enable_blend{false};
    LogicOp logic_op{LogicOp::no_op};
    RenderTargetWriteMask write_mask{RenderTargetWriteMask::enable_all};
};

struct BlendDesc {
    // TargetBlendDesc targets[kMaxRenderTargetCount];
    uint32_t target_count{0};

    bool alpha_to_coverage_enable{false};
};


enum class RayTracingPipelineFlags : uint8_t {
    none = static_cast<uint8_t>(gfx::RayTracingPipelineFlags::None),
    skip_triangles = static_cast<uint8_t>(gfx::RayTracingPipelineFlags::SkipTriangles),
    skip_procedurals = static_cast<uint8_t>(gfx::RayTracingPipelineFlags::SkipProcedurals),
};

KALI_ENUM_CLASS_OPERATORS(RayTracingPipelineFlags);
KALI_ENUM_INFO(
    RayTracingPipelineFlags,
    {
        {RayTracingPipelineFlags::none, "none"},
        {RayTracingPipelineFlags::skip_triangles, "skip_triangles"},
        {RayTracingPipelineFlags::skip_procedurals, "skip_procedurals"},
    }
);
KALI_ENUM_REGISTER(RayTracingPipelineFlags);


} // namespace kali
