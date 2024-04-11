// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/raytracing.h"
#include "sgl/device/shader.h"

namespace sgl {
SGL_DICT_TO_DESC_BEGIN(ShaderTableDesc)
SGL_DICT_TO_DESC_FIELD(program, ShaderProgram*)
SGL_DICT_TO_DESC_FIELD(ray_gen_entry_points, std::vector<std::string>)
SGL_DICT_TO_DESC_FIELD(miss_entry_points, std::vector<std::string>)
SGL_DICT_TO_DESC_FIELD(hit_group_names, std::vector<std::string>)
SGL_DICT_TO_DESC_FIELD(callable_entry_points, std::vector<std::string>)
SGL_DICT_TO_DESC_END()
} // namespace sgl

SGL_PY_EXPORT(device_raytracing)
{
    using namespace sgl;

    nb::class_<AccelerationStructureQueryDesc>(m, "AccelerationStructureQueryDesc", D(AccelerationStructureQueryDesc));

    nb::class_<AccelerationStructureBuildDesc>(m, "AccelerationStructureBuildDesc", D(AccelerationStructureBuildDesc));

    nb::class_<AccelerationStructure, DeviceResource> acceleration_structure(
        m,
        "AccelerationStructure",
        D(AccelerationStructure)
    );

    nb::class_<AccelerationStructurePrebuildInfo>(m, "AccelerationStructurePrebuildInfo")
        .def_ro(
            "result_data_max_size",
            &AccelerationStructurePrebuildInfo::result_data_max_size,
            D(AccelerationStructurePrebuildInfo, result_data_max_size)
        )
        .def_ro(
            "scratch_data_size",
            &AccelerationStructurePrebuildInfo::scratch_data_size,
            D(AccelerationStructurePrebuildInfo, scratch_data_size)
        )
        .def_ro(
            "update_scratch_data_size",
            &AccelerationStructurePrebuildInfo::update_scratch_data_size,
            D(AccelerationStructurePrebuildInfo, update_scratch_data_size)
        );


    acceleration_structure //
        .def("kind", &AccelerationStructure::kind, D(AccelerationStructure, kind))
        .def_prop_ro(
            "device_address",
            &AccelerationStructure::device_address,
            D(AccelerationStructure, device_address)
        );

    nb::class_<ShaderTableDesc>(m, "ShaderTableDesc", D(ShaderTableDesc))
        .def(nb::init<>())
        .def(
            "__init__",
            [](ShaderTableDesc* self, nb::dict dict) { new (self) ShaderTableDesc(dict_to_ShaderTableDesc(dict)); }
        )
        .def_rw("program", &ShaderTableDesc::program, D(ShaderTableDesc, program))
        .def_rw(
            "ray_gen_entry_points",
            &ShaderTableDesc::ray_gen_entry_points,
            D(ShaderTableDesc, ray_gen_entry_points)
        )
        .def_rw("miss_entry_points", &ShaderTableDesc::miss_entry_points, D(ShaderTableDesc, miss_entry_points))
        .def_rw("hit_group_names", &ShaderTableDesc::hit_group_names, D(ShaderTableDesc, hit_group_names))
        .def_rw(
            "callable_entry_points",
            &ShaderTableDesc::callable_entry_points,
            D(ShaderTableDesc, callable_entry_points)
        );
    nb::implicitly_convertible<nb::dict, ShaderTableDesc>();

    nb::class_<ShaderTable, DeviceResource>(m, "ShaderTable", D(ShaderTable));
}
