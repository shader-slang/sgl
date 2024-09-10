// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/device/buffer_cursor.h"
#include "sgl/math/vector_types.h"
#include "sgl/math/matrix_types.h"

SGL_PY_EXPORT(device_buffer_cursor)
{
    /*
    using namespace sgl;

    nb::class_<BufferElementCursor, BaseCursor> shader_cursor(m, "BufferElementCursor", D_NA(BufferElementCursor));

    shader_cursor //
        .def_prop_ro("offset", &BufferElementCursor::offset, D_NA(BufferElementCursor, offset))
        .def("find_field", &BufferElementCursor::find_field, "name"_a, D_NA(BufferElementCursor, find_field))
        .def("find_element", &BufferElementCursor::find_element, "index"_a, D_NA(BufferElementCursor, find_element))
        .def("has_field", &BufferElementCursor::has_field, "name"_a, D_NA(BufferElementCursor, has_field))
        .def("has_element", &BufferElementCursor::has_element, "index"_a, D_NA(BufferElementCursor, has_element))
        .def(
            "set_data",
            [](BufferElementCursor& self, nb::ndarray<nb::device::cpu> data)
            {
                SGL_CHECK(is_ndarray_contiguous(data), "data is not contiguous");
                self.set_data(data.data(), data.nbytes());
            },
            "data"_a,
            D_NA(BufferElementCursor, set_data)
        );

    shader_cursor.def("__getitem__", [](BufferElementCursor& self, std::string_view name) { return self[name]; });
    shader_cursor.def("__getitem__", [](BufferElementCursor& self, int index) { return self[index]; });
    shader_cursor.def("__getattr__", [](BufferElementCursor& self, std::string_view name) { return self[name]; });

#define def_setter(type)                                                                                               \
    shader_cursor.def(                                                                                                 \
        "__setitem__",                                                                                                 \
        [](BufferElementCursor& self, std::string_view name, type value) { self[name] = value; } \
    );                                                                                                                 \
    shader_cursor.def("__setattr__", [](BufferElementCursor& self, std::string_view name, type value) { self[name] =
value; });

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

    // We need to handle integers and floats specially.
    // Python only has an `int` and `float` type that can have different bit-width.
    // We use reflection data to convert the Python types to the correct types before assigning.

    auto set_int_field = [](BufferElementCursor& self, std::string_view name, nb::int_ value)
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

    auto set_int_element = [](BufferElementCursor& self, int index, nb::int_ value)
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

    shader_cursor.def("__setitem__", set_int_field);
    shader_cursor.def("__setitem__", set_int_element);
    shader_cursor.def("__setattr__", set_int_field);

    auto set_float_field = [](BufferElementCursor& self, std::string_view name, nb::float_ value)
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

    auto set_float_element = [](BufferElementCursor& self, int index, nb::float_ value)
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

    shader_cursor.def("__setitem__", set_float_field);
    shader_cursor.def("__setitem__", set_float_element);
    shader_cursor.def("__setattr__", set_float_field);

    auto set_numpy_field = [](BufferElementCursor& self, std::string_view name, nb::ndarray<nb::numpy> value)
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

    shader_cursor.def("__setitem__", set_numpy_field);
    shader_cursor.def("__setattr__", set_numpy_field);

    */
}
