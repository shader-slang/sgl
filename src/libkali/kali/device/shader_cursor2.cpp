#include "shader_cursor2.h"

#include "kali/core/error.h"

namespace kali {

//
// Navigation
//

ShaderCursor ShaderCursor::operator[](std::string_view name) const
{
    KALI_CHECK(is_valid(), "Invalid cursor");
    ShaderCursor result = find_member(name);
    KALI_CHECK(result.is_valid(), "Member '{}' not found.", name);
    return result;
}

ShaderCursor ShaderCursor::operator[](uint32_t index) const
{
    KALI_CHECK(is_valid(), "Invalid cursor");
    ShaderCursor result = find_element(index);
    KALI_CHECK(result.is_valid(), "Element '{}' not found.", index);
    return result;
}

ShaderCursor ShaderCursor::find_member(std::string_view name) const
{
    if (!is_valid())
        return *this;

    if (const StructTypeReflection* struct_type = m_type->as_struct_type()) {
        const VariableReflection* member = struct_type->find_member(name);
        if (!member)
            return {};
    }

    return {};
}

ShaderCursor ShaderCursor::find_element(uint32_t index) const
{
    if (!is_valid())
        return *this;

    KALI_UNUSED(index);
    return {};
}

//
// Resource binding
//

void ShaderCursor::set_buffer(const ref<Buffer>& buffer) const
{
    KALI_UNUSED(buffer);
}

void ShaderCursor::set_texture(const ref<Texture>& texture) const
{
    KALI_UNUSED(texture);
}

void ShaderCursor::set_resource(const ref<ResourceView>& resource_view) const
{
    KALI_UNUSED(resource_view);
}

void ShaderCursor::set_sampler(const ref<Sampler>& sampler) const
{
    KALI_UNUSED(sampler);
}


} // namespace kali
