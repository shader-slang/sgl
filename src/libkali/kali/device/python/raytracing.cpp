#include "nanobind.h"

#include "kali/device/raytracing.h"

KALI_PY_EXPORT(device_raytracing)
{
    using namespace kali;

    nb::class_<AccelerationStructure, DeviceResource> acceleration_structure(m, "AccelerationStructure");

    nb::kali_enum<AccelerationStructure::Kind>(acceleration_structure, "Kind");
    nb::kali_enum_flags<AccelerationStructure::BuildFlags>(acceleration_structure, "BuildFlags");
    nb::kali_enum<AccelerationStructure::GeometryType>(acceleration_structure, "GeometryType");
    nb::kali_enum_flags<AccelerationStructure::GeometryFlags>(acceleration_structure, "GeometryFlags");
    nb::kali_enum_flags<AccelerationStructure::InstanceFlags>(acceleration_structure, "InstanceFlags");

    nb::class_<AccelerationStructure::TriangleDesc>(acceleration_structure, "TriangleDesc")
        .def(nb::init<>())
        .def_rw("transform3x4", &AccelerationStructure::TriangleDesc::transform3x4)
        .def_rw("index_format", &AccelerationStructure::TriangleDesc::index_format)
        .def_rw("vertex_format", &AccelerationStructure::TriangleDesc::vertex_format)
        .def_rw("index_count", &AccelerationStructure::TriangleDesc::index_count)
        .def_rw("vertex_count", &AccelerationStructure::TriangleDesc::vertex_count)
        .def_rw("index_data", &AccelerationStructure::TriangleDesc::index_data)
        .def_rw("vertex_data", &AccelerationStructure::TriangleDesc::vertex_data)
        .def_rw("vertex_stride", &AccelerationStructure::TriangleDesc::vertex_stride);

    nb::class_<AccelerationStructure::ProceduralAABB>(acceleration_structure, "ProceduralAABB")
        .def(nb::init<>())
        .def_rw("min", &AccelerationStructure::ProceduralAABB::min)
        .def_rw("max", &AccelerationStructure::ProceduralAABB::max);

    nb::class_<AccelerationStructure::ProceduralAABBDesc>(acceleration_structure, "ProceduralAABBDesc")
        .def(nb::init<>())
        .def_rw("count", &AccelerationStructure::ProceduralAABBDesc::count)
        .def_rw("data", &AccelerationStructure::ProceduralAABBDesc::data)
        .def_rw("stride", &AccelerationStructure::ProceduralAABBDesc::stride);

    nb::class_<AccelerationStructure::GeometryDesc>(acceleration_structure, "GeometryDesc")
        .def(nb::init<>())
        .def_rw("type", &AccelerationStructure::GeometryDesc::type)
        .def_rw("flags", &AccelerationStructure::GeometryDesc::flags)
        .def_prop_rw(
            "triangles",
            [](AccelerationStructure::GeometryDesc& self) { return self.content.triangles; },
            [](AccelerationStructure::GeometryDesc& self, AccelerationStructure::TriangleDesc& value)
            { self.content.triangles = value; }
        )
        .def_prop_rw(
            "procedural_aabbs",
            [](AccelerationStructure::GeometryDesc& self) { return self.content.procedural_aabbs; },
            [](AccelerationStructure::GeometryDesc& self, AccelerationStructure::ProceduralAABBDesc& value)
            { self.content.procedural_aabbs = value; }
        );

    nb::class_<AccelerationStructure::InstanceDesc>(acceleration_structure, "InstanceDesc")
        .def(nb::init<>())
        .def_rw("transform", &AccelerationStructure::InstanceDesc::transform)
        .def_prop_rw(
            "instance_id",
            [](AccelerationStructure::InstanceDesc& self) { return self.instance_id; },
            [](AccelerationStructure::InstanceDesc& self, uint32_t value) { self.instance_id = value; }
        )
        .def_prop_rw(
            "instance_mask",
            [](AccelerationStructure::InstanceDesc& self) { return self.instance_mask; },
            [](AccelerationStructure::InstanceDesc& self, uint32_t value) { self.instance_mask = value; }
        )
        .def_prop_rw(
            "instance_contribution_to_hit_group_index",
            [](AccelerationStructure::InstanceDesc& self) { return self.instance_contribution_to_hit_group_index; },
            [](AccelerationStructure::InstanceDesc& self, uint32_t value)
            { self.instance_contribution_to_hit_group_index = value; }
        )
        .def_prop_rw(
            "flags",
            &AccelerationStructure::InstanceDesc::flags,
            &AccelerationStructure::InstanceDesc::set_flags
        )
        .def_rw("acceleration_structure", &AccelerationStructure::InstanceDesc::acceleration_structure);

    nb::class_<AccelerationStructure::PrebuildInfo>(acceleration_structure, "PrebuildInfo")
        .def_ro("result_data_max_size", &AccelerationStructure::PrebuildInfo::result_data_max_size)
        .def_ro("scratch_data_size", &AccelerationStructure::PrebuildInfo::scratch_data_size)
        .def_ro("update_scratch_data_size", &AccelerationStructure::PrebuildInfo::update_scratch_data_size);


    acceleration_structure //
        .def("kind", &AccelerationStructure::kind)
        .def("device_address", &AccelerationStructure::device_address);
}
