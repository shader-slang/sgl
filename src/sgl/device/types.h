// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/formats.h"

#include "sgl/core/macros.h"
#include "sgl/core/enum.h"

#include "sgl/math/vector_types.h"
#include "sgl/math/matrix_types.h"

#include <slang-gfx.h>

namespace sgl {

/// Represents an address in device memory.
using DeviceAddress = uint64_t;
/// Represents an offset in device memory (in bytes).
using DeviceOffset = uint64_t;
/// Represents a size in device memory (in bytes).
using DeviceSize = uint64_t;

enum CommandQueueType : uint32_t {
    graphics = static_cast<uint32_t>(gfx::ICommandQueue::QueueType::Graphics),
};

SGL_ENUM_INFO(
    CommandQueueType,
    {
        {CommandQueueType::graphics, "graphics"},
    }
);
SGL_ENUM_REGISTER(CommandQueueType);

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

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(ShaderModel);

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

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(ShaderStage);

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

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(ComparisonFunc);

// ----------------------------------------------------------------------------
// Sampler
// ----------------------------------------------------------------------------

enum class TextureFilteringMode : uint32_t {
    point = static_cast<uint32_t>(gfx::TextureFilteringMode::Point),
    linear = static_cast<uint32_t>(gfx::TextureFilteringMode::Linear),
};

SGL_ENUM_INFO(
    TextureFilteringMode,
    {
        {TextureFilteringMode::point, "point"},
        {TextureFilteringMode::linear, "linear"},
    }
);
SGL_ENUM_REGISTER(TextureFilteringMode);

enum class TextureAddressingMode : uint32_t {
    wrap = static_cast<uint32_t>(gfx::TextureAddressingMode::Wrap),
    clamp_to_edge = static_cast<uint32_t>(gfx::TextureAddressingMode::ClampToEdge),
    clamp_to_border = static_cast<uint32_t>(gfx::TextureAddressingMode::ClampToBorder),
    mirror_repeat = static_cast<uint32_t>(gfx::TextureAddressingMode::MirrorRepeat),
    mirror_once = static_cast<uint32_t>(gfx::TextureAddressingMode::MirrorOnce),
};

SGL_ENUM_INFO(
    TextureAddressingMode,
    {
        {TextureAddressingMode::wrap, "wrap"},
        {TextureAddressingMode::clamp_to_edge, "clamp_to_edge"},
        {TextureAddressingMode::clamp_to_border, "clamp_to_border"},
        {TextureAddressingMode::mirror_repeat, "mirror_repeat"},
        {TextureAddressingMode::mirror_once, "mirror_once"},
    }
);
SGL_ENUM_REGISTER(TextureAddressingMode);

enum class TextureReductionOp : uint32_t {
    average = static_cast<uint32_t>(gfx::TextureReductionOp::Average),
    comparison = static_cast<uint32_t>(gfx::TextureReductionOp::Comparison),
    minimum = static_cast<uint32_t>(gfx::TextureReductionOp::Minimum),
    maximum = static_cast<uint32_t>(gfx::TextureReductionOp::Maximum),
};

SGL_ENUM_INFO(
    TextureReductionOp,
    {
        {TextureReductionOp::average, "average"},
        {TextureReductionOp::comparison, "comparison"},
        {TextureReductionOp::minimum, "minimum"},
        {TextureReductionOp::maximum, "maximum"},
    }
);
SGL_ENUM_REGISTER(TextureReductionOp);

// ----------------------------------------------------------------------------
// Graphics
// ----------------------------------------------------------------------------

struct IndirectDispatchArguments {
    uint32_t thread_group_count_x;
    uint32_t thread_group_count_y;
    uint32_t thread_group_count_z;
};

// ----------------------------------------------------------------------------
// Graphics
// ----------------------------------------------------------------------------

struct IndirectDrawArguments {
    uint32_t vertex_count_per_instance{0};
    uint32_t instance_count{1};
    uint32_t start_vertex_location{0};
    uint32_t start_instance_location{0};
};

struct IndirectDrawIndexedArguments {
    uint32_t index_count_per_instance{0};
    uint32_t instance_count{1};
    uint32_t start_index_location{0};
    int32_t base_vertex_location{0};
    uint32_t start_instance_location{0};
};

struct Viewport {
    float x{0.f};
    float y{0.f};
    float width{0.f};
    float height{0.f};
    float min_depth{0.f};
    float max_depth{1.f};
};

static_assert(
    (sizeof(Viewport) == sizeof(gfx::Viewport)) && (offsetof(Viewport, x) == offsetof(gfx::Viewport, originX))
        && (offsetof(Viewport, y) == offsetof(gfx::Viewport, originY))
        && (offsetof(Viewport, width) == offsetof(gfx::Viewport, extentX))
        && (offsetof(Viewport, height) == offsetof(gfx::Viewport, extentY))
        && (offsetof(Viewport, min_depth) == offsetof(gfx::Viewport, minZ))
        && (offsetof(Viewport, max_depth) == offsetof(gfx::Viewport, maxZ)),
    "Viewport struct mismatch"
);

struct ScissorRect {
    int32_t min_x{0};
    int32_t min_y{0};
    int32_t max_x{0};
    int32_t max_y{0};
};

static_assert(
    (sizeof(ScissorRect) == sizeof(gfx::ScissorRect))
        && (offsetof(ScissorRect, min_x) == offsetof(gfx::ScissorRect, minX))
        && (offsetof(ScissorRect, min_y) == offsetof(gfx::ScissorRect, minY))
        && (offsetof(ScissorRect, max_x) == offsetof(gfx::ScissorRect, maxX))
        && (offsetof(ScissorRect, max_y) == offsetof(gfx::ScissorRect, maxY)),
    "ScissorRect struct mismatch"
);

enum class PrimitiveType : uint8_t {
    point = static_cast<uint8_t>(gfx::PrimitiveType::Point),
    line = static_cast<uint8_t>(gfx::PrimitiveType::Line),
    triangle = static_cast<uint8_t>(gfx::PrimitiveType::Triangle),
    patch = static_cast<uint8_t>(gfx::PrimitiveType::Patch),
};

SGL_ENUM_INFO(
    PrimitiveType,
    {
        {PrimitiveType::point, "point"},
        {PrimitiveType::line, "line"},
        {PrimitiveType::triangle, "triangle"},
        {PrimitiveType::patch, "patch"},
    }
);
SGL_ENUM_REGISTER(PrimitiveType);

enum class PrimitiveTopology : uint8_t {
    triangle_list = static_cast<uint8_t>(gfx::PrimitiveTopology::TriangleList),
    triangle_strip = static_cast<uint8_t>(gfx::PrimitiveTopology::TriangleStrip),
    point_list = static_cast<uint8_t>(gfx::PrimitiveTopology::PointList),
    line_list = static_cast<uint8_t>(gfx::PrimitiveTopology::LineList),
    line_strip = static_cast<uint8_t>(gfx::PrimitiveTopology::LineStrip),
};

SGL_ENUM_INFO(
    PrimitiveTopology,
    {
        {PrimitiveTopology::triangle_list, "triangle_list"},
        {PrimitiveTopology::triangle_strip, "triangle_strip"},
        {PrimitiveTopology::point_list, "point_list"},
        {PrimitiveTopology::line_list, "line_list"},
        {PrimitiveTopology::line_strip, "line_strip"},
    }
);
SGL_ENUM_REGISTER(PrimitiveTopology);

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

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(StencilOp);

enum class FillMode : uint8_t {
    solid = static_cast<uint8_t>(gfx::FillMode::Solid),
    wireframe = static_cast<uint8_t>(gfx::FillMode::Wireframe),
};

SGL_ENUM_INFO(
    FillMode,
    {
        {FillMode::solid, "solid"},
        {FillMode::wireframe, "wireframe"},
    }
);
SGL_ENUM_REGISTER(FillMode);

enum class CullMode : uint8_t {
    none = static_cast<uint8_t>(gfx::CullMode::None),
    front = static_cast<uint8_t>(gfx::CullMode::Front),
    back = static_cast<uint8_t>(gfx::CullMode::Back),
};

SGL_ENUM_INFO(
    CullMode,
    {
        {CullMode::none, "none"},
        {CullMode::front, "front"},
        {CullMode::back, "back"},
    }
);
SGL_ENUM_REGISTER(CullMode);

enum class FrontFaceMode : uint8_t {
    counter_clockwise = static_cast<uint8_t>(gfx::FrontFaceMode::CounterClockwise),
    clockwise = static_cast<uint8_t>(gfx::FrontFaceMode::Clockwise),
};

SGL_ENUM_INFO(
    FrontFaceMode,
    {
        {FrontFaceMode::counter_clockwise, "counter_clockwise"},
        {FrontFaceMode::clockwise, "clockwise"},
    }
);
SGL_ENUM_REGISTER(FrontFaceMode);

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

SGL_ENUM_INFO(
    LogicOp,
    {
        {LogicOp::no_op, "no_op"},
    }
);
SGL_ENUM_REGISTER(LogicOp);

enum class BlendOp : uint8_t {
    add = static_cast<uint8_t>(gfx::BlendOp::Add),
    subtract = static_cast<uint8_t>(gfx::BlendOp::Subtract),
    reverse_subtract = static_cast<uint8_t>(gfx::BlendOp::ReverseSubtract),
    min = static_cast<uint8_t>(gfx::BlendOp::Min),
    max = static_cast<uint8_t>(gfx::BlendOp::Max),
};

SGL_ENUM_INFO(
    BlendOp,
    {
        {BlendOp::add, "add"},
        {BlendOp::subtract, "subtract"},
        {BlendOp::reverse_subtract, "reverse_subtract"},
        {BlendOp::min, "min"},
        {BlendOp::max, "max"},
    }
);
SGL_ENUM_REGISTER(BlendOp);

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

SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(BlendFactor);

enum class RenderTargetWriteMask : uint8_t {
    enable_none = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableNone),
    enable_red = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableRed),
    enable_green = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableGreen),
    enable_blue = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableBlue),
    enable_alpha = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableAlpha),
    enable_all = static_cast<uint8_t>(gfx::RenderTargetWriteMask::EnableAll),
};

