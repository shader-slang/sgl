#include "nanobind.h"

#include "kali/device/reflection.h"

NB_MAKE_OPAQUE(std::vector<kali::ref<kali::VariableReflection>>)

KALI_PY_EXPORT(device_reflection)
{
    using namespace kali;

    nb::class_<TypeReflection> type_reflection(m, "TypeReflection");

    nb::kali_enum<TypeReflection::Kind>(type_reflection, "Kind");
    nb::kali_enum<TypeReflection::ScalarType>(type_reflection, "ScalarType");

    type_reflection //
        .def_prop_ro("kind", &TypeReflection::kind)
        .def_prop_ro("name", &TypeReflection::name)
        .def_prop_ro("fields", &TypeReflection::fields)
        .def_prop_ro("element_count", &TypeReflection::element_count)
        .def_prop_ro("element_type", &TypeReflection::element_type)
        .def_prop_ro("row_count", &TypeReflection::row_count)
        .def_prop_ro("col_count", &TypeReflection::col_count)
        .def_prop_ro("scalar_type", &TypeReflection::scalar_type)
        .def_prop_ro("resource_result_type", &TypeReflection::resource_result_type)
        .def("__repr__", &TypeReflection::to_string);

    nb::class_<TypeLayoutReflection>(m, "TypeLayoutReflection")
        .def_prop_ro("kind", &TypeLayoutReflection::kind)
        .def_prop_ro("name", &TypeLayoutReflection::name)
        .def_prop_ro("type", &TypeLayoutReflection::type)
        .def_prop_ro("fields", &TypeLayoutReflection::fields)
        .def("unwrap_array", &TypeLayoutReflection::unwrap_array)
        .def("__repr__", &TypeLayoutReflection::to_string);

    nb::class_<VariableReflection>(m, "VariableReflection")
        .def_prop_ro("name", &VariableReflection::name)
        .def_prop_ro("type", &VariableReflection::type);

    nb::class_<VariableLayoutReflection>(m, "VariableLayoutReflection")
        .def_prop_ro("name", &VariableLayoutReflection::name)
        .def_prop_ro("variable", &VariableLayoutReflection::variable)
        .def_prop_ro("type_layout", &VariableLayoutReflection::type_layout);

    nb::class_<EntryPointReflection>(m, "EntryPointReflection").def("__repr__", &EntryPointReflection::to_string);

    nb::class_<ProgramReflection> program_reflection(m, "ProgramReflection");

    nb::class_<ProgramReflection::HashedString>(program_reflection, "HashedString")
        .def_ro("string", &ProgramReflection::HashedString::string)
        .def_ro("hash", &ProgramReflection::HashedString::hash);

    program_reflection //
        .def_prop_ro("globals_type_layout", &ProgramReflection::globals_type_layout)
        .def_prop_ro("globals_variable_layout", &ProgramReflection::globals_variable_layout)
        .def_prop_ro("parameters", &ProgramReflection::parameters)
        .def_prop_ro("entry_points", &ProgramReflection::entry_points)
        .def_prop_ro("hashed_strings", &ProgramReflection::hashed_strings)
        .def("__repr__", &ProgramReflection::to_string);
}
