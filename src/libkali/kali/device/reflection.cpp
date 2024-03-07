// SPDX-License-Identifier: Apache-2.0

#include "reflection.h"

#include "kali/device/shader.h"

#include "kali/core/string.h"

#include "kali/math/vector.h"

#include <span>

namespace kali {

std::string c_str_to_string(const char* str)
{
    if (!str)
        return "null";
    return fmt::format("\"{}\"", str);
}

std::string TypeReflection::to_string() const
{
    std::string str;
    str += "TypeReflection(\n";
    // str += fmt::format("  kind={},\n", kind());
    // str += fmt::format("  name={},\n", c_str_to_string(name()));
    // str += fmt::format("  fields={},", vector_to_string(fields()));
    str += ")";
    return str;
}

std::string TypeLayoutReflection::to_string() const
{
    switch (kind()) {
    case TypeReflection::Kind::struct_:
        return fmt::format(
            "TypeLayoutReflection(\n"
            "  name = {},\n"
            "  kind = {},\n"
            "  size = {},\n"
            "  stride = {},\n"
            "  fields = {}\n"
            ")",
            c_str_to_string(name()),
            kind(),
            size(),
            stride(),
            string::indent(string::list_to_string(fields()))
        );
        break;
    case TypeReflection::Kind::resource:
        return fmt::format(
            "TypeLayoutReflection(\n"
            "  name = {},\n"
            "  kind = {},\n"
            "  shape = {},\n"
            "  access = {},\n"
            "  element_type_layout = {}\n"
            ")",
            c_str_to_string(name()),
            kind(),
            type()->resource_shape(),
            type()->resource_access(),
            string::indent(element_type_layout()->to_string())
        );
        break;
    case TypeReflection::Kind::scalar:
        return fmt::format(
            "TypeLayoutReflection(\n"
            "  kind = {},\n"
            "  scalar_type = {}\n"
            ")",
            kind(),
            type()->scalar_type()
        );
        break;
    case TypeReflection::Kind::vector:
    case TypeReflection::Kind::matrix:
        return fmt::format(
            "TypeLayoutReflection(\n"
            "  kind = {},\n"
            "  scalar_type = {},\n"
            "  row_count = {},\n"
            "  col_count = {}\n"
            ")",
            kind(),
            type()->scalar_type(),
            type()->row_count(),
            type()->col_count()
        );
        break;
    default:
        return fmt::format(
            "TypeLayoutReflection(\n"
            "  kind = {}\n"
            ")",
            kind()
        );
    }
}

std::string VariableLayoutReflection::to_string() const
{
    return fmt::format(
        "VariableLayoutReflection(\n"
        "  name = {},\n"
        "  type_layout = {}\n"
        ")",
        c_str_to_string(name()),
        string::indent(type_layout()->to_string())
    );
}

std::string EntryPointLayout::to_string() const
{
    return fmt::format(
        "EntryPointLayout(\n"
        "  name = {},\n"
        "  name_override = {},\n"
        "  stage = {},\n"
        "  compute_thread_group_size = {},\n"
        "  parameters = {}\n"
        ")",
        c_str_to_string(name()),
        c_str_to_string(name_override()),
        stage(),
        compute_thread_group_size(),
        string::indent(string::list_to_string(parameters()))
    );
}

std::string ProgramLayout::to_string() const
{
    return fmt::format(
        "ProgramLayout(\n"
        "  globals_type = {},\n"
        "  parameters = {},\n"
        "  entry_points = {}\n"
        ")",
        string::indent(globals_type_layout()->to_string()),
        string::indent(string::list_to_string(parameters())),
        string::indent(string::list_to_string(entry_points()))
    );
}

// ----------------------------------------------------------------------------
// ReflectionCursor
// ----------------------------------------------------------------------------

ReflectionCursor::ReflectionCursor(const ShaderProgram* shader_program)
    : m_shader_program(shader_program)
    , m_valid(m_shader_program != nullptr)
{
}

ReflectionCursor::ReflectionCursor(const EntryPointLayout* entry_point_layout)
    : m_entry_point_layout(entry_point_layout)
    , m_valid(m_entry_point_layout != nullptr)
{
}

ReflectionCursor::ReflectionCursor(const TypeLayoutReflection* type_layout)
    : m_type_layout(type_layout)
    , m_valid(m_type_layout != nullptr)
{
}

ReflectionCursor ReflectionCursor::operator[](std::string_view name) const
{
    KALI_CHECK(is_valid(), "Invalid cursor");
    ReflectionCursor result = find_field(name);
    KALI_CHECK(result.is_valid(), "Field \"{}\" not found.", name);
    return result;
}

ReflectionCursor ReflectionCursor::operator[](uint32_t index) const
{
    KALI_CHECK(is_valid(), "Invalid cursor");
    ReflectionCursor result = find_element(index);
    KALI_CHECK(result.is_valid(), "Element {} not found.", index);
    return result;
}

ReflectionCursor ReflectionCursor::find_field(std::string_view name) const
{
    if (m_shader_program) {
        // Try to find field in global variables.
        if (auto global_field
            = ReflectionCursor(m_shader_program->program_layout()->globals_type_layout()).find_field(name);
            global_field.is_valid())
            return global_field;
        // Try to find an entry point.
        for (const EntryPointLayout* entry_point_layout : m_shader_program->entry_point_layouts()) {
            if (entry_point_layout->name() == name)
                return ReflectionCursor(entry_point_layout);
        }
    } else if (m_entry_point_layout) {
        // Try to find parameter in entry point.
        for (uint32_t i = 0; i < m_entry_point_layout->parameter_count(); ++i) {
            if (m_entry_point_layout->get_parameter_by_index(i)->name() == name)
                return ReflectionCursor(m_entry_point_layout->get_parameter_by_index(i)->type_layout());
        }
    } else if (m_type_layout) {
        // If type is a constant buffer or parameter block, try to find field in element type.
        const TypeLayoutReflection* type_layout = m_type_layout;
        if (type_layout->kind() == TypeReflection::Kind::constant_buffer
            || type_layout->kind() == TypeReflection::Kind::parameter_block)
            type_layout = m_type_layout->element_type_layout();
        if (type_layout->kind() == TypeReflection::Kind::struct_) {
            int32_t field_index = type_layout->find_field_index_by_name(name.data(), name.data() + name.size());
            if (field_index >= 0) {
                const VariableLayoutReflection* field_layout = type_layout->get_field_by_index(field_index);
                return ReflectionCursor(field_layout->type_layout());
            }
        }
    }
    return {};
}

ReflectionCursor ReflectionCursor::find_element(uint32_t index) const
{
    KALI_UNUSED(index);
    return {};
}

std::string ReflectionCursor::to_string() const
{
    if (m_shader_program)
        return fmt::format("ReflectionCursor(program={})", string::indent(m_shader_program->to_string()));
    if (m_entry_point_layout)
        return fmt::format("ReflectionCursor(entry_point={})", string::indent(m_entry_point_layout->to_string()));
    if (m_type_layout)
        return fmt::format("ReflectionCursor(type={})", string::indent(m_type_layout->to_string()));
    return "ReflectionCursor(null)";
}

} // namespace kali
