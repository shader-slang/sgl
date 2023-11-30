#include "nanobind.h"

#include "kali/device/shader_cursor.h"
#include "kali/device/shader_object.h"
#include "kali/device/resource.h"
#include "kali/device/sampler.h"

#include "kali/math/vector_types.h"
#include "kali/math/matrix_types.h"

KALI_PY_EXPORT(device_shader_cursor)
{
    using namespace kali;

    nb::class_<ShaderCursor> shader_cursor(m, "ShaderCursor");

    shader_cursor.def(nb::init<ShaderObject*>(), "shader_object"_a);
    shader_cursor.def("is_valid", &ShaderCursor::is_valid);

    shader_cursor.def("__getitem__", [](ShaderCursor& self, std::string_view name) { return self[name]; });
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

    def_setter(uint2);
    def_setter(uint3);
    def_setter(uint4);

    def_setter(int2);
    def_setter(int3);
    def_setter(int4);

    def_setter(bool);
    def_setter(bool2);
    def_setter(bool3);
    def_setter(bool4);

    def_setter(float2);
    def_setter(float3);
    def_setter(float4);

    def_setter(float1x4);
    def_setter(float2x4);
    def_setter(float3x4);
    def_setter(float4x4);

#undef def_setter

#if 0
    // We need to handle integers and floats specially.
    // Python only has an `int` and `float` type that can have different bit-width.
    // We use reflection data to convert the python types to the correct types before assigning.

    auto set_int = [](ShaderCursor& self, std::string_view name, pybind11::int_ value)
    {
        const ReflectionBasicType* basicType = self[name].getType()->unwrapArray()->asBasicType();
        FALCOR_CHECK(basicType, "Error trying to set a variable that is not a basic type.");
        switch (basicType->getType()) {
        case ReflectionBasicType::Type::Int:
            self[name] = value.cast<int32_t>();
            break;
        case ReflectionBasicType::Type::Uint:
            self[name] = value.cast<uint32_t>();
            break;
        default:
            FALCOR_THROW("Error trying to set a variable that is not an integer type.");
            break;
        }
    };

    shader_cursor.def("__setitem__", set_int);
    shader_cursor.def("__setattr__", set_int);

    auto set_float = [](ShaderCursor& self, std::string_view name, pybind11::float_ value)
    {
        const ReflectionBasicType* basicType = self[name].getType()->unwrapArray()->asBasicType();
        FALCOR_CHECK(basicType, "Error trying to set a variable that is not a basic type.");
        switch (basicType->getType()) {
        case ReflectionBasicType::Type::Float:
            self[name] = value.cast<float>();
            break;
        default:
            FALCOR_THROW("Error trying to set a variable that is not an float type.");
            break;
        }
    };

    shader_cursor.def("__setitem__", set_float);
    shader_cursor.def("__setattr__", set_float);
#endif
}