SGL_ENUM_CLASS_OPERATORS(RenderTargetWriteMask);
SGL_ENUM_INFO(
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
SGL_ENUM_REGISTER(RenderTargetWriteMask);

struct AspectBlendDesc {
    BlendFactor src_factor{BlendFactor::one};
    BlendFactor dst_factor{BlendFactor::zero};
    BlendOp op{BlendOp::add};
};

struct TargetBlendDesc {
    bool enable_blend{false};
    AspectBlendDesc color;
    AspectBlendDesc alpha;
    LogicOp logic_op{LogicOp::no_op};
    RenderTargetWriteMask write_mask{RenderTargetWriteMask::enable_all};
};

struct BlendDesc {
    std::vector<TargetBlendDesc> targets;
    bool alpha_to_coverage_enable{false};
};

// ----------------------------------------------------------------------------
// Queries
// ----------------------------------------------------------------------------

enum class QueryType : uint32_t {
    timestamp = static_cast<uint32_t>(gfx::QueryType::Timestamp),
    acceleration_structure_compacted_size = static_cast<uint32_t>(gfx::QueryType::AccelerationStructureCompactedSize),
    acceleration_structure_serialized_size = static_cast<uint32_t>(gfx::QueryType::AccelerationStructureSerializedSize),
    acceleration_structure_current_size = static_cast<uint32_t>(gfx::QueryType::AccelerationStructureCurrentSize),
};

SGL_ENUM_INFO(
    QueryType,
    {
        {QueryType::timestamp, "timestamp"},
        {QueryType::acceleration_structure_compacted_size, "acceleration_structure_compacted_size"},
        {QueryType::acceleration_structure_serialized_size, "acceleration_structure_serialized_size"},
        {QueryType::acceleration_structure_current_size, "acceleration_structure_current_size"},
    }
);
SGL_ENUM_REGISTER(QueryType);

// ----------------------------------------------------------------------------
// RayTracing
// ----------------------------------------------------------------------------


enum class RayTracingPipelineFlags : uint8_t {
    none = static_cast<uint8_t>(gfx::RayTracingPipelineFlags::None),
    skip_triangles = static_cast<uint8_t>(gfx::RayTracingPipelineFlags::SkipTriangles),
    skip_procedurals = static_cast<uint8_t>(gfx::RayTracingPipelineFlags::SkipProcedurals),
};

SGL_ENUM_CLASS_OPERATORS(RayTracingPipelineFlags);
SGL_ENUM_INFO(
    RayTracingPipelineFlags,
    {
        {RayTracingPipelineFlags::none, "none"},
        {RayTracingPipelineFlags::skip_triangles, "skip_triangles"},
        {RayTracingPipelineFlags::skip_procedurals, "skip_procedurals"},
    }
);
SGL_ENUM_REGISTER(RayTracingPipelineFlags);

enum class RayTracingGeometryType : uint32_t {
    triangles = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryType::Triangles),
    procedural_primitives = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryType::ProcedurePrimitives),
};

