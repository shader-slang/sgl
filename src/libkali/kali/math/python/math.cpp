#include "nanobind.h"

#include "kali/math/vector.h"
#include "kali/math/matrix.h"
#include "kali/math/quaternion.h"
#include "kali/core/traits.h"

#include <array>
#include <type_traits>

namespace kali::math {

inline void register_scalar_math(nb::module_& m)
{
    // Floating point checks

    m.def("isfinite", [](float x) { return isfinite(x); });
    m.def("isfinite", [](double x) { return isfinite(x); });
    m.def("isfinite", [](float16_t x) { return isfinite(x); });

    m.def("isinf", [](float x) { return isinf(x); });
    m.def("isinf", [](double x) { return isinf(x); });
    m.def("isinf", [](float16_t x) { return isinf(x); });

    m.def("isnan", [](float x) { return isnan(x); });
    m.def("isnan", [](double x) { return isnan(x); });
    m.def("isnan", [](float16_t x) { return isnan(x); });

    // Rounding

    m.def("floor", [](float x) { return floor(x); });
    m.def("floor", [](double x) { return floor(x); });
    // m.def("floor", [](float16_t x) { return floor(x); });

    m.def("ceil", [](float x) { return ceil(x); });
    m.def("ceil", [](double x) { return ceil(x); });
    // m.def("ceil", [](float16_t x) { return ceil(x); });

    m.def("trunc", [](float x) { return trunc(x); });
    m.def("trunc", [](double x) { return trunc(x); });
    // m.def("trunc", [](float16_t x) { return trunc(x); });

    m.def("round", [](float x) { return round(x); });
    m.def("round", [](double x) { return round(x); });
    // m.def("round", [](float16_t x) { return round(x); });

    // Exponential

    m.def("pow", [](float x, float y) { return pow(x, y); });
    m.def("pow", [](double x, double y) { return pow(x, y); });
    // m.def("pow", [](float16_t x, float16_t y) { return pow(x, y); });

    m.def("sqrt", [](float x) { return sqrt(x); });
    m.def("sqrt", [](double x) { return sqrt(x); });
    // m.def("sqrt", [](float16_t x) { return sqrt(x); });

    m.def("rsqrt", [](float x) { return rsqrt(x); });
    m.def("rsqrt", [](double x) { return rsqrt(x); });
    // m.def("rsqrt", [](float16_t x) { return rsqrt(x); });

    m.def("exp", [](float x) { return exp(x); });
    m.def("exp", [](double x) { return exp(x); });
    m.def("exp", [](float16_t x) { return exp(x); });

    m.def("exp2", [](float x) { return exp2(x); });
    m.def("exp2", [](double x) { return exp2(x); });
    m.def("exp2", [](float16_t x) { return exp2(x); });

    m.def("log", [](float x) { return log(x); });
    m.def("log", [](double x) { return log(x); });
    m.def("log", [](float16_t x) { return log(x); });

    m.def("log2", [](float x) { return log2(x); });
    m.def("log2", [](double x) { return log2(x); });
    // m.def("log2", [](float16_t x) { return log2(x); });

    m.def("log10", [](float x) { return log10(x); });
    m.def("log10", [](double x) { return log10(x); });
    // m.def("log10", [](float16_t x) { return log10(x); });

    // Trigonometry

    m.def("radians", [](float x) { return radians(x); });
    m.def("radians", [](double x) { return radians(x); });

    m.def("degrees", [](float x) { return degrees(x); });
    m.def("degrees", [](double x) { return degrees(x); });

    m.def("sin", [](float x) { return sin(x); });
    m.def("sin", [](double x) { return sin(x); });

    m.def("cos", [](float x) { return cos(x); });
    m.def("cos", [](double x) { return cos(x); });

    m.def("tan", [](float x) { return tan(x); });
    m.def("tan", [](double x) { return tan(x); });

    m.def("asin", [](float x) { return asin(x); });
    m.def("asin", [](double x) { return asin(x); });

    m.def("acos", [](float x) { return acos(x); });
    m.def("acos", [](double x) { return acos(x); });

    m.def("atan", [](float x) { return atan(x); });
    m.def("atan", [](double x) { return atan(x); });

    m.def("atan2", [](float y, float x) { return atan2(y, x); });
    m.def("atan2", [](double y, double x) { return atan2(y, x); });

    m.def("sinh", [](float x) { return sinh(x); });
    m.def("sinh", [](double x) { return sinh(x); });

    m.def("cosh", [](float x) { return cosh(x); });
    m.def("cosh", [](double x) { return cosh(x); });

    m.def("tanh", [](float x) { return tanh(x); });
    m.def("tanh", [](double x) { return tanh(x); });

    // Misc

    m.def("fmod", [](float x, float y) { return fmod(x, y); });
    m.def("fmod", [](double x, double y) { return fmod(x, y); });

    m.def("frac", [](float x) { return frac(x); });
    m.def("frac", [](double x) { return frac(x); });

    m.def("lerp", [](float x, float y, float s) { return lerp(x, y, s); });
    m.def("lerp", [](double x, double y, double s) { return lerp(x, y, s); });

    m.def("rcp", [](float x) { return rcp(x); });
    m.def("rcp", [](double x) { return rcp(x); });

    m.def("saturate", [](float x) { return saturate(x); });
    m.def("saturate", [](double x) { return saturate(x); });

    m.def("step", [](float x, float y) { return step(x, y); });
    m.def("step", [](double x, double y) { return step(x, y); });

    m.def("smoothstep", [](float min_, float max_, float x) { return smoothstep(min_, max_, x); });
    m.def("smoothstep", [](double min_, double max_, double x) { return smoothstep(min_, max_, x); });

    // Conversion

    m.def("f16tof32", [](uint v) { return f16tof32(v); });
    m.def("f32tof16", [](float v) { return f32tof16(v); });

    m.def("asfloat", [](uint32_t i) { return asfloat(i); });
    m.def("asfloat", [](int32_t i) { return asfloat(i); });
    m.def("asfloat16", [](uint16_t i) { return asfloat16(i); });

    m.def("asuint", [](float f) { return asuint(f); });
    m.def("asint", [](float f) { return asint(f); });
    m.def("asuint16", [](float16_t f) { return asuint16(f); });
}

template<typename T>
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

inline void register_vector_math(nb::module_& m)
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

