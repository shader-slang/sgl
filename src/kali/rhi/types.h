#pragma once

#include "core/macros.h"
#include "core/enum.h"

#include <slang-gfx.h>

enum class ComparisonFunc {
    never,
    less,
    equal,
    less_equal,
    greater,
    not_equal,
    greater_equal,
    always,
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

enum class TextureFilteringMode {
    point,
    linear,
};

KALI_ENUM_INFO(
    TextureFilteringMode,
    {
        {TextureFilteringMode::point, "point"},
        {TextureFilteringMode::linear, "linear"},
    }
);
KALI_ENUM_REGISTER(TextureFilteringMode);

enum class TextureAddressingMode {
    wrap,
    clamp_to_edge,
    clamp_to_border,
    mirror_repeat,
    mirror_once,
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

enum class TextureReductionOp {
    average,
    comparison,
    minimum,
    maximum,
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

enum class PrimitiveType {
    point,
    line,
    triangle,
    patch,
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
    keep,
    zero,
    replace,
    increment_saturate,
    decrement_saturate,
    invert,
    increment_wrap,
    decrement_wrap,
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
    solid,
    wireframe,
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
    none,
    front,
    back,
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
    counter_clockwise,
    clockwise,
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
    StencilOp stencilFailOp{StencilOp::keep};
    StencilOp stencilDepthFailOp{StencilOp::keep};
    StencilOp stencilPassOp{StencilOp::keep};
    ComparisonFunc stencilFunc{ComparisonFunc::always};
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

    uint32_t stencilRef = 0;
};

struct RasterizerDesc {
    FillMode fillMode{FillMode::solid};
    CullMode cullMode{CullMode::none};
    FrontFaceMode frontFace{FrontFaceMode::counter_clockwise};
    int32_t depthBias{0};
    float depthBiasClamp{0.0f};
    float slopeScaledDepthBias{0.0f};
    bool depthClipEnable{true};
    bool scissorEnable{false};
    bool multisampleEnable{false};
    bool antialiasedLineEnable{false};
    bool enableConservativeRasterization{false};
    uint32_t forcedSampleCount{0};
};

enum class LogicOp {
    no_op,
};

KALI_ENUM_INFO(
    LogicOp,
    {
        {LogicOp::no_op, "no_op"},
    }
);
KALI_ENUM_REGISTER(LogicOp);

enum class BlendOp {
    add,
    subtract,
    reverse_subtract,
    min,
    max,
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

enum class BlendFactor {
    zero,
    one,
    src_color,
    inv_src_color,
    src_alpha,
    inv_src_alpha,
    dest_alpha,
    inv_dest_alpha,
    dest_color,
    inv_dest_color,
    src_alpha_saturate,
    blend_color,
    inv_blend_color,
    secondary_src_color,
    inv_secondary_src_color,
    secondary_src_alpha,
    inv_secondary_src_alpha,
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

enum class RenderTargetWriteMask {
    enable_none = 0,
    enable_red = 0x01,
    enable_green = 0x02,
    enable_blue = 0x04,
    enable_alpha = 0x08,
    enable_all = 0x0f,
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
