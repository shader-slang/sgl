// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/types.h"

namespace sgl {
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
} // namespace sgl

namespace sgl {
struct PyAccelerationStructureBuildInputs : AccelerationStructureBuildInputs {
    std::vector<RayTracingGeometryDesc> geometry_descs_data;
};
} // namespace sgl

SGL_PY_EXPORT(device_types)
{
    using namespace sgl;

    nb::sgl_enum<CommandQueueType>(m, "CommandQueueType");

    nb::sgl_enum<ShaderModel>(m, "ShaderModel");
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

    nb::class_<Viewport>(m, "Viewport")
        .def(nb::init<>())
        .def("__init__", [](Viewport* self, nb::dict dict) { new (self) Viewport(dict_to_Viewport(dict)); })
        .def_rw("x", &Viewport::x)
        .def_rw("y", &Viewport::y)
        .def_rw("width", &Viewport::width)
        .def_rw("height", &Viewport::height)
        .def_rw("min_depth", &Viewport::min_depth)
        .def_rw("max_depth", &Viewport::max_depth);
    nb::implicitly_convertible<nb::dict, Viewport>();

    nb::class_<ScissorRect>(m, "ScissorRect")
        .def(nb::init<>())
        .def("__init__", [](ScissorRect* self, nb::dict dict) { new (self) ScissorRect(dict_to_ScissorRect(dict)); })
        .def_rw("min_x", &ScissorRect::min_x)
        .def_rw("min_y", &ScissorRect::min_y)
        .def_rw("max_x", &ScissorRect::max_x)
        .def_rw("max_y", &ScissorRect::max_y);
    nb::implicitly_convertible<nb::dict, ScissorRect>();

    nb::sgl_enum<PrimitiveType>(m, "PrimitiveType");
    nb::sgl_enum<PrimitiveTopology>(m, "PrimitiveTopology");
    nb::sgl_enum<StencilOp>(m, "StencilOp");
    nb::sgl_enum<FillMode>(m, "FillMode");
    nb::sgl_enum<CullMode>(m, "CullMode");
    nb::sgl_enum<FrontFaceMode>(m, "FrontFaceMode");

    nb::class_<DepthStencilOpDesc>(m, "DepthStencilOpDesc")
        .def(nb::init<>())
        .def_rw("stencil_fail_op", &DepthStencilOpDesc::stencil_fail_op)
        .def_rw("stencil_depth_fail_op", &DepthStencilOpDesc::stencil_depth_fail_op)
        .def_rw("stencil_pass_op", &DepthStencilOpDesc::stencil_pass_op)
        .def_rw("stencil_func", &DepthStencilOpDesc::stencil_func);

    nb::class_<DepthStencilDesc>(m, "DepthStencilDesc")
        .def(nb::init<>())
        .def_rw("depth_test_enable", &DepthStencilDesc::depth_test_enable)
        .def_rw("depth_write_enable", &DepthStencilDesc::depth_write_enable)
        .def_rw("depth_func", &DepthStencilDesc::depth_func)
        .def_rw("stencil_enable", &DepthStencilDesc::stencil_enable)
        .def_rw("stencil_read_mask", &DepthStencilDesc::stencil_read_mask)
        .def_rw("stencil_write_mask", &DepthStencilDesc::stencil_write_mask)
        .def_rw("front_face", &DepthStencilDesc::front_face)
        .def_rw("back_face", &DepthStencilDesc::back_face)
        .def_rw("stencil_ref", &DepthStencilDesc::stencil_ref);

    nb::class_<RasterizerDesc>(m, "RasterizerDesc")
        .def(nb::init<>())
        .def_rw("fill_mode", &RasterizerDesc::fill_mode)
        .def_rw("cull_mode", &RasterizerDesc::cull_mode)
        .def_rw("front_face", &RasterizerDesc::front_face)
        .def_rw("depth_bias", &RasterizerDesc::depth_bias)
        .def_rw("depth_bias_clamp", &RasterizerDesc::depth_bias_clamp)
        .def_rw("slope_scaled_depth_bias", &RasterizerDesc::slope_scaled_depth_bias)
        .def_rw("depth_clip_enable", &RasterizerDesc::depth_clip_enable)
        .def_rw("scissor_enable", &RasterizerDesc::scissor_enable)
        .def_rw("multisample_enable", &RasterizerDesc::multisample_enable)
        .def_rw("antialiased_line_enable", &RasterizerDesc::antialiased_line_enable)
        .def_rw("enable_conservative_rasterization", &RasterizerDesc::enable_conservative_rasterization)
        .def_rw("forced_sample_count", &RasterizerDesc::forced_sample_count);

    nb::sgl_enum<LogicOp>(m, "LogicOp");
    nb::sgl_enum<BlendOp>(m, "BlendOp");
    nb::sgl_enum<BlendFactor>(m, "BlendFactor");
    nb::sgl_enum_flags<RenderTargetWriteMask>(m, "RenderTargetWriteMask");

    nb::class_<AspectBlendDesc>(m, "AspectBlendDesc")
        .def(nb::init<>())
        .def_rw("src_factor", &AspectBlendDesc::src_factor)
        .def_rw("dst_factor", &AspectBlendDesc::dst_factor)
        .def_rw("op", &AspectBlendDesc::op);

    nb::class_<TargetBlendDesc>(m, "TargetBlendDesc")
        .def(nb::init<>())
        .def_rw("color", &TargetBlendDesc::color)
        .def_rw("alpha", &TargetBlendDesc::alpha)
        .def_rw("enable_blend", &TargetBlendDesc::enable_blend)
        .def_rw("logic_op", &TargetBlendDesc::logic_op)
        .def_rw("write_mask", &TargetBlendDesc::write_mask);

    nb::class_<BlendDesc>(m, "BlendDesc")
        .def(nb::init<>())
        // TODO targets
        .def_rw("alpha_to_coverage_enable", &BlendDesc::alpha_to_coverage_enable);

    // ------------------------------------------------------------------------
    // Queries
    // ------------------------------------------------------------------------

    nb::sgl_enum<QueryType>(m, "QueryType");

    // ------------------------------------------------------------------------
    // Raytracing
    // ------------------------------------------------------------------------

    nb::sgl_enum_flags<RayTracingPipelineFlags>(m, "RayTracingPipelineFlags");
    nb::sgl_enum<RayTracingGeometryType>(m, "RayTracingGeometryType");
    nb::sgl_enum_flags<RayTracingGeometryFlags>(m, "RayTracingGeometryFlags");
    nb::sgl_enum_flags<RayTracingInstanceFlags>(m, "RayTracingInstanceFlags");

    nb::class_<RayTracingTrianglesDesc>(m, "RayTracingTrianglesDesc")
        .def(nb::init<>())
        .def_rw("transform3x4", &RayTracingTrianglesDesc::transform3x4)
        .def_rw("index_format", &RayTracingTrianglesDesc::index_format)
        .def_rw("vertex_format", &RayTracingTrianglesDesc::vertex_format)
        .def_rw("index_count", &RayTracingTrianglesDesc::index_count)
        .def_rw("vertex_count", &RayTracingTrianglesDesc::vertex_count)
        .def_rw("index_data", &RayTracingTrianglesDesc::index_data)
        .def_rw("vertex_data", &RayTracingTrianglesDesc::vertex_data)
        .def_rw("vertex_stride", &RayTracingTrianglesDesc::vertex_stride);

    nb::class_<RayTracingAABB>(m, "RayTracingAABB")
        .def(nb::init<>())
        .def_rw("min", &RayTracingAABB::min)
        .def_rw("max", &RayTracingAABB::max);

    nb::class_<RayTracingAABBsDesc>(m, "RayTracingAABBsDesc")
        .def(nb::init<>())
        .def_rw("count", &RayTracingAABBsDesc::count)
        .def_rw("data", &RayTracingAABBsDesc::data)
        .def_rw("stride", &RayTracingAABBsDesc::stride);

    nb::class_<RayTracingGeometryDesc>(m, "RayTracingGeometryDesc")
        .def(nb::init<>())
        .def_rw("type", &RayTracingGeometryDesc::type)
        .def_rw("flags", &RayTracingGeometryDesc::flags)
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

    nb::class_<RayTracingInstanceDesc>(m, "RayTracingInstanceDesc")
        .def(nb::init<>())
        .def_rw("transform", &RayTracingInstanceDesc::transform)
        .def_prop_rw(
            "instance_id",
            [](RayTracingInstanceDesc& self) { return self.instance_id; },
            [](RayTracingInstanceDesc& self, uint32_t value) { self.instance_id = value; }
        )
        .def_prop_rw(
            "instance_mask",
            [](RayTracingInstanceDesc& self) { return self.instance_mask; },
            [](RayTracingInstanceDesc& self, uint32_t value) { self.instance_mask = value; }
        )
        .def_prop_rw(
            "instance_contribution_to_hit_group_index",
            [](RayTracingInstanceDesc& self) { return self.instance_contribution_to_hit_group_index; },
            [](RayTracingInstanceDesc& self, uint32_t value) { self.instance_contribution_to_hit_group_index = value; }
        )
        .def_prop_rw("flags", &RayTracingInstanceDesc::flags, &RayTracingInstanceDesc::set_flags)
        .def_rw("acceleration_structure", &RayTracingInstanceDesc::acceleration_structure)
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
        "AccelerationStructureBuildInputs"
    )
        .def(nb::init<>())
        .def_rw("kind", &PyAccelerationStructureBuildInputs::kind)
        .def_rw("flags", &PyAccelerationStructureBuildInputs::flags)
        .def_rw("desc_count", &PyAccelerationStructureBuildInputs::desc_count)
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
            }
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
            }
        );
}
