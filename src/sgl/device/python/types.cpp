// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/types.h"

namespace sgl {

SGL_DICT_TO_DESC_BEGIN(DrawArguments)
SGL_DICT_TO_DESC_FIELD(vertex_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(instance_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(start_vertex_location, uint32_t)
SGL_DICT_TO_DESC_FIELD(start_instance_location, uint32_t)
SGL_DICT_TO_DESC_FIELD(start_index_location, uint32_t)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(Viewport)
SGL_DICT_TO_DESC_FIELD(x, float)
SGL_DICT_TO_DESC_FIELD(y, float)
SGL_DICT_TO_DESC_FIELD(width, float)
SGL_DICT_TO_DESC_FIELD(height, float)
SGL_DICT_TO_DESC_FIELD(min_depth, float)
SGL_DICT_TO_DESC_FIELD(max_depth, float)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(ScissorRect)
SGL_DICT_TO_DESC_FIELD(min_x, int32_t)
SGL_DICT_TO_DESC_FIELD(min_y, int32_t)
SGL_DICT_TO_DESC_FIELD(max_x, int32_t)
SGL_DICT_TO_DESC_FIELD(max_y, int32_t)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(AspectBlendDesc)
SGL_DICT_TO_DESC_FIELD(src_factor, BlendFactor)
SGL_DICT_TO_DESC_FIELD(dst_factor, BlendFactor)
SGL_DICT_TO_DESC_FIELD(op, BlendOp)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(ColorTargetDesc)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD(color, AspectBlendDesc)
SGL_DICT_TO_DESC_FIELD(alpha, AspectBlendDesc)
SGL_DICT_TO_DESC_FIELD(enable_blend, bool)
SGL_DICT_TO_DESC_FIELD(logic_op, LogicOp)
SGL_DICT_TO_DESC_FIELD(write_mask, RenderTargetWriteMask)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(MultisampleDesc)
SGL_DICT_TO_DESC_FIELD(sample_count, uint32_t)
SGL_DICT_TO_DESC_FIELD(sample_mask, uint32_t)
SGL_DICT_TO_DESC_FIELD(alpha_to_coverage_enable, bool)
SGL_DICT_TO_DESC_FIELD(alpha_to_one_enable, bool)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(DepthStencilOpDesc)
SGL_DICT_TO_DESC_FIELD(stencil_fail_op, StencilOp)
SGL_DICT_TO_DESC_FIELD(stencil_depth_fail_op, StencilOp)
SGL_DICT_TO_DESC_FIELD(stencil_pass_op, StencilOp)
SGL_DICT_TO_DESC_FIELD(stencil_func, ComparisonFunc)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(DepthStencilDesc)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD(depth_test_enable, bool)
SGL_DICT_TO_DESC_FIELD(depth_write_enable, bool)
SGL_DICT_TO_DESC_FIELD(depth_func, ComparisonFunc)
SGL_DICT_TO_DESC_FIELD(stencil_enable, bool)
SGL_DICT_TO_DESC_FIELD(stencil_read_mask, uint32_t)
SGL_DICT_TO_DESC_FIELD(stencil_write_mask, uint32_t)
SGL_DICT_TO_DESC_FIELD(front_face, DepthStencilOpDesc)
SGL_DICT_TO_DESC_FIELD(back_face, DepthStencilOpDesc)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(RasterizerDesc)
SGL_DICT_TO_DESC_FIELD(fill_mode, FillMode)
SGL_DICT_TO_DESC_FIELD(cull_mode, CullMode)
SGL_DICT_TO_DESC_FIELD(front_face, FrontFaceMode)
SGL_DICT_TO_DESC_FIELD(depth_bias, int32_t)
SGL_DICT_TO_DESC_FIELD(depth_bias_clamp, float)
SGL_DICT_TO_DESC_FIELD(slope_scaled_depth_bias, float)
SGL_DICT_TO_DESC_FIELD(depth_clip_enable, bool)
SGL_DICT_TO_DESC_FIELD(scissor_enable, bool)
SGL_DICT_TO_DESC_FIELD(multisample_enable, bool)
SGL_DICT_TO_DESC_FIELD(antialiased_line_enable, bool)
SGL_DICT_TO_DESC_FIELD(enable_conservative_rasterization, bool)
SGL_DICT_TO_DESC_FIELD(forced_sample_count, uint32_t)
SGL_DICT_TO_DESC_END()

} // namespace sgl

namespace sgl {

#if 0 // TODO(slang-rhi)
struct PyAccelerationStructureBuildInputs : AccelerationStructureBuildInputs {
    std::vector<RayTracingGeometryDesc> geometry_descs_data;
};
#endif

} // namespace sgl

SGL_PY_EXPORT(device_types)
{
    using namespace sgl;

    nb::sgl_enum<CommandQueueType>(m, "CommandQueueType");

    nb::sgl_enum<ShaderModel>(m, "ShaderModel", nb::is_arithmetic());
    nb::sgl_enum<ShaderStage>(m, "ShaderStage");

    nb::sgl_enum<ComparisonFunc>(m, "ComparisonFunc");

    // ------------------------------------------------------------------------
    // Sampler
    // ------------------------------------------------------------------------

    nb::sgl_enum<TextureFilteringMode>(m, "TextureFilteringMode");
    nb::sgl_enum<TextureAddressingMode>(m, "TextureAddressingMode");
    nb::sgl_enum<TextureReductionOp>(m, "TextureReductionOp");

    // ------------------------------------------------------------------------
    // Graphics
    // ------------------------------------------------------------------------

    nb::class_<DrawArguments>(m, "DrawArguments", D_NA(DrawArguments))
        .def(nb::init<>())
        .def(
            "__init__",
            [](DrawArguments* self, nb::dict dict) { new (self) DrawArguments(dict_to_DrawArguments(dict)); }
        )
        .def_rw("vertex_count", &DrawArguments::vertex_count, D_NA(DrawArguments, vertex_count))
        .def_rw("instance_count", &DrawArguments::instance_count, D_NA(DrawArguments, instance_count))
        .def_rw(
            "start_vertex_location",
            &DrawArguments::start_vertex_location,
            D_NA(DrawArguments, start_vertex_location)
        )
        .def_rw(
            "start_instance_location",
            &DrawArguments::start_instance_location,
            D_NA(DrawArguments, start_instance_location)
        )
        .def_rw(
            "start_index_location",
            &DrawArguments::start_index_location,
            D_NA(DrawArguments, start_index_location)
        );
    nb::implicitly_convertible<nb::dict, DrawArguments>();

    nb::class_<Viewport>(m, "Viewport", D(Viewport))
        .def(nb::init<>())
        .def("__init__", [](Viewport* self, nb::dict dict) { new (self) Viewport(dict_to_Viewport(dict)); })
        .def_static("from_size", &Viewport::from_size, "width"_a, "height"_a, D_NA(Viewport, from_size))
        .def_rw("x", &Viewport::x, D(Viewport, x))
        .def_rw("y", &Viewport::y, D(Viewport, y))
        .def_rw("width", &Viewport::width, D(Viewport, width))
        .def_rw("height", &Viewport::height, D(Viewport, height))
        .def_rw("min_depth", &Viewport::min_depth, D(Viewport, min_depth))
        .def_rw("max_depth", &Viewport::max_depth, D(Viewport, max_depth))
        .def("__repr__", &Viewport::to_string);
    nb::implicitly_convertible<nb::dict, Viewport>();

    nb::class_<ScissorRect>(m, "ScissorRect", D(ScissorRect))
        .def(nb::init<>())
        .def("__init__", [](ScissorRect* self, nb::dict dict) { new (self) ScissorRect(dict_to_ScissorRect(dict)); })
        .def_static("from_size", &ScissorRect::from_size, "width"_a, "height"_a, D_NA(ScissorRect, from_size))
        .def_rw("min_x", &ScissorRect::min_x, D(ScissorRect, min_x))
        .def_rw("min_y", &ScissorRect::min_y, D(ScissorRect, min_y))
        .def_rw("max_x", &ScissorRect::max_x, D(ScissorRect, max_x))
        .def_rw("max_y", &ScissorRect::max_y, D(ScissorRect, max_y))
        .def("__repr__", &ScissorRect::to_string);
    nb::implicitly_convertible<nb::dict, ScissorRect>();

    nb::sgl_enum<IndexFormat>(m, "IndexFormat");
    nb::sgl_enum<PrimitiveTopology>(m, "PrimitiveTopology");
    nb::sgl_enum<LoadOp>(m, "LoadOp");
    nb::sgl_enum<StoreOp>(m, "StoreOp");
    nb::sgl_enum<StencilOp>(m, "StencilOp");
    nb::sgl_enum<FillMode>(m, "FillMode");
    nb::sgl_enum<CullMode>(m, "CullMode");
    nb::sgl_enum<FrontFaceMode>(m, "FrontFaceMode");
    nb::sgl_enum<LogicOp>(m, "LogicOp");
    nb::sgl_enum<BlendOp>(m, "BlendOp");
    nb::sgl_enum<BlendFactor>(m, "BlendFactor");
    nb::sgl_enum_flags<RenderTargetWriteMask>(m, "RenderTargetWriteMask");

    nb::class_<AspectBlendDesc>(m, "AspectBlendDesc", D(AspectBlendDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](AspectBlendDesc* self, nb::dict dict) { new (self) AspectBlendDesc(dict_to_AspectBlendDesc(dict)); }
        )
        .def_rw("src_factor", &AspectBlendDesc::src_factor, D(AspectBlendDesc, src_factor))
        .def_rw("dst_factor", &AspectBlendDesc::dst_factor, D(AspectBlendDesc, dst_factor))
        .def_rw("op", &AspectBlendDesc::op, D(AspectBlendDesc, op));
    nb::implicitly_convertible<nb::dict, AspectBlendDesc>();

    nb::class_<ColorTargetDesc>(m, "ColorTargetDesc", D_NA(ColorTargetDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](ColorTargetDesc* self, nb::dict dict) { new (self) ColorTargetDesc(dict_to_ColorTargetDesc(dict)); }
        )
        .def_rw("format", &ColorTargetDesc::format, D_NA(ColorTargetDesc, format))
        .def_rw("write_mask", &ColorTargetDesc::write_mask, D_NA(ColorTargetDesc, write_mask))
        .def_rw("enable_blend", &ColorTargetDesc::enable_blend, D_NA(ColorTargetDesc, enable_blend))
        .def_rw("logic_op", &ColorTargetDesc::logic_op, D_NA(ColorTargetDesc, logic_op))
        .def_rw("write_mask", &ColorTargetDesc::write_mask, D_NA(ColorTargetDesc, write_mask));
    nb::implicitly_convertible<nb::dict, ColorTargetDesc>();

    nb::class_<MultisampleDesc>(m, "MultisampleDesc", D_NA(MultisampleDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](MultisampleDesc* self, nb::dict dict) { new (self) MultisampleDesc(dict_to_MultisampleDesc(dict)); }
        )
        .def_rw("sample_count", &MultisampleDesc::sample_count, D_NA(MultisampleDesc, sample_count))
        .def_rw("sample_mask", &MultisampleDesc::sample_mask, D_NA(MultisampleDesc, sample_mask))
        .def_rw(
            "alpha_to_coverage_enable",
            &MultisampleDesc::alpha_to_coverage_enable,
            D_NA(MultisampleDesc, alpha_to_coverage_enable)
        )
        .def_rw(
            "alpha_to_one_enable",
            &MultisampleDesc::alpha_to_one_enable,
            D_NA(MultisampleDesc, alpha_to_one_enable)
        );
    nb::implicitly_convertible<nb::dict, MultisampleDesc>();

    nb::class_<DepthStencilOpDesc>(m, "DepthStencilOpDesc", D(DepthStencilOpDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](DepthStencilOpDesc* self, nb::dict dict)
            { new (self) DepthStencilOpDesc(dict_to_DepthStencilOpDesc(dict)); }
        )
        .def_rw("stencil_fail_op", &DepthStencilOpDesc::stencil_fail_op, D(DepthStencilOpDesc, stencil_fail_op))
        .def_rw(
            "stencil_depth_fail_op",
            &DepthStencilOpDesc::stencil_depth_fail_op,
            D(DepthStencilOpDesc, stencil_depth_fail_op)
        )
        .def_rw("stencil_pass_op", &DepthStencilOpDesc::stencil_pass_op, D(DepthStencilOpDesc, stencil_pass_op))
        .def_rw("stencil_func", &DepthStencilOpDesc::stencil_func, D(DepthStencilOpDesc, stencil_func));
    nb::implicitly_convertible<nb::dict, DepthStencilOpDesc>();

    nb::class_<DepthStencilDesc>(m, "DepthStencilDesc", D_NA(DepthStencilDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](DepthStencilDesc* self, nb::dict dict) { new (self) DepthStencilDesc(dict_to_DepthStencilDesc(dict)); }
        )
        .def_rw("format", &DepthStencilDesc::format, D_NA(DepthStencilDesc, format))
        .def_rw("depth_test_enable", &DepthStencilDesc::depth_test_enable, D_NA(DepthStencilDesc, depth_test_enable))
        .def_rw("depth_write_enable", &DepthStencilDesc::depth_write_enable, D_NA(DepthStencilDesc, depth_write_enable))
        .def_rw("depth_func", &DepthStencilDesc::depth_func, D_NA(DepthStencilDesc, depth_func))
        .def_rw("stencil_enable", &DepthStencilDesc::stencil_enable, D_NA(DepthStencilDesc, stencil_enable))
        .def_rw("stencil_read_mask", &DepthStencilDesc::stencil_read_mask, D_NA(DepthStencilDesc, stencil_read_mask))
        .def_rw("stencil_write_mask", &DepthStencilDesc::stencil_write_mask, D_NA(DepthStencilDesc, stencil_write_mask))
        .def_rw("front_face", &DepthStencilDesc::front_face, D_NA(DepthStencilDesc, front_face))
        .def_rw("back_face", &DepthStencilDesc::back_face, D_NA(DepthStencilDesc, back_face));
    nb::implicitly_convertible<nb::dict, DepthStencilDesc>();

    nb::class_<RasterizerDesc>(m, "RasterizerDesc", D_NA(RasterizerDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](RasterizerDesc* self, nb::dict dict) { new (self) RasterizerDesc(dict_to_RasterizerDesc(dict)); }
        )
        .def_rw("fill_mode", &RasterizerDesc::fill_mode, D_NA(RasterizerDesc, fill_mode))
        .def_rw("cull_mode", &RasterizerDesc::cull_mode, D_NA(RasterizerDesc, cull_mode))
        .def_rw("front_face", &RasterizerDesc::front_face, D_NA(RasterizerDesc, front_face))
        .def_rw("depth_bias", &RasterizerDesc::depth_bias, D_NA(RasterizerDesc, depth_bias))
        .def_rw("depth_bias_clamp", &RasterizerDesc::depth_bias_clamp, D_NA(RasterizerDesc, depth_bias_clamp))
        .def_rw(
            "slope_scaled_depth_bias",
            &RasterizerDesc::slope_scaled_depth_bias,
            D_NA(RasterizerDesc, slope_scaled_depth_bias)
        )
        .def_rw("depth_clip_enable", &RasterizerDesc::depth_clip_enable, D_NA(RasterizerDesc, depth_clip_enable))
        .def_rw("scissor_enable", &RasterizerDesc::scissor_enable, D_NA(RasterizerDesc, scissor_enable))
        .def_rw("multisample_enable", &RasterizerDesc::multisample_enable, D_NA(RasterizerDesc, multisample_enable))
        .def_rw(
            "antialiased_line_enable",
            &RasterizerDesc::antialiased_line_enable,
            D_NA(RasterizerDesc, antialiased_line_enable)
        )
        .def_rw(
            "enable_conservative_rasterization",
            &RasterizerDesc::enable_conservative_rasterization,
            D_NA(RasterizerDesc, enable_conservative_rasterization)
        )
        .def_rw("forced_sample_count", &RasterizerDesc::forced_sample_count, D_NA(RasterizerDesc, forced_sample_count));
    nb::implicitly_convertible<nb::dict, RasterizerDesc>();

    // ------------------------------------------------------------------------
    // Queries
    // ------------------------------------------------------------------------

    nb::sgl_enum<QueryType>(m, "QueryType");

    // ------------------------------------------------------------------------
    // Raytracing
    // ------------------------------------------------------------------------

    nb::sgl_enum_flags<RayTracingPipelineFlags>(m, "RayTracingPipelineFlags");
}
