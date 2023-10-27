#include "nanobind.h"

#include "kali/math/vector.h"
#include "kali/core/traits.h"

#include <array>
#include <type_traits>

namespace kali::math {

template<typename T, bool BindIntrinsics = true>
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

    vec.def("__getitem__", [](const T& self, int i) { return self[i]; });
    vec.def("__setitem__", [](T& self, int i, value_type v) { self[i] = v; });

    // Constructors

    auto init_empty = [](T* self) { new (self) T(value_type(0)); };
    vec.def("__init__", init_empty);

    vec.def(nb::init<value_type>(), "scalar"_a);

    if constexpr (dimension == 2)
        vec.def(nb::init<value_type, value_type>(), "x"_a, "y"_a);
    else if constexpr (dimension == 3)
        vec.def(nb::init<value_type, value_type, value_type>(), "x"_a, "y"_a, "z"_a);
    else if constexpr (dimension == 4)
        vec.def(nb::init<value_type, value_type, value_type, value_type>(), "x"_a, "y"_a, "z"_a, "w"_a);

    vec.def(nb::init_implicit<std::array<value_type, dimension>>(), "a"_a);

    auto to_string_ = [](const T& self) { return to_string(self); };
    vec.def("__repr__", to_string_);
    vec.def("__str__", to_string_);

    // Operators

