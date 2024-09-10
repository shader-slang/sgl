// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/shader_offset.h"
#include "sgl/device/reflection.h"
#include "sgl/device/cursor_utils.h"

#include "sgl/core/config.h"
#include "sgl/core/macros.h"

#include <string_view>

namespace sgl {

/// Represents a single element of a given type in a block of memory, and
/// provides read/write tools to access its members via reflection.
class SGL_API BufferElementCursor : public Object {
public:
    BufferElementCursor() = default;

    /// Create with none-owning view of specific block of memory
    BufferElementCursor(ref<TypeLayoutReflection> layout, void* data, size_t size);

    ref<const TypeLayoutReflection> type_layout() const { return m_type_layout; }
    ref<const TypeReflection> type() const { return m_type_layout->type(); }

    size_t offset() const { return m_offset; }

    bool is_valid() const { return m_buffer != nullptr; }

    std::string to_string() const;

    BufferElementCursor operator[](std::string_view name) const;
    BufferElementCursor operator[](uint32_t index) const;

    BufferElementCursor find_field(std::string_view name) const;
    BufferElementCursor find_element(uint32_t index) const;

    bool has_field(std::string_view name) const { return find_field(name).is_valid(); }
    bool has_element(uint32_t index) const { return find_element(index).is_valid(); }

    void set_data(const void* data, size_t size) const;

    std::span<const uint8_t> data() const { return {m_buffer, m_size}; }
    std::span<uint8_t> data() { return {m_buffer, m_size}; }

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

    template<typename T>
    void set(const T& value) const;

    void _set_array(const void* data, size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) const;
    void _set_scalar(const void* data, size_t size, TypeReflection::ScalarType scalar_type) const;
    void _set_vector(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension) const;
    void _set_matrix(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const;

    void _get_array(void* data, size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) const;
    void _get_scalar(void* data, size_t size, TypeReflection::ScalarType scalar_type) const;
    void _get_vector(void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension) const;
    void _get_matrix(void* data, size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const;

private:
    void write_data(size_t offset, const void* data, size_t size) const;
    void read_data(size_t offset, void* data, size_t size) const;

    ref<const TypeLayoutReflection> m_type_layout;
    uint8_t* m_buffer{nullptr};
    size_t m_size{0};
    size_t m_offset{0};

    friend class BufferCursor;
};

/// Represents a list of elements in a block of memory, and provides
/// simple interface to get a BufferElementCursor for each one.
class SGL_API BufferCursor : public Object {
public:
    BufferCursor() = default;

    /// Create with none-owning view of specific block of memory. Number of
    /// elements is inferred from the size of the block and the type layout.
    BufferCursor(ref<TypeLayoutReflection> layout, void* data, size_t size);

    /// Create buffer + allocate space internally for a given number of elements.
    BufferCursor(ref<TypeLayoutReflection> layout, size_t element_count);

    ~BufferCursor();

    BufferElementCursor operator[](uint32_t index) const;

    size_t element_count() const { return m_size / element_size(); }

    size_t element_size() const { return m_type_layout->size(); }

    std::span<const uint8_t> data() const { return {m_buffer, m_size}; }
    std::span<uint8_t> data() { return {m_buffer, m_size}; }

private:
    ref<const TypeLayoutReflection> m_type_layout;
    uint8_t* m_buffer{nullptr};
    size_t m_size{0};
    bool m_owner{false};
};


} // namespace sgl
