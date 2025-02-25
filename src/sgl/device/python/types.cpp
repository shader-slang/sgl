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

SGL_DICT_TO_DESC_BEGIN(ColorTargetState)
SGL_DICT_TO_DESC_FIELD(format, Format)
SGL_DICT_TO_DESC_FIELD_DICT(color, AspectBlendDesc)
SGL_DICT_TO_DESC_FIELD_DICT(alpha, AspectBlendDesc)
SGL_DICT_TO_DESC_FIELD(enable_blend, bool)
SGL_DICT_TO_DESC_FIELD(logic_op, LogicOp)
SGL_DICT_TO_DESC_FIELD(write_mask, RenderTargetWriteMask)
SGL_DICT_TO_DESC_END()

SGL_DICT_TO_DESC_BEGIN(MultisampleState)
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

SGL_DICT_TO_DESC_BEGIN(DepthStencilState)
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

SGL_DICT_TO_DESC_BEGIN(RasterizerState)
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

    nb::class_<Viewport>(m, "Viewport", D(Viewport))
        .def(nb::init<>())
        .def("__init__", [](Viewport* self, nb::dict dict) { new (self) Viewport(dict_to_Viewport(dict)); })
        .def_rw("x", &Viewport::x, D(Viewport, x))
        .def_rw("y", &Viewport::y, D(Viewport, y))
        .def_rw("width", &Viewport::width, D(Viewport, width))
        .def_rw("height", &Viewport::height, D(Viewport, height))
        .def_rw("min_depth", &Viewport::min_depth, D(Viewport, min_depth))
        .def_rw("max_depth", &Viewport::max_depth, D(Viewport, max_depth));
    nb::implicitly_convertible<nb::dict, Viewport>();

    nb::class_<ScissorRect>(m, "ScissorRect", D(ScissorRect))
        .def(nb::init<>())
        .def("__init__", [](ScissorRect* self, nb::dict dict) { new (self) ScissorRect(dict_to_ScissorRect(dict)); })
        .def_rw("min_x", &ScissorRect::min_x, D(ScissorRect, min_x))
        .def_rw("min_y", &ScissorRect::min_y, D(ScissorRect, min_y))
        .def_rw("max_x", &ScissorRect::max_x, D(ScissorRect, max_x))
        .def_rw("max_y", &ScissorRect::max_y, D(ScissorRect, max_y));
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

    nb::class_<ColorTargetState>(m, "ColorTargetState", D_NA(ColorTargetState))
        .def(nb::init<>())
        .def(
            "__init__",
            [](ColorTargetState* self, nb::dict dict) { new (self) ColorTargetState(dict_to_ColorTargetState(dict)); }
        )
        .def_rw("format", &ColorTargetState::format, D_NA(ColorTargetState, format))
        .def_rw("write_mask", &ColorTargetState::write_mask, D_NA(ColorTargetState, write_mask))
        .def_rw("enable_blend", &ColorTargetState::enable_blend, D_NA(ColorTargetState, enable_blend))
        .def_rw("logic_op", &ColorTargetState::logic_op, D_NA(ColorTargetState, logic_op))
        .def_rw("write_mask", &ColorTargetState::write_mask, D_NA(ColorTargetState, write_mask));
    nb::implicitly_convertible<nb::dict, ColorTargetState>();

    nb::class_<MultisampleState>(m, "MultisampleState", D_NA(MultisampleState))
        .def(nb::init<>())
        .def(
            "__init__",
            [](MultisampleState* self, nb::dict dict) { new (self) MultisampleState(dict_to_MultisampleState(dict)); }
        )
        .def_rw("sample_count", &MultisampleState::sample_count, D_NA(MultisampleState, sample_count))
        .def_rw("sample_mask", &MultisampleState::sample_mask, D_NA(MultisampleState, sample_mask))
        .def_rw(
            "alpha_to_coverage_enable",
            &MultisampleState::alpha_to_coverage_enable,
            D_NA(MultisampleState, alpha_to_coverage_enable)
        )
        .def_rw(
            "alpha_to_one_enable",
            &MultisampleState::alpha_to_one_enable,
            D_NA(MultisampleState, alpha_to_one_enable)
        );
    nb::implicitly_convertible<nb::dict, MultisampleState>();

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

    nb::class_<DepthStencilState>(m, "DepthStencilState", D_NA(DepthStencilState))
        .def(nb::init<>())
        .def(
            "__init__",
            [](DepthStencilState* self, nb::dict dict)
            { new (self) DepthStencilState(dict_to_DepthStencilState(dict)); }
        )
        .def_rw("format", &DepthStencilState::format, D_NA(DepthStencilState, format))
        .def_rw("depth_test_enable", &DepthStencilState::depth_test_enable, D_NA(DepthStencilState, depth_test_enable))
        .def_rw(
            "depth_write_enable",
            &DepthStencilState::depth_write_enable,
            D_NA(DepthStencilState, depth_write_enable)
        )
        .def_rw("depth_func", &DepthStencilState::depth_func, D_NA(DepthStencilState, depth_func))
        .def_rw("stencil_enable", &DepthStencilState::stencil_enable, D_NA(DepthStencilState, stencil_enable))
        .def_rw("stencil_read_mask", &DepthStencilState::stencil_read_mask, D_NA(DepthStencilState, stencil_read_mask))
        .def_rw(
            "stencil_write_mask",
            &DepthStencilState::stencil_write_mask,
            D_NA(DepthStencilState, stencil_write_mask)
        )
        .def_rw("front_face", &DepthStencilState::front_face, D_NA(DepthStencilState, front_face))
        .def_rw("back_face", &DepthStencilState::back_face, D_NA(DepthStencilState, back_face));
    nb::implicitly_convertible<nb::dict, DepthStencilState>();

    nb::class_<RasterizerState>(m, "RasterizerState", D_NA(RasterizerState))
        .def(nb::init<>())
        .def(
            "__init__",
            [](RasterizerState* self, nb::dict dict) { new (self) RasterizerState(dict_to_RasterizerState(dict)); }
        )
        .def_rw("fill_mode", &RasterizerState::fill_mode, D_NA(RasterizerState, fill_mode))
        .def_rw("cull_mode", &RasterizerState::cull_mode, D_NA(RasterizerState, cull_mode))
        .def_rw("front_face", &RasterizerState::front_face, D_NA(RasterizerState, front_face))
        .def_rw("depth_bias", &RasterizerState::depth_bias, D_NA(RasterizerState, depth_bias))
        .def_rw("depth_bias_clamp", &RasterizerState::depth_bias_clamp, D_NA(RasterizerState, depth_bias_clamp))
        .def_rw(
            "slope_scaled_depth_bias",
            &RasterizerState::slope_scaled_depth_bias,
            D_NA(RasterizerState, slope_scaled_depth_bias)
        )
        .def_rw("depth_clip_enable", &RasterizerState::depth_clip_enable, D_NA(RasterizerState, depth_clip_enable))
        .def_rw("scissor_enable", &RasterizerState::scissor_enable, D_NA(RasterizerState, scissor_enable))
        .def_rw("multisample_enable", &RasterizerState::multisample_enable, D_NA(RasterizerState, multisample_enable))
        .def_rw(
            "antialiased_line_enable",
            &RasterizerState::antialiased_line_enable,
            D_NA(RasterizerState, antialiased_line_enable)
        )
        .def_rw(
            "enable_conservative_rasterization",
            &RasterizerState::enable_conservative_rasterization,
            D_NA(RasterizerState, enable_conservative_rasterization)
        )
        .def_rw(
            "forced_sample_count",
            &RasterizerState::forced_sample_count,
            D_NA(RasterizerState, forced_sample_count)
        );
    nb::implicitly_convertible<nb::dict, RasterizerState>();

    // ------------------------------------------------------------------------
    // Queries
    // ------------------------------------------------------------------------

    nb::sgl_enum<QueryType>(m, "QueryType");

    // ------------------------------------------------------------------------
    // Raytracing
    // ------------------------------------------------------------------------

