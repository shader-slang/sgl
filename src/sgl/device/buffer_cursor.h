// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/shader_offset.h"
#include "sgl/device/reflection.h"
#include "sgl/device/base_cursor.h"

#include "sgl/core/config.h"
#include "sgl/core/macros.h"

#include <string_view>

namespace sgl {

class SGL_API BufferCursor : public BaseCursor {
public:
    BufferCursor() = default;

    BufferCursor(ref<TypeLayoutReflection> layout, void* data, size_t size);

    ref<const TypeLayoutReflection> type_layout() const { return m_type_layout; }
    ref<const TypeReflection> type() const { return m_type_layout->type(); }

    size_t offset() const { return m_offset; }

    bool is_valid() const { return m_buffer != nullptr; }

    std::string to_string() const;

    BufferCursor operator[](std::string_view name) const;
    BufferCursor operator[](uint32_t index) const;

    BufferCursor find_field(std::string_view name) const;
    BufferCursor find_element(uint32_t index) const;

    bool has_field(std::string_view name) const { return find_field(name).is_valid(); }
    bool has_element(uint32_t index) const { return find_element(index).is_valid(); }

    void set_data(const void* data, size_t size) const;

    template<typename T>
    void operator=(const T& value) const
    {
        set(value);
    }

    template<typename T>
    T as() const
    {
        T value;
        get(value);
        return value;
    }

    template<typename T>
    void get(T& value) const;

    void _set_array(const void* data, size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) const;
    void _set_scalar(const void* data, size_t size, TypeReflection::ScalarType scalar_type) const;
    void _set_vector(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension) const;
    void _set_matrix(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const;

    void _get_array(void* data, size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) const;
    void _get_scalar(void* data, size_t size, TypeReflection::ScalarType scalar_type) const;
    void _get_vector(void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension) const;
    void _get_matrix(void* data, size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const;

private:
    template<typename T>
    void set(const T& value) const;


    void write_data(size_t offset, const void* data, size_t size) const;
    void read_data(size_t offset, void* data, size_t size) const;

    uint8_t* m_buffer{nullptr};
    size_t m_size{0};
    size_t m_offset{0};
};

} // namespace sgl
