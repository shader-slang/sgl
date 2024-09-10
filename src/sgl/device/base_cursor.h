// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/reflection.h"

#include "sgl/core/macros.h"

namespace sgl {

/// Base class for data cursors.
class SGL_API BaseCursor : Object {
public:
    BaseCursor() = default;

    BaseCursor(ref<const TypeLayoutReflection> layout);

    ref<const TypeLayoutReflection> type_layout() const { return m_type_layout; }
    ref<const TypeReflection> type() const { return m_type_layout->type(); }

protected:
    ref<const TypeLayoutReflection> m_type_layout{nullptr};

    void check_array(size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) const;
    void check_scalar(size_t size, TypeReflection::ScalarType scalar_type) const;
    void check_vector(size_t size, TypeReflection::ScalarType scalar_type, int dimension) const;
    void check_matrix(size_t size, TypeReflection::ScalarType scalar_type, int rows, int cols) const;
};

/// Concept that defines the requirements for a cursor that can be traversed using
/// field names and element indices. Each traversal function should return a new
/// cursor object that represents the field or element.
template<typename T>
concept TraversableCursor = requires(T obj, std::string_view name_idx, uint32_t el_index) {
    {
        obj[name_idx]
    } -> std::same_as<T>;
    {
        obj[el_index]
    } -> std::same_as<T>;
    {
        obj.find_field(name_idx)
    } -> std::same_as<T>;
    {
        obj.find_element(el_index)
    } -> std::same_as<T>;
    {
        obj.has_field(name_idx)
    } -> std::convertible_to<bool>;
    {
        obj.has_element(el_index)
    } -> std::convertible_to<bool>;
};

/// Dummy type to represent traits of a value that can be passed to a cursor get/set
struct _AnyCursorInputValue { };

/// Concept that defines the requirements for a cursor that can be read from.
template<typename T>
concept ReadableCursor
    = requires(T obj, const void* data, size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) {
          {
              obj.template get<_AnyCursorInputValue>({})
          } -> std::same_as<void>; // Ensure set() method exists
          {
              obj.template as<_AnyCursorInputValue>()
          } -> std::same_as<_AnyCursorInputValue>;
          {
              obj._get_array(data, size, scalar_type, element_count)
          } -> std::same_as<void>;
          {
              obj._get_scalar(data, size, scalar_type)
          } -> std::same_as<void>;
          {
              obj._get_vector(data, size, scalar_type, 0)
          } -> std::same_as<void>;
          {
              obj._get_matrix(data, size, scalar_type, 0, 0)
          } -> std::same_as<void>;
      };

/// Concept that defines the requirements for a cursor that can be written to.
template<typename T>
concept WritableCursor
    = requires(T obj, void* data, size_t size, TypeReflection::ScalarType scalar_type, size_t element_count) {
          {
              obj.template set<_AnyCursorInputValue>({})
          } -> std::same_as<void>;
          {
              obj.template operator=<_AnyCursorInputValue>({})
          } -> std::same_as<void>;
          {
              obj._set_array(data, size, scalar_type, element_count)
          } -> std::same_as<void>;
          {
              obj._set_scalar(data, size, scalar_type)
          } -> std::same_as<void>;
          {
              obj._set_vector(data, size, scalar_type, 0)
          } -> std::same_as<void>;
          {
              obj._set_matrix(data, size, scalar_type, 0, 0)
          } -> std::same_as<void>;
      };

} // namespace sgl
