#include "nanobind.h"

#include "kali/device/pipeline.h"
#include "kali/device/shader.h"
#include "kali/device/input_layout.h"
#include "kali/device/framebuffer.h"

namespace kali {
KALI_DICT_TO_DESC_BEGIN(ComputePipelineDesc)
KALI_DICT_TO_DESC_FIELD(program, ShaderProgram*)
KALI_DICT_TO_DESC_END()

KALI_DICT_TO_DESC_BEGIN(DepthStencilDesc)
KALI_DICT_TO_DESC_FIELD(depth_test_enable, bool)
KALI_DICT_TO_DESC_FIELD(depth_write_enable, bool)
KALI_DICT_TO_DESC_FIELD(depth_func, ComparisonFunc)
KALI_DICT_TO_DESC_FIELD(stencil_enable, bool)
KALI_DICT_TO_DESC_FIELD(stencil_read_mask, uint32_t)
KALI_DICT_TO_DESC_FIELD(stencil_write_mask, uint32_t)
KALI_DICT_TO_DESC_FIELD(front_face, DepthStencilOpDesc)
KALI_DICT_TO_DESC_FIELD(back_face, DepthStencilOpDesc)
KALI_DICT_TO_DESC_FIELD(stencil_ref, uint32_t)
KALI_DICT_TO_DESC_END()

KALI_DICT_TO_DESC_BEGIN(RasterizerDesc)
KALI_DICT_TO_DESC_FIELD(fill_mode, FillMode)
KALI_DICT_TO_DESC_FIELD(cull_mode, CullMode)
KALI_DICT_TO_DESC_FIELD(front_face, FrontFaceMode)
KALI_DICT_TO_DESC_FIELD(depth_bias, int32_t)
KALI_DICT_TO_DESC_FIELD(depth_bias_clamp, float)
KALI_DICT_TO_DESC_FIELD(slope_scaled_depth_bias, float)
KALI_DICT_TO_DESC_FIELD(depth_clip_enable, bool)
KALI_DICT_TO_DESC_FIELD(scissor_enable, bool)
KALI_DICT_TO_DESC_FIELD(multisample_enable, bool)
KALI_DICT_TO_DESC_FIELD(antialiased_line_enable, bool)
KALI_DICT_TO_DESC_FIELD(enable_conservative_rasterization, bool)
KALI_DICT_TO_DESC_FIELD(forced_sample_count, uint32_t)
KALI_DICT_TO_DESC_END()

KALI_DICT_TO_DESC_BEGIN(BlendDesc)
// TODO
KALI_DICT_TO_DESC_FIELD(alpha_to_coverage_enable, bool)
KALI_DICT_TO_DESC_END()

KALI_DICT_TO_DESC_BEGIN(GraphicsPipelineDesc)
KALI_DICT_TO_DESC_FIELD(program, ShaderProgram*)
KALI_DICT_TO_DESC_FIELD(input_layout, InputLayout*)
KALI_DICT_TO_DESC_FIELD(framebuffer, Framebuffer*)
KALI_DICT_TO_DESC_FIELD(primitive_type, PrimitiveType)
KALI_DICT_TO_DESC_FIELD_DICT(depth_stencil, DepthStencilDesc)
KALI_DICT_TO_DESC_FIELD_DICT(rasterizer, RasterizerDesc)
KALI_DICT_TO_DESC_FIELD_DICT(blend, BlendDesc)
KALI_DICT_TO_DESC_END()

KALI_DICT_TO_DESC_BEGIN(HitGroupDesc)
KALI_DICT_TO_DESC_FIELD(hit_group_name, std::string)
KALI_DICT_TO_DESC_FIELD(closest_hit_entry_point, std::string)
KALI_DICT_TO_DESC_FIELD(any_hit_entry_point, std::string)
KALI_DICT_TO_DESC_FIELD(intersection_entry_point, std::string)
KALI_DICT_TO_DESC_END()

KALI_DICT_TO_DESC_BEGIN(RayTracingPipelineDesc)
KALI_DICT_TO_DESC_FIELD(program, ShaderProgram*)
KALI_DICT_TO_DESC_FIELD_LIST(hit_groups, HitGroupDesc)
KALI_DICT_TO_DESC_FIELD(max_recursion, uint32_t)
KALI_DICT_TO_DESC_FIELD(max_ray_payload_size, uint32_t)
KALI_DICT_TO_DESC_FIELD(max_attribute_size, uint32_t)
KALI_DICT_TO_DESC_FIELD(flags, RayTracingPipelineFlags)
KALI_DICT_TO_DESC_END()
} // namespace kali

KALI_PY_EXPORT(device_pipeline)
{
    using namespace kali;

    nb::class_<Pipeline, DeviceResource>(m, "Pipeline");

    nb::class_<ComputePipelineDesc>(m, "ComputePipelineDesc")
        .def(nb::init<>())
        .def(
            "__init__",
            [](ComputePipelineDesc* self, const nb::dict& dict)
            { new (self) ComputePipelineDesc(dict_to_ComputePipelineDesc(dict)); }
        )
        .def_rw("program", &ComputePipelineDesc::program);
    nb::implicitly_convertible<nb::dict, ComputePipelineDesc>();

    nb::class_<ComputePipeline, Pipeline>(m, "ComputePipeline")
        .def_prop_ro("thread_group_size", &ComputePipeline::thread_group_size);

    nb::class_<GraphicsPipeline, Pipeline>(m, "GraphicsPipeline");

    nb::class_<HitGroupDesc>(m, "HitGroupDesc")
        .def(nb::init<>())
        .def(
            "__init__",
            [](HitGroupDesc* self, const nb::dict& dict) { new (self) HitGroupDesc(dict_to_HitGroupDesc(dict)); }
        )
        .def(
            nb::init<std::string, std::string, std::string, std::string>(),
            "hit_group_name"_a,
            "closest_hit_entry_point"_a = "",
            "any_hit_entry_point"_a = "",
            "intersection_entry_point"_a = ""
        )
        .def_rw("hit_group_name", &HitGroupDesc::hit_group_name)
        .def_rw("closest_hit_entry_point", &HitGroupDesc::closest_hit_entry_point)
        .def_rw("any_hit_entry_point", &HitGroupDesc::any_hit_entry_point)
        .def_rw("intersection_entry_point", &HitGroupDesc::intersection_entry_point);
    nb::implicitly_convertible<nb::dict, HitGroupDesc>();

    nb::class_<RayTracingPipelineDesc>(m, "RayTracingPipelineDesc")
        .def(nb::init<>())
        .def(
            "__init__",
            [](RayTracingPipelineDesc* self, const nb::dict& dict)
            { new (self) RayTracingPipelineDesc(dict_to_RayTracingPipelineDesc(dict)); }
        )
        .def_rw("program", &RayTracingPipelineDesc::program)
        .def_ro("hit_groups", &RayTracingPipelineDesc::hit_groups, nb::rv_policy::reference_internal)
        .def_rw("max_recursion", &RayTracingPipelineDesc::max_recursion)
        .def_rw("max_ray_payload_size", &RayTracingPipelineDesc::max_ray_payload_size)
        .def_rw("max_attribute_size", &RayTracingPipelineDesc::max_attribute_size)
        .def_rw("flags", &RayTracingPipelineDesc::flags);
    nb::implicitly_convertible<nb::dict, RayTracingPipelineDesc>();

    nb::class_<RayTracingPipeline, Pipeline>(m, "RayTracingPipeline");
}
