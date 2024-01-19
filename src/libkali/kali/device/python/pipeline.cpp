#include "nanobind.h"

#include "kali/device/pipeline.h"
#include "kali/device/shader.h"

KALI_PY_EXPORT(device_pipeline)
{
    using namespace kali;

    nb::class_<Pipeline, DeviceResource>(m, "Pipeline");

    nb::class_<ComputePipelineDesc>(m, "ComputePipelineDesc")
        .def(nb::init<>())
        .def_rw("program", &ComputePipelineDesc::program);

    nb::class_<ComputePipeline, Pipeline>(m, "ComputePipeline")
        .def_prop_ro("thread_group_size", &ComputePipeline::thread_group_size);

    nb::class_<HitGroupDesc>(m, "HitGroupDesc")
        .def(nb::init<>())
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

    nb::class_<RayTracingPipelineDesc>(m, "RayTracingPipelineDesc")
        .def(nb::init<>())
        .def_rw("program", &RayTracingPipelineDesc::program)
        .def_ro("hit_groups", &RayTracingPipelineDesc::hit_groups, nb::rv_policy::reference_internal)
        .def_rw("max_recursion", &RayTracingPipelineDesc::max_recursion)
        .def_rw("max_ray_payload_size", &RayTracingPipelineDesc::max_ray_payload_size)
        .def_rw("max_attribute_size", &RayTracingPipelineDesc::max_attribute_size)
        .def_rw("flags", &RayTracingPipelineDesc::flags);

    nb::class_<RayTracingPipeline, Pipeline>(m, "RayTracingPipeline");
}