SGL_ENUM_INFO(
    RayTracingGeometryType,
    {
        {RayTracingGeometryType::triangles, "triangles"},
        {RayTracingGeometryType::procedural_primitives, "procedural_primitives"},
    }
);
SGL_ENUM_REGISTER(RayTracingGeometryType);

enum class RayTracingGeometryFlags : uint32_t {
    none = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryFlags::None),
    opaque = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryFlags::Opaque),
    no_duplicate_any_hit_invocation
    = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryFlags::NoDuplicateAnyHitInvocation),
};

SGL_ENUM_CLASS_OPERATORS(RayTracingGeometryFlags);
SGL_ENUM_INFO(
    RayTracingGeometryFlags,
    {
        {RayTracingGeometryFlags::none, "none"},
        {RayTracingGeometryFlags::opaque, "opaque"},
        {RayTracingGeometryFlags::no_duplicate_any_hit_invocation, "no_duplicate_any_hit_invocation"},
    }
);
SGL_ENUM_REGISTER(RayTracingGeometryFlags);

struct RayTracingTrianglesDesc {
    DeviceAddress transform3x4;
    Format index_format;
    Format vertex_format;
    uint32_t index_count;
    uint32_t vertex_count;
    DeviceAddress index_data;
    DeviceAddress vertex_data;
    DeviceSize vertex_stride;
};
// clang-format off
static_assert(sizeof(RayTracingTrianglesDesc) == sizeof(gfx::IAccelerationStructure::TriangleDesc));
static_assert(offsetof(RayTracingTrianglesDesc, transform3x4) == offsetof(gfx::IAccelerationStructure::TriangleDesc, transform3x4));
static_assert(offsetof(RayTracingTrianglesDesc, index_format) == offsetof(gfx::IAccelerationStructure::TriangleDesc, indexFormat));
static_assert(offsetof(RayTracingTrianglesDesc, vertex_format) == offsetof(gfx::IAccelerationStructure::TriangleDesc, vertexFormat));
static_assert(offsetof(RayTracingTrianglesDesc, index_count) == offsetof(gfx::IAccelerationStructure::TriangleDesc, indexCount));
static_assert(offsetof(RayTracingTrianglesDesc, vertex_count) == offsetof(gfx::IAccelerationStructure::TriangleDesc, vertexCount));
static_assert(offsetof(RayTracingTrianglesDesc, index_data) == offsetof(gfx::IAccelerationStructure::TriangleDesc, indexData));
static_assert(offsetof(RayTracingTrianglesDesc, vertex_data) == offsetof(gfx::IAccelerationStructure::TriangleDesc, vertexData));
static_assert(offsetof(RayTracingTrianglesDesc, vertex_stride) == offsetof(gfx::IAccelerationStructure::TriangleDesc, vertexStride));
// clang-format on

