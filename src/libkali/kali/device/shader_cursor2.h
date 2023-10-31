#pragma once

#include "kali/device/fwd.h"
#include "kali/device/shader_offset.h"
#include "kali/device/reflection.h"

#include "kali/core/macros.h"

#include <string_view>

namespace kali {

class KALI_API ShaderCursor {
public:
    ShaderCursor() = default;

    ShaderCursor(ShaderObject* object_, TypeReflection* type_, ShaderOffset offset_ = ShaderOffset::zero())
        : m_object(object_)
        , m_type(type_)
        , m_offset(offset_)
    {
        KALI_ASSERT(m_object);
        KALI_ASSERT(m_type);
        KALI_ASSERT(m_offset.is_valid());
    }

    bool is_valid() const { return m_offset.is_valid(); }

    //
    // Navigation
    //

    ShaderCursor operator[](std::string_view name) const;
    ShaderCursor operator[](uint32_t index) const;

    ShaderCursor find_member(std::string_view name) const;
    ShaderCursor find_element(uint32_t index) const;

    bool has_member(std::string_view name) const { return find_member(name).is_valid(); }
    bool has_element(uint32_t index) const { return find_element(index).is_valid(); }

    //
    // Resource binding
    //

    void set_buffer(const ref<Buffer>& buffer) const;
    void set_texture(const ref<Texture>& texture) const;
    void set_resource(const ref<ResourceView>& resource_view) const;
    void set_sampler(const ref<Sampler>& sampler) const;

    template<typename T>
    void operator=(const T& value) const
    {
        set(value);
    }

private:
    template<typename T>
    void set(const T& value) const;

    ShaderObject* m_object{nullptr};
    TypeReflection* m_type{nullptr};
    ShaderOffset m_offset;
};

template<>
inline void ShaderCursor::set(const ref<Buffer>& value) const
{
    set_buffer(value);
}

template<>
inline void ShaderCursor::set(const ref<Texture>& value) const
{
    set_texture(value);
}

template<>
inline void ShaderCursor::set(const ref<ResourceView>& value) const
{
    set_resource(value);
}

template<>
inline void ShaderCursor::set(const ref<Sampler>& value) const
{
    set_sampler(value);
}

} // namespace kali
