#include "nanobind.h"

#include "kali/device/reflection.h"

NB_MAKE_OPAQUE(std::vector<kali::ref<kali::VariableReflection>>)

KALI_PY_EXPORT(device_reflection)
{
    using namespace kali;

    nb::class_<TypeReflection, Object> type_reflection(m, "TypeReflection");

    nb::kali_enum<TypeReflection::Kind>(type_reflection, "Kind");

    type_reflection //
        .def_prop_ro("kind", &TypeReflection::kind)
        .def("as_struct_type", &TypeReflection::as_struct_type, nb::rv_policy::reference_internal)
        .def("as_array_type", &TypeReflection::as_array_type, nb::rv_policy::reference_internal)
        .def("as_basic_type", &TypeReflection::as_basic_type, nb::rv_policy::reference_internal)
        .def("as_resource_type", &TypeReflection::as_resource_type, nb::rv_policy::reference_internal)
        .def("as_interface_type", &TypeReflection::as_interface_type, nb::rv_policy::reference_internal);

    nb::class_<StructTypeReflection, TypeReflection>(m, "StructTypeReflection")
        .def_prop_ro("name", &StructTypeReflection::name)
        .def_prop_ro("members", &StructTypeReflection::members);

    nb::class_<ArrayTypeReflection, TypeReflection>(m, "ArrayTypeReflection")
        .def_prop_ro("element_type", &ArrayTypeReflection::element_type)
        .def_prop_ro("element_count", &ArrayTypeReflection::element_count)
        .def_prop_ro("element_stride", &ArrayTypeReflection::element_stride);

    nb::class_<BasicTypeReflection, TypeReflection>(m, "BasicTypeReflection")
        .def_prop_ro("scalar_type", &BasicTypeReflection::scalar_type)
        .def_prop_ro("row_count", &BasicTypeReflection::row_count)
        .def_prop_ro("col_count", &BasicTypeReflection::col_count)
        .def_prop_ro("is_row_major", &BasicTypeReflection::is_row_major)
        .def_prop_ro("is_vector", &BasicTypeReflection::is_vector)
        .def_prop_ro("is_matrix", &BasicTypeReflection::is_matrix);

    nb::class_<ResourceTypeReflection, TypeReflection>(m, "ResourceTypeReflection");
    nb::class_<InterfaceTypeReflection, TypeReflection>(m, "InterfaceTypeReflection");

    nb::class_<ProgramLayout, Object>(m, "ProgramLayout").def_prop_ro("globals_type", &ProgramLayout::globals_type);

    nb::class_<EntryPointLayout, Object>(m, "EntryPointLayout")
        .def_prop_ro("name", &EntryPointLayout::name)
        .def_prop_ro("compute_thread_group_size", &EntryPointLayout::compute_thread_group_size);
}
