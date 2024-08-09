// SPDX-License-Identifier: Apache-2.0

#include "reflection.h"

#include "sgl/device/device.h"
#include "sgl/device/shader.h"

#include "sgl/core/string.h"

#include "sgl/math/vector.h"

#include <span>

namespace sgl {

namespace detail {

    ref<const DeclReflection> from_slang(ref<const Object> owner, slang::DeclReflection* decl_reflection)
    {
        return make_ref<const DeclReflection>(owner, decl_reflection);
    }
    ref<const TypeReflection> from_slang(ref<const Object> owner, slang::TypeReflection* type_reflection)
    {
        return make_ref<const TypeReflection>(owner, type_reflection);
    }
    ref<const TypeLayoutReflection>
    from_slang(ref<const Object> owner, slang::TypeLayoutReflection* type_layout_reflection)
    {
        return make_ref<const TypeLayoutReflection>(owner, type_layout_reflection);
    }
    ref<const FunctionReflection> from_slang(ref<const Object> owner, slang::FunctionReflection* variable_reflection)
    {
        return make_ref<const FunctionReflection>(owner, variable_reflection);
    }
    ref<const VariableReflection> from_slang(ref<const Object> owner, slang::VariableReflection* variable_reflection)
    {
        return make_ref<const VariableReflection>(owner, variable_reflection);
    }
    ref<const VariableLayoutReflection>
    from_slang(ref<const Object> owner, slang::VariableLayoutReflection* variable_layout_reflection)
    {
        return make_ref<const VariableLayoutReflection>(owner, variable_layout_reflection);
    }
    ref<const EntryPointLayout> from_slang(ref<const Object> owner, slang::EntryPointLayout* entry_point_reflection)
    {
        return make_ref<const EntryPointLayout>(owner, entry_point_reflection);
    }
    ref<const ProgramLayout> from_slang(ref<const Object> owner, slang::ProgramLayout* program_layout)
    {
        return make_ref<const ProgramLayout>(owner, program_layout);
    }
} // namespace detail

std::string c_str_to_string(const char* str)
{
    if (!str)
        return "null";
    return fmt::format("\"{}\"", str);
}

std::vector<ref<const DeclReflection>> DeclReflection::children() const
{
    std::vector<ref<const DeclReflection>> res;
    int32_t count = child_count();
    res.reserve(count);
    for (int32_t i = 0; i < count; i++) {
        res.push_back(detail::from_slang(m_owner, base()->getChild(i)));
    }
    return res;
}

std::vector<ref<const DeclReflection>> DeclReflection::children_of_kind(Kind kind) const
{
    std::vector<ref<const DeclReflection>> res;
    int32_t count = child_count();
    res.reserve(count);
    for (int32_t i = 0; i < count; i++) {
        ref<const DeclReflection> child = detail::from_slang(m_owner, base()->getChild(i));
        if (child->kind() == kind) {
            res.push_back(child);
        }
    }
    return res;
}

std::string DeclReflection::to_string() const
{
    std::string str;
    str += "DeclReflection(\n";
    str += fmt::format("  kind={},\n", kind());
    if (kind() == Kind::variable || kind() == Kind::func || kind() == Kind::struct_)
        str += fmt::format("  name={},\n", name());
    str += ")";
    return str;
}

ref<const TypeReflection> DeclReflection::as_type() const
{
    return detail::from_slang(m_owner, base()->getType());
}

std::string DeclReflection::name() const
{
    switch (kind()) {
    case Kind::variable:
        return as_variable()->name();
    case Kind::func:
        return as_function()->name();
    case Kind::struct_:
        return as_type()->name();
    default:
        SGL_THROW("Invalid decl kind to request name: {}", kind());
    }
}

std::vector<ref<const DeclReflection>>
DeclReflection::find_children_of_kind(Kind kind, std::string_view child_name) const
{
    std::vector<ref<const DeclReflection>> res;
    int32_t count = child_count();
    res.reserve(count);
    for (int32_t i = 0; i < count; i++) {
        ref<const DeclReflection> child = detail::from_slang(m_owner, base()->getChild(i));
        if (child->kind() == kind && child->name() == child_name) {
            res.push_back(child);
        }
    }
    return res;
}

ref<const DeclReflection> DeclReflection::find_first_child_of_kind(Kind kind, std::string_view child_name) const
{
    std::vector<ref<const DeclReflection>> res;
    int32_t count = child_count();
    res.reserve(count);
    for (int32_t i = 0; i < count; i++) {
        ref<const DeclReflection> child = detail::from_slang(m_owner, base()->getChild(i));
        if (child->kind() == kind && child->name() == child_name) {
            return child;
        }
    }
    return nullptr;
}

std::string TypeReflection::to_string() const
{
    std::string str;
    str += "TypeReflection(\n";
    str += fmt::format("  kind={},\n", kind());
    str += fmt::format("  name={},\n", c_str_to_string(name()));
    // str += fmt::format("  fields={},", vector_to_string(fields()));
    // str += fmt::format("  scalar_type={},\n", scalar_type());
    // str += fmt::format("  row_count={},\n", row_count());
    // str += fmt::format("  col_count={},\n", col_count());
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

ReflectionCursor::ReflectionCursor(ref<const EntryPointLayout> entry_point_layout)
    : m_entry_point_layout(entry_point_layout)
    , m_valid(m_entry_point_layout != nullptr)
{
}

ReflectionCursor::ReflectionCursor(ref<const TypeLayoutReflection> type_layout)
    : m_type_layout(type_layout)
    , m_valid(m_type_layout != nullptr)
{
}

ReflectionCursor ReflectionCursor::operator[](std::string_view name) const
{
    SGL_CHECK(is_valid(), "Invalid cursor");
    ReflectionCursor result = find_field(name);
    SGL_CHECK(result.is_valid(), "Field \"{}\" not found.", name);
    return result;
}

ReflectionCursor ReflectionCursor::operator[](uint32_t index) const
{
    SGL_CHECK(is_valid(), "Invalid cursor");
    ReflectionCursor result = find_element(index);
    SGL_CHECK(result.is_valid(), "Element {} not found.", index);
    return result;
}

ReflectionCursor ReflectionCursor::find_field(std::string_view name) const
{
    if (m_shader_program) {
        // Try to find field in global variables.
        if (auto global_field = ReflectionCursor(m_shader_program->layout()->globals_type_layout()).find_field(name);
            global_field.is_valid())
            return global_field;
        // Try to find an entry point.
        if (ref<const EntryPointLayout> entry_point_layout
            = m_shader_program->layout()->find_entry_point_by_name(name)) {
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
    SGL_UNUSED(index);
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

} // namespace sgl