struct RayTracingAABB {
    float3 min;
    float3 max;
};
static_assert(sizeof(RayTracingAABB) == 24);
static_assert(offsetof(RayTracingAABB, min) == offsetof(gfx::IAccelerationStructure::ProceduralAABB, minX));
static_assert(offsetof(RayTracingAABB, max) == offsetof(gfx::IAccelerationStructure::ProceduralAABB, maxX));

struct RayTracingAABBsDesc {
    /// Number of AABBs.
    uint32_t count;

    /// Pointer to an array of `RayTracingAABB` values in device memory.
    DeviceAddress data;

    /// Stride in bytes of the AABB values array.
    DeviceSize stride;
};
// clang-format off
static_assert(sizeof(RayTracingAABBsDesc) == sizeof(gfx::IAccelerationStructure::ProceduralAABBDesc));
static_assert(offsetof(RayTracingAABBsDesc, count) == offsetof(gfx::IAccelerationStructure::ProceduralAABBDesc, count));
static_assert(offsetof(RayTracingAABBsDesc, data) == offsetof(gfx::IAccelerationStructure::ProceduralAABBDesc, data));
static_assert(offsetof(RayTracingAABBsDesc, stride) == offsetof(gfx::IAccelerationStructure::ProceduralAABBDesc, stride));
// clang-format on

