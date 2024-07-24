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
        .def_prop_ro("parameters", &FunctionReflection::parameters);


    nb::class_<VariableReflection> variable_reflection(m, "VariableReflection");

    nb::sgl_enum<VariableReflection::ModifierType>(variable_reflection, "ModifierType");

    variable_reflection //
        .def_prop_ro("name", &VariableReflection::name)
        .def_prop_ro("type", &VariableReflection::type)
        .def("has_modifier", &VariableReflection::has_modifier, "modifier"_a, D_NA(VariableReflection, modifier));

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

    nb::class_<ASTCursor, Object> ast_cursor(m, "ASTCursor", D_NA(ASTCursor));

    nb::sgl_enum<ASTCursor::Kind>(ast_cursor, "Kind");

    ast_cursor //
        .def_prop_ro("kind", &ASTCursor::kind, D_NA(ASTCursor, kind))
        .def_prop_ro("children", &ASTCursor::children, D_NA(ASTCursor, kind))
        .def("__len__", [](ASTCursor& self) { return self.child_count(); })
        .def("__getitem__", [](ASTCursor& self, int index) { return self[index]; })
        .def("__repr__", &ASTCursor::to_string);

    nb::class_<ASTCursorModule, ASTCursor>(m, "ASTCursorModule", D_NA(ASTCursorModule))
        .def_prop_ro("name", &ASTCursorModule::name)
        .def_prop_ro("globals", &ASTCursorModule::globals)
        .def_prop_ro("functions", &ASTCursorModule::functions)
        .def_prop_ro("structs", &ASTCursorModule::structs)
        .def("find_functions", &ASTCursorModule::find_functions, "name"_a, D_NA(ASTCursorModule, name))
        .def("find_first_function", &ASTCursorModule::find_first_function, "name"_a, D_NA(ASTCursorModule, name))
        .def("find_global", &ASTCursorModule::find_global, "name"_a, D_NA(ASTCursorModule, name));

    nb::class_<ASTCursorStruct, ASTCursor>(m, "ASTCursorStruct", D_NA(ASTCursorStruct))
        .def_prop_ro("name", &ASTCursorStruct::name)
        .def_prop_ro("type", &ASTCursorStruct::type)
        .def_prop_ro("fields", &ASTCursorStruct::fields)
        .def_prop_ro("functions", &ASTCursorStruct::functions)
        .def_prop_ro("structs", &ASTCursorStruct::structs)
        .def("find_functions", &ASTCursorStruct::find_functions, "name"_a, D_NA(ASTCursorStruct, name))
        .def("find_first_function", &ASTCursorStruct::find_first_function, "name"_a, D_NA(ASTCursorStruct, name))
        .def("find_field", &ASTCursorStruct::find_field, "name"_a, D_NA(ASTCursorStruct, name));

    nb::class_<ASTCursorFunction, ASTCursor>(m, "ASTCursorFunction", D_NA(ASTCursorFunction))
        .def_prop_ro("name", &ASTCursorFunction::name)
        .def_prop_ro("function", &ASTCursorFunction::function)
        .def_prop_ro("parameters", &ASTCursorFunction::parameters)
        .def("find_parameter", &ASTCursorFunction::find_parameter, "name"_a, D_NA(ASTCursorFunction, name));

    nb::class_<ASTCursorVariable, ASTCursor>(m, "ASTCursorVariable", D_NA(ASTCursorVariable))
        .def_prop_ro("name", &ASTCursorVariable::name)
        .def_prop_ro("type", &ASTCursorVariable::type);

    nb::class_<ASTCursorGeneric, ASTCursor>(m, "ASTCursorGeneric", D_NA(ASTCursorGeneric));
}