#if 0 // TODO(slang-rhi)
    nb::sgl_enum_flags<RayTracingPipelineFlags>(m, "RayTracingPipelineFlags");
    nb::sgl_enum<RayTracingGeometryType>(m, "RayTracingGeometryType");
    nb::sgl_enum_flags<RayTracingGeometryFlags>(m, "RayTracingGeometryFlags");
    nb::sgl_enum_flags<RayTracingInstanceFlags>(m, "RayTracingInstanceFlags");

    nb::class_<RayTracingTrianglesDesc>(m, "RayTracingTrianglesDesc", D(RayTracingTrianglesDesc))
        .def(nb::init<>())
        .def_rw("transform3x4", &RayTracingTrianglesDesc::transform3x4, D(RayTracingTrianglesDesc, transform3x4))
        .def_rw("index_format", &RayTracingTrianglesDesc::index_format, D(RayTracingTrianglesDesc, index_format))
        .def_rw("vertex_format", &RayTracingTrianglesDesc::vertex_format, D(RayTracingTrianglesDesc, vertex_format))
        .def_rw("index_count", &RayTracingTrianglesDesc::index_count, D(RayTracingTrianglesDesc, index_count))
        .def_rw("vertex_count", &RayTracingTrianglesDesc::vertex_count, D(RayTracingTrianglesDesc, vertex_count))
        .def_rw("index_data", &RayTracingTrianglesDesc::index_data, D(RayTracingTrianglesDesc, index_data))
        .def_rw("vertex_data", &RayTracingTrianglesDesc::vertex_data, D(RayTracingTrianglesDesc, vertex_data))
        .def_rw("vertex_stride", &RayTracingTrianglesDesc::vertex_stride, D(RayTracingTrianglesDesc, vertex_stride));

    nb::class_<RayTracingAABB>(m, "RayTracingAABB", D(RayTracingAABB))
        .def(nb::init<>())
        .def_rw("min", &RayTracingAABB::min, D(RayTracingAABB, min))
        .def_rw("max", &RayTracingAABB::max, D(RayTracingAABB, max));

    nb::class_<RayTracingAABBsDesc>(m, "RayTracingAABBsDesc", D(RayTracingAABBsDesc))
        .def(nb::init<>())
        .def_rw("count", &RayTracingAABBsDesc::count, D(RayTracingAABBsDesc, count))
        .def_rw("data", &RayTracingAABBsDesc::data, D(RayTracingAABBsDesc, data))
        .def_rw("stride", &RayTracingAABBsDesc::stride, D(RayTracingAABBsDesc, stride));

    nb::class_<RayTracingGeometryDesc>(m, "RayTracingGeometryDesc", D(RayTracingGeometryDesc))
        .def(nb::init<>())
        .def_rw("type", &RayTracingGeometryDesc::type, D(RayTracingGeometryDesc, type))
        .def_rw("flags", &RayTracingGeometryDesc::flags, D(RayTracingGeometryDesc, flags))
        .def_prop_rw(
            "triangles",
            [](RayTracingGeometryDesc& self) -> RayTracingTrianglesDesc&
            {
                SGL_CHECK(self.type == RayTracingGeometryType::triangles, "geometry type is not triangles");
                return self.triangles;
            },
            [](RayTracingGeometryDesc& self, const RayTracingTrianglesDesc& value)
            {
                self.type = RayTracingGeometryType::triangles;
                self.triangles = value;
            },
            nb::rv_policy::reference_internal
        )
        .def_prop_rw(
            "aabbs",
            [](RayTracingGeometryDesc& self) -> RayTracingAABBsDesc&
            {
                SGL_CHECK(
                    self.type == RayTracingGeometryType::procedural_primitives,
                    "geometry type is not proecedural_primitives"
                );
                return self.aabbs;
            },
            [](RayTracingGeometryDesc& self, const RayTracingAABBsDesc& value)
            {
                self.type = RayTracingGeometryType::procedural_primitives;
                self.aabbs = value;
            }
        );

    nb::class_<RayTracingInstanceDesc>(m, "RayTracingInstanceDesc", D(RayTracingInstanceDesc))
        .def(nb::init<>())
        .def_rw("transform", &RayTracingInstanceDesc::transform, D(RayTracingInstanceDesc, transform))
        .def_prop_rw(
            "instance_id",
            [](RayTracingInstanceDesc& self) { return self.instance_id; },
            [](RayTracingInstanceDesc& self, uint32_t value) { self.instance_id = value; },
            D(RayTracingInstanceDesc, instance_id)
        )
        .def_prop_rw(
            "instance_mask",
            [](RayTracingInstanceDesc& self) { return self.instance_mask; },
            [](RayTracingInstanceDesc& self, uint32_t value) { self.instance_mask = value; },
            D(RayTracingInstanceDesc, instance_mask)
        )
        .def_prop_rw(
            "instance_contribution_to_hit_group_index",
            [](RayTracingInstanceDesc& self) { return self.instance_contribution_to_hit_group_index; },
            [](RayTracingInstanceDesc& self, uint32_t value) { self.instance_contribution_to_hit_group_index = value; },
            D(RayTracingInstanceDesc, instance_contribution_to_hit_group_index)
        )
        .def_prop_rw(
            "flags",
            &RayTracingInstanceDesc::flags,
            &RayTracingInstanceDesc::set_flags,
            D(RayTracingInstanceDesc, flags)
        )
        .def_rw(
            "acceleration_structure",
            &RayTracingInstanceDesc::acceleration_structure,
            D(RayTracingInstanceDesc, acceleration_structure)
        )
        .def(
            "to_numpy",
            [](RayTracingInstanceDesc& self)
            {
                size_t shape[1] = {64};
                return nb::ndarray<nb::numpy, const uint8_t, nb::shape<64>>(&self, 1, shape, nb::handle());
            }
        );

    nb::sgl_enum<AccelerationStructureKind>(m, "AccelerationStructureKind");
    nb::sgl_enum_flags<AccelerationStructureBuildFlags>(m, "AccelerationStructureBuildFlags");
    nb::sgl_enum<AccelerationStructureCopyMode>(m, "AccelerationStructureCopyMode");

    nb::class_<AccelerationStructureBuildInputs>(m, "AccelerationStructureBuildInputsBase");

    nb::class_<PyAccelerationStructureBuildInputs, AccelerationStructureBuildInputs>(
        m,
        "AccelerationStructureBuildInputs",
        D(AccelerationStructureBuildInputs)
    )
        .def(nb::init<>())
        .def_rw("kind", &PyAccelerationStructureBuildInputs::kind, D(AccelerationStructureBuildInputs, kind))
        .def_rw("flags", &PyAccelerationStructureBuildInputs::flags, D(AccelerationStructureBuildInputs, flags))
        .def_rw(
            "desc_count",
            &PyAccelerationStructureBuildInputs::desc_count,
            D(AccelerationStructureBuildInputs, desc_count)
        )
        .def_prop_rw(
            "instance_descs",
            [](PyAccelerationStructureBuildInputs& self)
            {
                SGL_CHECK(self.kind == AccelerationStructureKind::top_level, "kind is not top_level");
                return self.instance_descs;
            },
            [](PyAccelerationStructureBuildInputs& self, DeviceAddress instance_descs)
            {
                self.kind = AccelerationStructureKind::top_level;
                self.instance_descs = instance_descs;
            },
            D(AccelerationStructureBuildInputs, instance_descs)
        )
        .def_prop_rw(
            "geometry_descs",
            [](PyAccelerationStructureBuildInputs& self)
            {
                SGL_CHECK(self.kind == AccelerationStructureKind::bottom_level, "kind is not bottom_level");
                return self.geometry_descs_data;
            },
            [](PyAccelerationStructureBuildInputs& self, std::vector<RayTracingGeometryDesc> geometry_descs)
            {
                self.kind = AccelerationStructureKind::bottom_level;
                self.geometry_descs_data = geometry_descs;
                self.geometry_descs = self.geometry_descs_data.data();
                self.desc_count = narrow_cast<uint32_t>(geometry_descs.size());
            },
            D(AccelerationStructureBuildInputs, geometry_descs)
        );
#endif
}
