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
class SGL_API BufferElementCursor {
public:
    BufferElementCursor() = default;

    /// Create with none-owning view of specific block of memory
    BufferElementCursor(ref<TypeLayoutReflection> layout, ref<BufferCursor> owner);

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

    void set_data(const void* data, size_t size);

    template<typename T>
    void operator=(const T& value)
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
    void set(const T& value);

    void _set_array(const void* data, size_t size, TypeReflection::ScalarType scalar_type, size_t element_count);
    void _set_scalar(const void* data, size_t size, TypeReflection::ScalarType scalar_type);
    void _set_vector(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension);
    void _set_matrix(const void* data, size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols);

    void _get_array(void* data, size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) const;
    void _get_scalar(void* data, size_t size, TypeReflection::ScalarType scalar_type) const;
    void _get_vector(void* data, size_t size, TypeReflection::ScalarType scalar_type, int dimension) const;
    void _get_matrix(void* data, size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const;

private:
    void write_data(size_t offset, const void* data, size_t size);
    void read_data(size_t offset, void* data, size_t size) const;

    ref<const TypeLayoutReflection> m_type_layout;
    ref<BufferCursor> m_buffer;
    size_t m_offset{0};

    friend class BufferCursor;
};

/// Represents a list of elements in a block of memory, and provides
/// simple interface to get a BufferElementCursor for each one. As
/// this can be the owner of its data, it is a ref counted object that
/// elements refer to.
class SGL_API BufferCursor : Object {
public:
    BufferCursor() = default;

    /// Create with none-owning view of specific block of memory. Number of
    /// elements is inferred from the size of the block and the type layout.
    BufferCursor(ref<TypeLayoutReflection> element_layout, void* data, size_t size);

    /// Create buffer + allocate space internally for a given number of elements.
    BufferCursor(ref<TypeLayoutReflection> element_layout, size_t element_count);

    /// Create as a view onto a buffer resource.
    BufferCursor(ref<TypeLayoutReflection> element_layout, ref<Buffer> resource);

    /// Create as a view onto a section of a buffer resource.
    BufferCursor(
        ref<TypeLayoutReflection> element_layout,
        ref<Buffer> resource,
        size_t element_count,
        size_t first_element
    );

    ~BufferCursor();

    /// Get type layout of an element of the cursor.
    ref<const TypeLayoutReflection> element_type_layout() const { return m_element_type_layout; }

    /// Get type of an element of the cursor.
    ref<const TypeReflection> element_type() const { return m_element_type_layout->type(); }

    /// Get element at a given index.
    BufferElementCursor find_element(uint32_t index);

    /// Index operator to get element at a given index.
    BufferElementCursor operator[](uint32_t index) { return find_element(index); }

    /// Number of elements in the buffer.
    size_t element_count() const { return size() / element_size(); }

    /// Size of element.
    size_t element_size() const { return m_element_type_layout->size(); }

    /// Size of whole buffer.
    size_t size() const { return m_size; }

    /// Check if internal buffer exists.
    bool is_loaded() const { return m_buffer != nullptr; }

    /// Write data to buffer (note: writes only to host memory).
    void write_data(size_t offset, const void* data, size_t size);

    /// Reads data from buffer (note: reads only from host memory).
    void read_data(size_t offset, void* data, size_t size) const;

    /// In case of GPU only buffers, loads all data from GPU.
    void load();

    /// In case of GPU only buffers, pushes all data to the GPU.
    void apply();

    /// Get the resource this cursor represents (if any).
    ref<Buffer> resource() const { return m_resource; }

private:
    ref<const TypeLayoutReflection> m_element_type_layout;
    ref<Buffer> m_resource;
    uint8_t* m_buffer{nullptr};
    size_t m_size{0};
    bool m_owner{false};
    DeviceOffset m_offset{0};
};


} // namespace sgl
