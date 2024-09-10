#pragma once

#include <optional>

#include "nanobind.h"

#include "sgl/device/reflection.h"
#include "sgl/device/cursor_utils.h"

#include "sgl/math/vector_types.h"
#include "sgl/math/matrix_types.h"

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
    requires TraversableCursor<CursorType>
inline void bind_traversable_cursor(nanobind::class_<CursorType>& cursor)
{
    cursor //
        .def_prop_ro("type_layout", &ShaderCursor::type_layout, D(ShaderCursor, type_layout))
        .def_prop_ro("type", &ShaderCursor::type, D(ShaderCursor, type))
        .def("is_valid", &ShaderCursor::is_valid, D(ShaderCursor, is_valid))
        .def("find_field", &CursorType::find_field, "name"_a, D(ShaderCursor, find_field))
        .def("find_element", &CursorType::find_element, "index"_a, D(ShaderCursor, find_element))
        .def("has_field", &ShaderCursor::has_field, "name"_a, D(ShaderCursor, has_field))
        .def("has_element", &ShaderCursor::has_element, "index"_a, D(ShaderCursor, has_element))
        .def("__getitem__", [](ShaderCursor& self, std::string_view name) { return self[name]; })
        .def("__getitem__", [](ShaderCursor& self, int index) { return self[index]; })
        .def("__getattr__", [](ShaderCursor& self, std::string_view name) { return self[name]; });
}

template<typename CursorType>
    requires WritableCursor<CursorType>
inline void bind_writable_cursor_basic_types(nanobind::class_<CursorType>& cursor)
{
#define def_setter(type)                                                                                               \
    cursor.def("__setitem__", [](CursorType& self, std::string_view name, type value) { self[name] = value; });        \
    cursor.def("__setattr__", [](CursorType& self, std::string_view name, type value) { self[name] = value; });

    def_setter(bool);
    def_setter(bool2);
    def_setter(bool3);
    def_setter(bool4);

    def_setter(uint2);
    def_setter(uint3);
    def_setter(uint4);

    def_setter(int2);
    def_setter(int3);
    def_setter(int4);

    def_setter(float2);
    def_setter(float3);
    def_setter(float4);

    def_setter(float2x2);
    def_setter(float3x3);
    def_setter(float2x4);
    def_setter(float3x4);
    def_setter(float4x4);

    def_setter(float16_t2);
    def_setter(float16_t3);
    def_setter(float16_t4);

#undef def_setter

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

    auto set_float_field = [](CursorType& self, std::string_view name, nb::float_ value)
    {
        ref<const TypeReflection> type = self[name].type();
        SGL_CHECK(type->kind() == TypeReflection::Kind::scalar, "Field \"{}\" is not a scalar type.", name);
        switch (type->scalar_type()) {
        case TypeReflection::ScalarType::float16:
            self[name] = float16_t(nb::cast<float>(value));
            break;
        case TypeReflection::ScalarType::float32:
            self[name] = nb::cast<float>(value);
            break;
        case TypeReflection::ScalarType::float64:
            self[name] = nb::cast<double>(value);
            break;
        default:
            SGL_THROW("Field \"{}\" is not a floating point type.");
            break;
        }
    };

    auto set_float_element = [](CursorType& self, int index, nb::float_ value)
    {
        ref<const TypeReflection> type = self[index].type();
        SGL_CHECK(type->kind() == TypeReflection::Kind::scalar, "Element {} is not a scalar type.", index);
        switch (type->scalar_type()) {
        case TypeReflection::ScalarType::float16:
            self[index] = float16_t(nb::cast<float>(value));
            break;
        case TypeReflection::ScalarType::float32:
            self[index] = nb::cast<float>(value);
            break;
        case TypeReflection::ScalarType::float64:
            self[index] = nb::cast<double>(value);
            break;
        default:
            SGL_THROW("Element {} is not a floating point type.");
            break;
        }
    };

    cursor.def("__setitem__", set_float_field);
    cursor.def("__setitem__", set_float_element);
    cursor.def("__setattr__", set_float_field);

    auto set_numpy_field = [](CursorType& self, std::string_view name, nb::ndarray<nb::numpy> value)
    {
        ref<const TypeReflection> type = self[name].type();
        auto src_scalar_type = dtype_to_scalar_type(value.dtype());
        SGL_CHECK(src_scalar_type, "numpy array has unsupported dtype.");
        SGL_CHECK(is_ndarray_contiguous(value), "numpy array is not contiguous.");

        switch (type->kind()) {
        case TypeReflection::Kind::array:
            SGL_CHECK(value.ndim() == 1, "numpy array must have 1 dimension.");
            self[name]._set_array(value.data(), value.nbytes(), *src_scalar_type, narrow_cast<int>(value.shape(0)));
            break;
        case TypeReflection::Kind::matrix:
            SGL_CHECK(value.ndim() == 2, "numpy array must have 2 dimensions.");
            self[name]._set_matrix(
                value.data(),
                value.nbytes(),
                *src_scalar_type,
                narrow_cast<int>(value.shape(0)),
                narrow_cast<int>(value.shape(1))
            );
            break;
        case TypeReflection::Kind::vector: {
            SGL_CHECK(value.ndim() == 1 || value.ndim() == 2, "numpy array must have 1 or 2 dimensions.");
            size_t dimension = 1;
            for (size_t i = 0; i < value.ndim(); ++i)
                dimension *= value.shape(i);
            self[name]._set_vector(value.data(), value.nbytes(), *src_scalar_type, narrow_cast<int>(dimension));
            break;
        }
        default:
            SGL_THROW("Field \"{}\" is not a vector, matrix, or array type.", name);
        }
    };

    cursor.def("__setitem__", set_numpy_field);
    cursor.def("__setattr__", set_numpy_field);
}

} // namespace sgl
