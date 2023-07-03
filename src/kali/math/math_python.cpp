#include "vector.h"
#include "matrix.h"
#include "quaternion.h"
#include "core/traits.h"

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace kali {

template<typename T, bool with_operators = true>
void bind_vector_type(nb::module_& m, const char* name)
{
    auto constexpr dimension = T::dimension;
    using value_type = typename T::value_type;

    static_assert(dimension >= 1 && dimension <= 4, "Invalid dimension");

    nb::class_<T> vec(m, name);

    // Field access

    vec.def_rw("x", &T::x);
    if constexpr (dimension >= 2)
        vec.def_rw("y", &T::y);
    if constexpr (dimension >= 3)
        vec.def_rw("z", &T::z);
    if constexpr (dimension >= 4)
        vec.def_rw("w", &T::w);

    // Constructors

    auto init_empty = [](T* v) { new (v) T(value_type(0)); };
    vec.def("__init__", init_empty);

    auto init_scalar = [](T* v, value_type c) { new (v) T(c); };
    vec.def("__init__", init_scalar, "c"_a);

    if constexpr (dimension == 2) {
        auto init_xy = [](T* v, value_type x, value_type y) { new (v) T(x, y); };
        vec.def("__init__", init_xy, "x"_a, "y"_a);
    } else if constexpr (dimension == 3) {
        auto init_xyz = [](T* v, value_type x, value_type y, value_type z) { new (v) T(x, y, z); };
        vec.def("__init__", init_xyz, "x"_a, "y"_a, "z"_a);
    } else if constexpr (dimension == 4) {
        auto init_xyzw = [](T* v, value_type x, value_type y, value_type z, value_type w) { new (v) T(x, y, z, w); };
        vec.def("__init__", init_xyzw, "x"_a, "y"_a, "z"_a, "w"_a);
    }

    auto to_string = [](const T& v) { return kali::math::to_string(v); };
    vec.def("__repr__", to_string);
    vec.def("__str__", to_string);

    // Operators

    if constexpr (with_operators) {
        vec.def(nb::self == nb::self);
        vec.def(nb::self != nb::self);

        if constexpr (!is_boolean_v<value_type>) {
            vec.def(+nb::self);
            vec.def(-nb::self);

            vec.def(nb::self + nb::self);
            vec.def(nb::self + value_type());
            vec.def(value_type() + nb::self);
            vec.def(nb::self - nb::self);
            vec.def(nb::self - value_type());
            vec.def(value_type() - nb::self);
            vec.def(nb::self * nb::self);
            vec.def(nb::self * value_type());
            vec.def(value_type() * nb::self);
            vec.def(nb::self / nb::self);
            vec.def(nb::self / value_type());
            vec.def(value_type() / nb::self);

            vec.def(nb::self += nb::self);
            vec.def(nb::self += value_type());
            vec.def(nb::self -= nb::self);
            vec.def(nb::self -= value_type());
            vec.def(nb::self *= nb::self);
            vec.def(nb::self *= value_type());
            vec.def(nb::self /= nb::self);
            vec.def(nb::self /= value_type());
        }

        if constexpr (std::is_integral_v<value_type> && !is_boolean_v<value_type>) {
            vec.def(nb::self % nb::self);
            vec.def(nb::self % value_type());
            vec.def(value_type() % nb::self);
            vec.def(nb::self << nb::self);
            vec.def(nb::self << value_type());
            vec.def(value_type() << nb::self);
            vec.def(nb::self >> nb::self);
            vec.def(nb::self >> value_type());
            vec.def(value_type() >> nb::self);

            vec.def(nb::self | nb::self);
            vec.def(nb::self | value_type());
            vec.def(value_type() | nb::self);
            vec.def(nb::self & nb::self);
            vec.def(nb::self & value_type());
            vec.def(value_type() & nb::self);
            vec.def(nb::self ^ nb::self);
            vec.def(nb::self ^ value_type());
            vec.def(value_type() ^ nb::self);
        }
    }
}

void register_math(nb::module_& m)
{
    bind_vector_type<float1>(m, "float1");
    bind_vector_type<float2>(m, "float2");
    bind_vector_type<float3>(m, "float3");
    bind_vector_type<float4>(m, "float4");
    bind_vector_type<uint1>(m, "uint1");
    bind_vector_type<uint2>(m, "uint2");
    bind_vector_type<uint3>(m, "uint3");
    bind_vector_type<uint4>(m, "uint4");
    bind_vector_type<int1>(m, "int1");
    bind_vector_type<int2>(m, "int2");
    bind_vector_type<int3>(m, "int3");
    bind_vector_type<int4>(m, "int4");
    bind_vector_type<bool1>(m, "bool1");
    bind_vector_type<bool2>(m, "bool2");
    bind_vector_type<bool3>(m, "bool3");
    bind_vector_type<bool4>(m, "bool4");
}

} // namespace kali
