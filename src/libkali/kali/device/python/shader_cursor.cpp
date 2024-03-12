// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "kali/device/shader_cursor.h"
#include "kali/device/shader_object.h"
#include "kali/device/resource.h"
#include "kali/device/sampler.h"
#include "kali/device/raytracing.h"
#include "kali/device/cuda_interop.h"

#include "kali/math/vector_types.h"
#include "kali/math/matrix_types.h"

KALI_PY_EXPORT(device_shader_cursor)
{
    using namespace kali;

    nb::class_<ShaderOffset>(m, "ShaderOffset")
        .def_ro("uniform_offset", &ShaderOffset::uniform_offset)
        .def_ro("binding_range_index", &ShaderOffset::binding_range_index)
        .def_ro("binding_array_index", &ShaderOffset::binding_array_index)
        .def("is_valid", &ShaderOffset::is_valid);

    nb::class_<ShaderCursor> shader_cursor(m, "ShaderCursor", D(ShaderCursor));

    shader_cursor //
        .def(nb::init<ShaderObject*>(), "shader_object"_a, D(ShaderCursor, ShaderCursor))
        .def_prop_ro("type_layout", &ShaderCursor::type_layout, D(ShaderCursor, type_layout))
        .def_prop_ro("type", &ShaderCursor::type, D(ShaderCursor, type))
        .def_prop_ro("offset", &ShaderCursor::offset, D(ShaderCursor, offset))
        .def("is_valid", &ShaderCursor::is_valid, D(ShaderCursor, is_valid))
        .def("dereference", &ShaderCursor::dereference, D(ShaderCursor, dereference))
        .def("find_field", &ShaderCursor::find_field, "name"_a, D(ShaderCursor, find_field))
        .def("find_element", &ShaderCursor::find_element, "index"_a, D(ShaderCursor, find_element))
        .def("find_entry_point", &ShaderCursor::find_entry_point, "index"_a, D(ShaderCursor, find_entry_point))
        .def("has_field", &ShaderCursor::has_field, "name"_a, D(ShaderCursor, has_field))
        .def("has_element", &ShaderCursor::has_element, "index"_a, D(ShaderCursor, has_element))
        .def("set_object", &ShaderCursor::set_object, "object"_a, D(ShaderCursor, set_object))
        .def("set_resource", &ShaderCursor::set_resource, "resource_view"_a, D(ShaderCursor, set_resource))
        .def("set_buffer", &ShaderCursor::set_buffer, "buffer"_a, D(ShaderCursor, set_buffer))
        .def("set_texture", &ShaderCursor::set_texture, "texture"_a, D(ShaderCursor, set_texture))
        .def("set_sampler", &ShaderCursor::set_sampler, "sampler"_a, D(ShaderCursor, set_sampler))
        .def(
            "set_acceleration_structure",
            &ShaderCursor::set_acceleration_structure,
            "acceleration_structure"_a,
            D(ShaderCursor, set_acceleration_structure)
        )
        .def(
            "set_data",
            [](ShaderCursor& self, nb::ndarray<nb::device::cpu> data)
            {
                KALI_CHECK(is_ndarray_contiguous(data), "data is not contiguous");
                self.set_data(data.data(), data.nbytes());
            },
            "data"_a,
            D(ShaderCursor, set_data)
        );

    shader_cursor.def("__getitem__", [](ShaderCursor& self, std::string_view name) { return self[name]; });
    shader_cursor.def("__getitem__", [](ShaderCursor& self, int index) { return self[index]; });
    shader_cursor.def("__getattr__", [](ShaderCursor& self, std::string_view name) { return self[name]; });

#define def_setter(type)                                                                                               \
    shader_cursor.def(                                                                                                 \
        "__setitem__",                                                                                                 \
        [](ShaderCursor& self, std::string_view name, type value) { self[name] = value; }                              \
    );                                                                                                                 \
    shader_cursor.def("__setattr__", [](ShaderCursor& self, std::string_view name, type value) { self[name] = value; });

    def_setter(ref<ResourceView>);
    def_setter(ref<Buffer>);
    def_setter(ref<Texture>);
    def_setter(ref<Sampler>);
    def_setter(ref<AccelerationStructure>);

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

    auto set_int_field = [](ShaderCursor& self, std::string_view name, nb::int_ value)
    {
        const TypeReflection* type = self[name].type();
        KALI_CHECK(type->kind() == TypeReflection::Kind::scalar, "Field \"{}\" is not a scalar type.", name);
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
            KALI_THROW("Field \"{}\" is not an integer type.");
            break;
        }
    };

    auto set_int_element = [](ShaderCursor& self, int index, nb::int_ value)
    {
        const TypeReflection* type = self[index].type();
        KALI_CHECK(type->kind() == TypeReflection::Kind::scalar, "Element {} is not a scalar type.", index);
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
            KALI_THROW("Element {} is not an integer type.");
            break;
        }
    };

    shader_cursor.def("__setitem__", set_int_field);
    shader_cursor.def("__setitem__", set_int_element);
    shader_cursor.def("__setattr__", set_int_field);

    auto set_float_field = [](ShaderCursor& self, std::string_view name, nb::float_ value)
    {
        const TypeReflection* type = self[name].type();
        KALI_CHECK(type->kind() == TypeReflection::Kind::scalar, "Field \"{}\" is not a scalar type.", name);
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
            KALI_THROW("Field \"{}\" is not a floating point type.");
            break;
        }
    };

    auto set_float_element = [](ShaderCursor& self, int index, nb::float_ value)
    {
        const TypeReflection* type = self[index].type();
        KALI_CHECK(type->kind() == TypeReflection::Kind::scalar, "Element {} is not a scalar type.", index);
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
            KALI_THROW("Element {} is not a floating point type.");
            break;
        }
    };

    shader_cursor.def("__setitem__", set_float_field);
    shader_cursor.def("__setitem__", set_float_element);
    shader_cursor.def("__setattr__", set_float_field);

    auto set_cuda_tensor_field = [](ShaderCursor& self, std::string_view name, nb::ndarray<nb::device::cuda> ndarray)
    { self[name].set_cuda_tensor_view(ndarray_to_cuda_tensor_view(ndarray)); };

    auto set_cuda_tensor_element = [](ShaderCursor& self, int index, nb::ndarray<nb::device::cuda> ndarray)
    { self[index].set_cuda_tensor_view(ndarray_to_cuda_tensor_view(ndarray)); };

    shader_cursor.def("__setitem__", set_cuda_tensor_field);
    shader_cursor.def("__setitem__", set_cuda_tensor_element);
    shader_cursor.def("__setattr__", set_cuda_tensor_field);
}