struct RayTracingGeometryDesc {
    RayTracingGeometryType type;
    RayTracingGeometryFlags flags;
    union {
        RayTracingTrianglesDesc triangles;
        RayTracingAABBsDesc aabbs;
    };
};
// clang-format off
static_assert(sizeof(RayTracingGeometryDesc) == sizeof(gfx::IAccelerationStructure::GeometryDesc));
static_assert(offsetof(RayTracingGeometryDesc, type) == offsetof(gfx::IAccelerationStructure::GeometryDesc, type));
static_assert(offsetof(RayTracingGeometryDesc, flags) == offsetof(gfx::IAccelerationStructure::GeometryDesc, flags));
static_assert(offsetof(RayTracingGeometryDesc, triangles) == offsetof(gfx::IAccelerationStructure::GeometryDesc, content));
static_assert(offsetof(RayTracingGeometryDesc, aabbs) == offsetof(gfx::IAccelerationStructure::GeometryDesc, content));
// clang-format on

enum class RayTracingInstanceFlags : uint32_t {
    none = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::None),
    triangle_facing_cull_disable
    = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::TriangleFacingCullDisable),
    triangle_front_counter_clockwise
    = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::TriangleFrontCounterClockwise),
    force_opaque = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::ForceOpaque),
    no_opaque = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::NoOpaque),
};

SGL_ENUM_CLASS_OPERATORS(RayTracingInstanceFlags);
SGL_ENUM_INFO(
    RayTracingInstanceFlags,
    {
        {RayTracingInstanceFlags::none, "none"},
        {RayTracingInstanceFlags::triangle_facing_cull_disable, "triangle_facing_cull_disable"},
        {RayTracingInstanceFlags::triangle_front_counter_clockwise, "triangle_front_counter_clockwise"},
        {RayTracingInstanceFlags::force_opaque, "force_opaque"},
        {RayTracingInstanceFlags::no_opaque, "no_opaque"},
    }
);
SGL_ENUM_REGISTER(RayTracingInstanceFlags);

struct RayTracingInstanceDesc {
    float3x4 transform;
    uint32_t instance_id : 24;
    uint32_t instance_mask : 8;
    uint32_t instance_contribution_to_hit_group_index : 24;
    uint32_t flags_ : 8; // Combination of RayTracingInstanceFlags values.
    DeviceAddress acceleration_structure;

    RayTracingInstanceFlags flags() const { return static_cast<RayTracingInstanceFlags>(flags_); }
    void set_flags(RayTracingInstanceFlags flags) { flags_ = static_cast<uint32_t>(flags); }
};
static_assert(sizeof(RayTracingInstanceDesc) == 64);