    m.def("f16tof32", [](const uint2& value) { return f16tof32(value); });
    m.def("f16tof32", [](const uint3& value) { return f16tof32(value); });
    m.def("f16tof32", [](const uint4& value) { return f16tof32(value); });

    m.def("f32tof16", [](const float2& value) { return f32tof16(value); });
    m.def("f32tof16", [](const float3& value) { return f32tof16(value); });
    m.def("f32tof16", [](const float4& value) { return f32tof16(value); });
}

template<typename T>
void bind_matrix_type(nb::module_& m, const char* name)
{
    nb::class_<T> mat(m, name);

    auto constexpr rows = T::get_row_count();
    auto constexpr cols = T::get_col_count();
    using value_type = typename T::value_type;
    using row_type = typename T::row_type;
    using col_type = typename T::col_type;

    mat.def(nb::init<>());

    // Initialization from array (to allow implicit conversion from python lists).
    mat.def(nb::init_implicit<std::array<value_type, rows * cols>>());

    mat.def("__getitem__", [](const T& self, int i) { return self[i]; });
    mat.def("__setitem__", [](T& self, int i, const row_type& v) { self[i] = v; });

    mat.def("__getitem__", [](const T& self, std::array<int, 2> ij) { return self[ij[0]][ij[1]]; });
    mat.def("__setitem__", [](T& self, std::array<int, 2> ij, const value_type& v) { self[ij[0]][ij[1]] = v; });

    mat.def("get_row", nb::overload_cast<int>(&T::get_row, nb::const_), "row"_a);
    mat.def("set_row", &T::set_row, "row"_a, "value"_a);
    mat.def("get_col", &T::get_col, "col"_a);
    mat.def("set_col", &T::set_col, "col"_a, "value"_a);

    auto to_string_ = [](const T& self) { return to_string(self); };
    mat.def("__repr__", to_string_);
    mat.def("__str__", to_string_);

    mat.def_static("zeros", &T::zeros);
    mat.def_static("identity", &T::identity);

    mat.def(nb::self == nb::self);
    mat.def(nb::self != nb::self);

    m.def("transpose", [](const T& m) { return transpose(m); });

    if constexpr (rows == cols) {
        m.def("determinant", [](const T& m) { return determinant(m); });
        m.def("inverse", [](const T& m) { return inverse(m); });
    }
}

inline void register_matrix_math(nb::module_& m)
{
    bind_matrix_type<float2x2>(m, "float2x2");
    bind_matrix_type<float3x3>(m, "float3x3");
    bind_matrix_type<float1x4>(m, "float1x4");
    bind_matrix_type<float2x4>(m, "float2x4");
    bind_matrix_type<float3x4>(m, "float3x4");
    bind_matrix_type<float4x4>(m, "float4x4");

    m.def(
        "perspective",
        [](float fovy, float aspect, float z_near, float z_far) { return perspective(fovy, aspect, z_near, z_far); }
    );
    m.def(
        "ortho",
        [](float left, float right, float bottom, float top, float z_near, float z_far)
        { return ortho(left, right, bottom, top, z_near, z_far); }
    );
}

template<typename T>
void bind_quaternion_type(nb::module_& m, const char* name)
{
    nb::class_<T> quat(m, name);
}

inline void register_quaternion_math(nb::module_& m)
{
    bind_quaternion_type<quatf>(m, "quatf");

    // TODO add bindings
}

inline void register_kali_math(nb::module_& m)
{
    register_scalar_math(m);
    register_vector_math(m);
    register_matrix_math(m);
    register_quaternion_math(m);
}

} // namespace kali::math

KALI_PY_EXPORT(math)
{
    nb::module_ math = m.def_submodule("math", "Math module");

    kali::math::register_kali_math(math);

    m.attr("float1") = math.attr("float1");
    m.attr("float2") = math.attr("float2");
    m.attr("float3") = math.attr("float3");
    m.attr("float4") = math.attr("float4");
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

    m.attr("float2x2") = math.attr("float2x2");
    m.attr("float3x3") = math.attr("float3x3");
    m.attr("float1x4") = math.attr("float1x4");
    m.attr("float2x4") = math.attr("float2x4");
    m.attr("float3x4") = math.attr("float3x4");
    m.attr("float4x4") = math.attr("float4x4");

    m.attr("quatf") = math.attr("quatf");
}
