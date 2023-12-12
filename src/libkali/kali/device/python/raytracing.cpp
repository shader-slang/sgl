#include "nanobind.h"

#include "kali/device/raytracing.h"

KALI_PY_EXPORT(device_raytracing)
{
    using namespace kali;

    nb::class_<AccelerationStructureQueryDesc>(m, "AccelerationStructureQueryDesc");

    nb::class_<AccelerationStructureBuildDesc>(m, "AccelerationStructureBuildDesc");

    nb::class_<AccelerationStructure, DeviceResource> acceleration_structure(m, "AccelerationStructure");

    nb::class_<AccelerationStructurePrebuildInfo>(m, "AccelerationStructurePrebuildInfo")
        .def_ro("result_data_max_size", &AccelerationStructurePrebuildInfo::result_data_max_size)
        .def_ro("scratch_data_size", &AccelerationStructurePrebuildInfo::scratch_data_size)
        .def_ro("update_scratch_data_size", &AccelerationStructurePrebuildInfo::update_scratch_data_size);


    acceleration_structure //
        .def("kind", &AccelerationStructure::kind)
        .def_prop_ro("device_address", &AccelerationStructure::device_address);
}
