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

    ShaderCursor(ShaderObject* shader_object);

    const TypeLayoutReflection* type_layout() const { return m_type_layout; }
    const TypeReflection* type() const { return m_type_layout->type(); }

    bool is_valid() const { return m_offset.is_valid(); }

    std::string to_string() const;

    ShaderCursor dereference() const;

    //
    // Navigation
    //

    ShaderCursor operator[](std::string_view name) const;
    ShaderCursor operator[](uint32_t index) const;

    ShaderCursor find_field(std::string_view name) const;
    ShaderCursor find_element(uint32_t index) const;

    ShaderCursor find_entry_point(uint32_t index) const;

    bool has_field(std::string_view name) const { return find_field(name).is_valid(); }
    bool has_element(uint32_t index) const { return find_element(index).is_valid(); }

    //
    // Resource binding
    //

    void set_object(const ref<MutableShaderObject>& object) const;

    void set_resource(const ref<ResourceView>& resource_view) const;
    void set_buffer(const ref<Buffer>& buffer) const;
    void set_texture(const ref<Texture>& texture) const;
    void set_sampler(const ref<Sampler>& sampler) const;
    void set_acceleration_structure(const ref<AccelerationStructure>& acceleration_structure) const;

    void set_data(const void* data, size_t size) const;

    template<typename T>
    void operator=(const T& value) const
    {
        set(value);
    }

private:
    void set_scalar(const void* data, size_t size, TypeReflection::ScalarType scalar_type) const;
    void set_vector(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension) const;
    void set_matrix(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const;

    template<typename T>
    void set(const T& value) const;

    ShaderObject* m_shader_object{nullptr};
    const TypeLayoutReflection* m_type_layout{nullptr};
    ShaderOffset m_offset;
};

} // namespace kali
