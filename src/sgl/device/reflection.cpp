// SPDX-License-Identifier: Apache-2.0

#include "reflection.h"

#include "sgl/device/device.h"
#include "sgl/device/shader.h"

#include "sgl/core/string.h"

#include "sgl/math/vector.h"

#include <span>

namespace sgl {

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
        if (const EntryPointLayout* entry_point_layout = m_shader_program->layout()->find_entry_point_by_name(name)) {
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

ASTCursor::ASTCursor(ref<SlangModule> module, slang::DeclReflection* decl_ref)
    : m_module(module)
    , m_decl_ref(decl_ref)
{
}

ASTCursor::~ASTCursor()
{
    // TODO: Remove this when finished debugging destruction of cursors!
}

std::string ASTCursor::to_string() const
{
    return fmt::format(
        "ASTCursor(module={}, kind={}, children={})",
        m_module->name(),
        kind(),
        m_decl_ref->getChildrenCount()
    );
}

template<typename T>
std::vector<ref<T>> ASTCursor::nodes_of_kind(slang::DeclReflection::Kind kind) const
{
    std::vector<ref<T>> res;
    uint32_t num_children = m_decl_ref->getChildrenCount();
    for (uint32_t idx = 0; idx < num_children; idx++) {
        slang::DeclReflection* child_decl_ref = m_decl_ref->getChild(idx);
        if (!child_decl_ref)
            SGL_THROW("Slang returned a null decl ref - this should never happen");
        if (child_decl_ref->getKind() == kind)
            res.push_back(make_ref<T>(m_module, child_decl_ref));
    }
    return res;
}

std::vector<ref<ASTCursorFunction>> ASTCursor::_find_func(std::string_view name) const
{
    std::vector<ref<ASTCursorFunction>> res;
    uint32_t num_children = m_decl_ref->getChildrenCount();
    for (uint32_t idx = 0; idx < num_children; idx++) {
        slang::DeclReflection* child_decl_ref = m_decl_ref->getChild(idx);
        if (!child_decl_ref)
            SGL_THROW("Slang returned a null decl ref - this should never happen");
        if (auto func = child_decl_ref->asFunction())
            if (func->getName() == name)
                res.push_back(make_ref<ASTCursorFunction>(m_module, child_decl_ref));
    }
    return res;
}

ref<ASTCursorFunction> ASTCursor::_find_first_func(std::string_view name) const
{
    uint32_t num_children = m_decl_ref->getChildrenCount();
    for (uint32_t idx = 0; idx < num_children; idx++) {
        slang::DeclReflection* child_decl_ref = m_decl_ref->getChild(idx);
        if (!child_decl_ref)
            SGL_THROW("Slang returned a null decl ref - this should never happen");
        if (auto func = child_decl_ref->asFunction())
            if (func->getName() == name)
                return make_ref<ASTCursorFunction>(m_module, child_decl_ref);
    }
    return nullptr;
}

std::vector<ref<ASTCursorVariable>> ASTCursor::_find_variable(std::string_view name) const
{
    std::vector<ref<ASTCursorVariable>> res;
    uint32_t num_children = m_decl_ref->getChildrenCount();
    for (uint32_t idx = 0; idx < num_children; idx++) {
        slang::DeclReflection* child_decl_ref = m_decl_ref->getChild(idx);
        if (!child_decl_ref)
            SGL_THROW("Slang returned a null decl ref - this should never happen");
        if (auto var = child_decl_ref->asVariable())
            if (var->getName() == name)
                res.push_back(make_ref<ASTCursorVariable>(m_module, child_decl_ref));
    }
    return res;
}

ref<ASTCursorVariable> ASTCursor::_find_first_variable(std::string_view name) const
{
    uint32_t num_children = m_decl_ref->getChildrenCount();
    for (uint32_t idx = 0; idx < num_children; idx++) {
        slang::DeclReflection* child_decl_ref = m_decl_ref->getChild(idx);
        if (!child_decl_ref)
            SGL_THROW("Slang returned a null decl ref - this should never happen");
        if (auto var = child_decl_ref->asVariable())
            if (var->getName() == name)
                return make_ref<ASTCursorVariable>(m_module, child_decl_ref);
    }
    return nullptr;
}

std::vector<ref<ASTCursor>> ASTCursor::children() const
{
    std::vector<ref<ASTCursor>> res;
    uint32_t num_children = m_decl_ref->getChildrenCount();
    for (uint32_t idx = 0; idx < num_children; idx++) {
        slang::DeclReflection* child_decl_ref = m_decl_ref->getChild(idx);
        if (!child_decl_ref)
            SGL_THROW("Slang returned a null decl ref - this should never happen");
        res.push_back(from_decl(m_module, child_decl_ref));
    }
    return res;
}

ref<ASTCursor> ASTCursor::operator[](int32_t index) const
{
    if (index > child_count())
        SGL_THROW("Index out of range - index={}, child count={}", index, child_count());
    return from_decl(m_module, m_decl_ref->getChild(index));
}

ref<ASTCursor> ASTCursor::from_decl(ref<SlangModule> module, slang::DeclReflection* decl_ref)
{
    // Return correct derived type for the decl type, or just another ASTCursor if
    // it's an unsupported decl type.
    switch (decl_ref->getKind()) {
    case slang::DeclReflection::Kind::Func:
        return ref((ASTCursor*)new ASTCursorFunction(module, decl_ref));
    case slang::DeclReflection::Kind::Struct:
        return ref((ASTCursor*)new ASTCursorStruct(module, decl_ref));
    case slang::DeclReflection::Kind::Variable:
        return ref((ASTCursor*)new ASTCursorVariable(module, decl_ref));
    case slang::DeclReflection::Kind::Module:
        return ref((ASTCursor*)new ASTCursorModule(module, decl_ref));
    case slang::DeclReflection::Kind::Generic:
        return ref((ASTCursor*)new ASTCursorGeneric(module, decl_ref));
    default:
        return make_ref<ASTCursor>(module, decl_ref);
    }
}

std::string ASTCursorModule::name() const
{
    return m_module->name();
}

std::vector<ref<ASTCursorVariable>> ASTCursorModule::globals() const
{
    return nodes_of_kind<ASTCursorVariable>(slang::DeclReflection::Kind::Variable);
}

std::vector<ref<ASTCursorFunction>> ASTCursorModule::functions() const
{
    return nodes_of_kind<ASTCursorFunction>(slang::DeclReflection::Kind::Func);
}

std::vector<ref<ASTCursorStruct>> ASTCursorModule::structs() const
{
    return nodes_of_kind<ASTCursorStruct>(slang::DeclReflection::Kind::Struct);
}

std::vector<ref<ASTCursorVariable>> ASTCursorStruct::fields() const
{
    return nodes_of_kind<ASTCursorVariable>(slang::DeclReflection::Kind::Variable);
}

std::vector<ref<ASTCursorFunction>> ASTCursorStruct::functions() const
{
    return nodes_of_kind<ASTCursorFunction>(slang::DeclReflection::Kind::Func);
}

std::vector<ref<ASTCursorStruct>> ASTCursorStruct::structs() const
{
    return nodes_of_kind<ASTCursorStruct>(slang::DeclReflection::Kind::Struct);
}

const TypeReflection* ASTCursorStruct::base() const
{
    if (!m_type_decl)
        m_type_decl = detail::from_slang(m_decl_ref->getType(m_module->session()->device()->global_session()));
    return m_type_decl;
}

std::vector<ref<ASTCursorVariable>> ASTCursorFunction::parameters() const
{
    return nodes_of_kind<ASTCursorVariable>(slang::DeclReflection::Kind::Variable);
}

} // namespace sgl