enum class AccelerationStructureCopyMode : uint32_t {
    clone = static_cast<uint32_t>(gfx::AccelerationStructureCopyMode::Clone),
    compact = static_cast<uint32_t>(gfx::AccelerationStructureCopyMode::Compact),
};

SGL_ENUM_INFO(
    AccelerationStructureCopyMode,
    {
        {AccelerationStructureCopyMode::clone, "clone"},
        {AccelerationStructureCopyMode::compact, "compact"},
    }
);
SGL_ENUM_REGISTER(AccelerationStructureCopyMode);

enum class AccelerationStructureKind : uint32_t {
    top_level = static_cast<uint32_t>(gfx::IAccelerationStructure::Kind::TopLevel),
    bottom_level = static_cast<uint32_t>(gfx::IAccelerationStructure::Kind::BottomLevel),
};

SGL_ENUM_INFO(
    AccelerationStructureKind,
    {
        {AccelerationStructureKind::top_level, "top_level"},
        {AccelerationStructureKind::bottom_level, "bottom_level"},
    }
);
SGL_ENUM_REGISTER(AccelerationStructureKind);

enum class AccelerationStructureBuildFlags : uint32_t {
    none = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::None),
    allow_update = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::AllowUpdate),
    allow_compaction = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::AllowCompaction),
    prefer_fast_trace = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::PreferFastTrace),
    prefer_fast_build = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::PreferFastBuild),
    minimize_memory = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::MinimizeMemory),
    perform_update = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::PerformUpdate),
};

SGL_ENUM_CLASS_OPERATORS(AccelerationStructureBuildFlags);
SGL_ENUM_INFO(
    AccelerationStructureBuildFlags,
    {
        {AccelerationStructureBuildFlags::none, "none"},
        {AccelerationStructureBuildFlags::allow_update, "allow_update"},
        {AccelerationStructureBuildFlags::allow_compaction, "allow_compaction"},
        {AccelerationStructureBuildFlags::prefer_fast_trace, "prefer_fast_trace"},
        {AccelerationStructureBuildFlags::prefer_fast_build, "prefer_fast_build"},
        {AccelerationStructureBuildFlags::minimize_memory, "minimize_memory"},
        {AccelerationStructureBuildFlags::perform_update, "perform_update"},
    }
);
SGL_ENUM_REGISTER(AccelerationStructureBuildFlags);


struct AccelerationStructurePrebuildInfo {
    DeviceSize result_data_max_size;
    DeviceSize scratch_data_size;
    DeviceSize update_scratch_data_size;
};

struct AccelerationStructureBuildInputs {
    AccelerationStructureKind kind;

    AccelerationStructureBuildFlags flags;

    uint32_t desc_count{0};

    /// Array of `RayTracingInstanceDesc` values in device memory.
    /// Used when `kind` is `top_level`.
    DeviceAddress instance_descs{0};

    /// Array of `RayTracingGeometryDesc` values.
    /// Used when `kind` is `bottom_level`.
    const RayTracingGeometryDesc* geometry_descs{nullptr};
};
// clang-format off
static_assert(sizeof(AccelerationStructureBuildInputs) == sizeof(gfx::IAccelerationStructure::BuildInputs));
static_assert(offsetof(AccelerationStructureBuildInputs, kind) == offsetof(gfx::IAccelerationStructure::BuildInputs, kind));
static_assert(offsetof(AccelerationStructureBuildInputs, flags) == offsetof(gfx::IAccelerationStructure::BuildInputs, flags));
static_assert(offsetof(AccelerationStructureBuildInputs, desc_count) == offsetof(gfx::IAccelerationStructure::BuildInputs, descCount));
static_assert(offsetof(AccelerationStructureBuildInputs, instance_descs) == offsetof(gfx::IAccelerationStructure::BuildInputs, instanceDescs));
static_assert(offsetof(AccelerationStructureBuildInputs, geometry_descs) == offsetof(gfx::IAccelerationStructure::BuildInputs, geometryDescs));
// clang-format on


} // namespace sgl
