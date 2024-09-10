#pragma once

#include <optional>

#include "nanobind.h"

#include "sgl/device/reflection.h"
#include "sgl/device/base_cursor.h"

namespace sgl {

inline std::optional<TypeReflection::ScalarType> dtype_to_scalar_type(nb::dlpack::dtype dtype)
{
    switch (dtype.code) {
    case uint8_t(nb::dlpack::dtype_code::Int):
        switch (dtype.bits) {
        case 8:
            return TypeReflection::ScalarType::int8;
        case 16:
            return TypeReflection::ScalarType::int16;
        case 32:
            return TypeReflection::ScalarType::int32;
        case 64:
            return TypeReflection::ScalarType::int64;
        }
        break;
    case uint8_t(nb::dlpack::dtype_code::UInt):
        switch (dtype.bits) {
        case 8:
            return TypeReflection::ScalarType::uint8;
        case 16:
            return TypeReflection::ScalarType::uint16;
        case 32:
            return TypeReflection::ScalarType::uint32;
        case 64:
            return TypeReflection::ScalarType::uint64;
        }
        break;
    case uint8_t(nb::dlpack::dtype_code::Float):
        switch (dtype.bits) {
        case 16:
            return TypeReflection::ScalarType::float16;
        case 32:
            return TypeReflection::ScalarType::float32;
        case 64:
            return TypeReflection::ScalarType::float64;
        }
        break;
    }
    return {};
}

template<typename CursorType>
    requires WritableCursor<CursorType>
inline void add_field_setters(nanobind::class_<CursorType, BaseCursor>& cursor)
{
    auto set_int_field = [](CursorType& self, std::string_view name, nb::int_ value)
    {
        ref<const TypeReflection> type = self[name].type();
        SGL_CHECK(type->kind() == TypeReflection::Kind::scalar, "Field \"{}\" is not a scalar type.", name);
        switch (type->scalar_type()) {
        case TypeReflection::ScalarType::int16:
        case TypeReflection::ScalarType::int32:
            self[name] = nb::cast<int32_t>(value);
            break;
        case TypeReflection::ScalarType::int64:
            self[name] = nb::cast<int64_t>(value);
            break;
        case TypeReflection::ScalarType::uint16:
        case TypeReflection::ScalarType::uint32:
            self[name] = nb::cast<uint32_t>(value);
            break;
        case TypeReflection::ScalarType::uint64:
            self[name] = nb::cast<uint64_t>(value);
            break;
        default:
            SGL_THROW("Field \"{}\" is not an integer type.");
            break;
        }
    };

    auto set_int_element = [](CursorType& self, int index, nb::int_ value)
    {
        ref<const TypeReflection> type = self[index].type();
        SGL_CHECK(type->kind() == TypeReflection::Kind::scalar, "Element {} is not a scalar type.", index);
        switch (type->scalar_type()) {
        case TypeReflection::ScalarType::int16:
        case TypeReflection::ScalarType::int32:
            self[index] = nb::cast<int32_t>(value);
            break;
        case TypeReflection::ScalarType::int64:
            self[index] = nb::cast<int64_t>(value);
            break;
        case TypeReflection::ScalarType::uint16:
        case TypeReflection::ScalarType::uint32:
            self[index] = nb::cast<uint32_t>(value);
            break;
        case TypeReflection::ScalarType::uint64:
            self[index] = nb::cast<uint64_t>(value);
            break;
        default:
            SGL_THROW("Element {} is not an integer type.");
            break;
        }
    };

    cursor.def("__setitem__", set_int_field);
    cursor.def("__setitem__", set_int_element);
    cursor.def("__setattr__", set_int_field);
}

} // namespace sgl
