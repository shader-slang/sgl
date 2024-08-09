// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/reflection.h"
#include "sgl/device/shader.h"

NB_MAKE_OPAQUE(std::vector<sgl::ref<sgl::VariableReflection>>)

SGL_PY_EXPORT(device_reflection)
{
    using namespace sgl;

    nb::class_<DeclReflection> decl_reflection(m, "DeclReflection");

    nb::sgl_enum<DeclReflection::Kind>(decl_reflection, "Kind");

    decl_reflection //
        .def_prop_ro("kind", &DeclReflection::kind, D_NA(DeclReflection, kind))
        .def_prop_ro("children", &DeclReflection::children, D_NA(DeclReflection, children))
        .def_prop_ro("child_count", &DeclReflection::child_count, D_NA(DeclReflection, child_count))
        .def_prop_ro("name", &DeclReflection::name)
        .def(
            "children_of_kind",
            &DeclReflection::children_of_kind,
            nb::rv_policy::reference_internal,
            "kind"_a,
            D_NA(DeclReflection, children_of_kind)
        )
        .def("as_type", &DeclReflection::as_type, nb::rv_policy::reference_internal, D_NA(DeclReflection, as_type))
        .def(
            "as_variable",
            &DeclReflection::as_variable,
            nb::rv_policy::reference_internal,
            D_NA(DeclReflection, as_variable)
        )
        .def(
            "as_function",
            &DeclReflection::as_function,
            nb::rv_policy::reference_internal,
            D_NA(DeclReflection, as_function)
        )
        .def(
            "find_children_of_kind",
            &DeclReflection::find_children_of_kind,
            nb::rv_policy::reference_internal,
            "kind"_a,
            "child_name"_a,
            D_NA(DeclReflection, find_children_of_kind)
        )
        .def(
            "find_first_child_of_kind",
            &DeclReflection::find_first_child_of_kind,
            nb::rv_policy::reference_internal,
            "kind"_a,
            "child_name"_a,
            D_NA(DeclReflection, find_first_child_of_kind)
        )
        .def("__len__", [](DeclReflection& self) { return self.child_count(); })
        .def(
            "__getitem__",
            [](DeclReflection& self, int index) { return self[index]; },
            nb::rv_policy::reference_internal
        )
        .def("__repr__", &DeclReflection::to_string);


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
        .def("unwrap_array", &TypeReflection::unwrap_array)
        .def("__repr__", &TypeReflection::to_string);

    nb::class_<TypeLayoutReflection>(m, "TypeLayoutReflection")
        .def_prop_ro("kind", &TypeLayoutReflection::kind)
        .def_prop_ro("name", &TypeLayoutReflection::name)
        .def_prop_ro("size", &TypeLayoutReflection::size)
        .def_prop_ro("stride", &TypeLayoutReflection::stride)
        .def_prop_ro("alignment", &TypeLayoutReflection::alignment)
        .def_prop_ro("type", &TypeLayoutReflection::type)
        .def_prop_ro("fields", &TypeLayoutReflection::fields)
        .def_prop_ro("element_type_layout", &TypeLayoutReflection::element_type_layout)
        .def("unwrap_array", &TypeLayoutReflection::unwrap_array)
        .def("__repr__", &TypeLayoutReflection::to_string);

    nb::class_<FunctionReflection>(m, "FunctionReflection")
        .def_prop_ro("name", &FunctionReflection::name)
        .def_prop_ro("return_type", &FunctionReflection::return_type)
        .def_prop_ro("parameters", &FunctionReflection::parameters)
        .def("has_modifier", &FunctionReflection::has_modifier, "modifier"_a, D_NA(FunctionReflection, has_modifier));

    nb::sgl_enum<ModifierID>(m, "ModifierID");

    nb::class_<VariableReflection>(m, "VariableReflection")
        .def_prop_ro("name", &VariableReflection::name)
        .def_prop_ro("type", &VariableReflection::type)
        .def("has_modifier", &VariableReflection::has_modifier, "modifier"_a, D_NA(VariableReflection, has_modifier));

    nb::class_<VariableLayoutReflection>(m, "VariableLayoutReflection")
        .def_prop_ro("name", &VariableLayoutReflection::name)
        .def_prop_ro("variable", &VariableLayoutReflection::variable)
        .def_prop_ro("type_layout", &VariableLayoutReflection::type_layout)
        .def_prop_ro("offset", &VariableLayoutReflection::offset)
        .def("__repr__", &VariableLayoutReflection::to_string);

    nb::class_<EntryPointLayout>(m, "EntryPointLayout", D(EntryPointLayout))
        .def_prop_ro("name", &EntryPointLayout::name, D(EntryPointLayout, name))
        .def_prop_ro("name_override", &EntryPointLayout::name_override, D(EntryPointLayout, name_override))
        .def_prop_ro("stage", &EntryPointLayout::stage, D(EntryPointLayout, stage))
        .def_prop_ro(
            "compute_thread_group_size",
            &EntryPointLayout::compute_thread_group_size,
            D(EntryPointLayout, compute_thread_group_size)
        )
        .def_prop_ro("parameters", &EntryPointLayout::parameters, D_NA(EntryPointLayout, parameters))
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
