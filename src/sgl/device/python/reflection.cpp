// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/reflection.h"
#include "sgl/device/shader.h"

NB_MAKE_OPAQUE(std::vector<sgl::ref<sgl::VariableReflection>>)

SGL_PY_EXPORT(device_reflection)
{
    using namespace sgl;

    nb::class_<TypeReflection> type_reflection(m, "TypeReflection");

    nb::sgl_enum<TypeReflection::Kind>(type_reflection, "Kind");
    nb::sgl_enum<TypeReflection::ScalarType>(type_reflection, "ScalarType");
    nb::sgl_enum<TypeReflection::ResourceShape>(type_reflection, "ResourceShape");
    nb::sgl_enum<TypeReflection::ResourceAccess>(type_reflection, "ResourceAccess");
    nb::sgl_enum<TypeReflection::ParameterCategory>(type_reflection, "ParameterCategory");

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
        .def_prop_ro("resource_shape", &TypeReflection::resource_shape)
        .def_prop_ro("resource_access", &TypeReflection::resource_access)
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

    nb::class_<EntryPointLayout>(m, "EntryPointLayout", D(EntryPointLayout))
        .def_prop_ro("name", &EntryPointLayout::name, D(EntryPointLayout, name))
        .def_prop_ro("name_override", &EntryPointLayout::name_override, D(EntryPointLayout, name_override))
        .def_prop_ro("stage", &EntryPointLayout::stage, D(EntryPointLayout, stage))
        .def_prop_ro(
            "compute_thread_group_size",
            &EntryPointLayout::compute_thread_group_size,
            D(EntryPointLayout, compute_thread_group_size)
        )
        .def("__repr__", &EntryPointLayout::to_string);

    nb::class_<ProgramLayout> program_layout(m, "ProgramLayout", D(ProgramLayout));

    nb::class_<ProgramLayout::HashedString>(program_layout, "HashedString", D(ProgramLayout, HashedString))
        .def_ro("string", &ProgramLayout::HashedString::string, D(ProgramLayout, HashedString, string))
        .def_ro("hash", &ProgramLayout::HashedString::hash, D(ProgramLayout, HashedString, hash));

    program_layout //
        .def_prop_ro("globals_type_layout", &ProgramLayout::globals_type_layout, D(ProgramLayout, globals_type_layout))
        .def_prop_ro(
            "globals_variable_layout",
            &ProgramLayout::globals_variable_layout,
            D(ProgramLayout, globals_variable_layout)
        )
        .def_prop_ro("parameters", &ProgramLayout::parameters, D(ProgramLayout, parameters))
        .def_prop_ro("entry_points", &ProgramLayout::entry_points, D(ProgramLayout, entry_points))
        .def_prop_ro("hashed_strings", &ProgramLayout::hashed_strings, D(ProgramLayout, hashed_strings))
        .def("__repr__", &ProgramLayout::to_string);

    nb::class_<ReflectionCursor>(m, "ReflectionCursor", D(ReflectionCursor))
        .def(nb::init<const ShaderProgram*>(), "shader_program"_a)
        .def("is_valid", &ReflectionCursor::is_valid, D(ReflectionCursor, is_valid))
        .def("find_field", &ReflectionCursor::find_field, "name"_a, D(ReflectionCursor, find_field))
        .def("find_element", &ReflectionCursor::find_element, "index"_a, D(ReflectionCursor, find_element))
        .def("has_field", &ReflectionCursor::has_field, "name"_a, D(ReflectionCursor, has_field))
        .def("has_element", &ReflectionCursor::has_element, "index"_a, D(ReflectionCursor, has_element))
        .def_prop_ro("type_layout", &ReflectionCursor::type_layout, D(ReflectionCursor, type_layout))
        .def_prop_ro("type", &ReflectionCursor::type, D(ReflectionCursor, type))
        .def("__getitem__", [](ReflectionCursor& self, std::string_view name) { return self[name]; })
        .def("__getitem__", [](ReflectionCursor& self, int index) { return self[index]; })
        .def("__getattr__", [](ReflectionCursor& self, std::string_view name) { return self[name]; })
        .def("__repr__", &ReflectionCursor::to_string);
}