    if constexpr (is_arithmetic_v<value_type> && !is_bool_v<value_type>) {
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

    if constexpr (is_integral_v<value_type> && !is_bool_v<value_type>) {
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

        vec.def(nb::self %= nb::self);
        vec.def(nb::self %= value_type());

        vec.def(nb::self <<= nb::self);
        vec.def(nb::self <<= value_type());

        vec.def(nb::self >>= nb::self);
        vec.def(nb::self >>= value_type());

        vec.def(nb::self |= nb::self);
        vec.def(nb::self |= value_type());

        vec.def(nb::self &= nb::self);
        vec.def(nb::self &= value_type());

        vec.def(nb::self ^= nb::self);
        vec.def(nb::self ^= value_type());
    }

    if constexpr (is_bool_v<value_type>) {
        // vec.def(nb::self || nb::self);
        // vec.def(nb::self || value_type());
        // vec.def(value_type() || nb::self);
        // vec.def(nb::self && nb::self);
        // vec.def(nb::self && value_type());
        // vec.def(value_type() && nb::self);
    }

    vec.def(nb::self == nb::self);
    vec.def(nb::self == value_type());
    vec.def(value_type() == nb::self);
    vec.def(nb::self != nb::self);
    vec.def(nb::self != value_type());
    vec.def(value_type() != nb::self);

    if constexpr (is_arithmetic_v<value_type> && !is_bool_v<value_type>) {
        vec.def(nb::self < nb::self);
        vec.def(nb::self < value_type());
        vec.def(value_type() < nb::self);
        vec.def(nb::self > nb::self);
        vec.def(nb::self > value_type());
        vec.def(value_type() > nb::self);
        vec.def(nb::self <= nb::self);
        vec.def(nb::self <= value_type());
        vec.def(value_type() <= nb::self);
        vec.def(nb::self >= nb::self);
        vec.def(nb::self >= value_type());
        vec.def(value_type() >= nb::self);
    }

    // Intrinsics

    if constexpr (BindIntrinsics) {

        // Reductions

        if constexpr (is_bool_v<value_type>) {
            m.def("any", [](const T& v) { return any(v); });
            m.def("all", [](const T& v) { return all(v); });
            m.def("none", [](const T& v) { return none(v); });
        }

        // Basic functions

        if constexpr (is_arithmetic_v<value_type>) {
            m.def("min", [](const T& x, const T& y) { return min(x, y); });
            m.def("max", [](const T& x, const T& y) { return max(x, y); });
            m.def("clamp", [](const T& x, const T& min_, const T& max_) { return clamp(x, min_, max_); });
        }

        if constexpr (is_signed_v<value_type>) {
            m.def("abs", [](const T& x) { return abs(x); });
            m.def("sign", [](const T& x) { return sign(x); });
        }

        // Floating point checks

        if constexpr (is_floating_point_v<value_type>) {
            m.def("isfinite", [](const T& x) { return isfinite(x); });
            m.def("isinf", [](const T& x) { return isinf(x); });
            m.def("isnan", [](const T& x) { return isnan(x); });
        }

        // Rounding

        if constexpr (is_floating_point_v<value_type>) {
            m.def("floor", [](const T& x) { return floor(x); });
            m.def("ceil", [](const T& x) { return ceil(x); });
            m.def("trunc", [](const T& x) { return trunc(x); });
            m.def("round", [](const T& x) { return round(x); });
        }

        // Exponential

        if constexpr (is_floating_point_v<value_type>) {
            m.def("pow", [](const T& x, const T& y) { return pow(x, y); });
            m.def("sqrt", [](const T& x) { return sqrt(x); });
            m.def("rsqrt", [](const T& x) { return rsqrt(x); });
            m.def("exp", [](const T& x) { return exp(x); });
            m.def("exp2", [](const T& x) { return exp2(x); });
            m.def("log", [](const T& x) { return log(x); });
            m.def("log2", [](const T& x) { return log2(x); });
            m.def("log10", [](const T& x) { return log10(x); });
        }

        // Trigonometry

        if constexpr (is_floating_point_v<value_type>) {
            m.def("radians", [](const T& x) { return radians(x); });
            m.def("degrees", [](const T& x) { return degrees(x); });
            m.def("sin", [](const T& x) { return sin(x); });
            m.def("cos", [](const T& x) { return cos(x); });
            m.def("tan", [](const T& x) { return tan(x); });
            m.def("asin", [](const T& x) { return asin(x); });
            m.def("acos", [](const T& x) { return acos(x); });
            m.def("atan", [](const T& x) { return atan(x); });
            m.def("atan2", [](const T& y, const T& x) { return atan2(y, x); });
            m.def("sinh", [](const T& x) { return sinh(x); });
            m.def("cosh", [](const T& x) { return cosh(x); });
            m.def("tanh", [](const T& x) { return tanh(x); });
        }

        // Misc

        if constexpr (is_floating_point_v<value_type>) {
            m.def("fmod", [](const T& x, const T& y) { return fmod(x, y); });
            m.def("frac", [](const T& x) { return frac(x); });
            m.def("lerp", [](const T& x, const T& y, const T& s) { return lerp(x, y, s); });
            m.def("lerp", [](const T& x, const T& y, const value_type& s) { return lerp(x, y, s); });
            m.def("rcp", [](const T& x) { return rcp(x); });
            m.def("saturate", [](const T& x) { return saturate(x); });
            m.def("smoothstep", [](const T& min_, const T& max_, const T& x) { return smoothstep(min_, max_, x); });
            m.def("step", [](const T& x, const T& y) { return step(x, y); });
        }

        if constexpr (is_arithmetic_v<value_type> && !is_bool_v<value_type>) {
            m.def("dot", [](const T& x, const T& y) { return dot(x, y); });
            if constexpr (dimension == 3)
                m.def("cross", [](const T& x, const T& y) { return cross(x, y); });
        }

        if constexpr (is_floating_point_v<value_type>) {
            m.def("length", [](const T& x) { return length(x); });
            m.def("normalize", [](const T& x) { return normalize(x); });
            m.def("reflect", [](const T& i, const T& n) { return reflect(i, n); });
        }
    }
}

inline void bind_vector(nb::module_& m)
{
    bind_vector_type<float1>(m, "float1");
    bind_vector_type<float2>(m, "float2");
    bind_vector_type<float3>(m, "float3");
    bind_vector_type<float4>(m, "float4");
    bind_vector_type<float16_t1, false>(m, "float16_t1");
    bind_vector_type<float16_t2, false>(m, "float16_t2");
    bind_vector_type<float16_t3, false>(m, "float16_t3");
    bind_vector_type<float16_t4, false>(m, "float16_t4");
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

    m.def("f16tof32", [](const uint2& value) { return f16tof32(value); });
    m.def("f16tof32", [](const uint3& value) { return f16tof32(value); });
    m.def("f16tof32", [](const uint4& value) { return f16tof32(value); });

    m.def("f32tof16", [](const float2& value) { return f32tof16(value); });
    m.def("f32tof16", [](const float3& value) { return f32tof16(value); });
    m.def("f32tof16", [](const float4& value) { return f32tof16(value); });
}

} // namespace kali::math

KALI_PY_EXPORT(math_vector)
{
    nb::module_ math = m.attr("math");

    kali::math::bind_vector(math);

    m.attr("float1") = math.attr("float1");
    m.attr("float2") = math.attr("float2");
    m.attr("float3") = math.attr("float3");
    m.attr("float4") = math.attr("float4");
    m.attr("float16_t1") = math.attr("float16_t1");
    m.attr("float16_t2") = math.attr("float16_t2");
    m.attr("float16_t3") = math.attr("float16_t3");
    m.attr("float16_t4") = math.attr("float16_t4");
    m.attr("uint1") = math.attr("uint1");
    m.attr("uint2") = math.attr("uint2");
    m.attr("uint3") = math.attr("uint3");
    m.attr("uint4") = math.attr("uint4");
    m.attr("int1") = math.attr("int1");
    m.attr("int2") = math.attr("int2");
    m.attr("int3") = math.attr("int3");
    m.attr("int4") = math.attr("int4");
    m.attr("bool1") = math.attr("bool1");
    m.attr("bool2") = math.attr("bool2");
    m.attr("bool3") = math.attr("bool3");
    m.attr("bool4") = math.attr("bool4");
}
