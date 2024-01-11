#include "nanobind.h"

#include "kali/math/scalar_types.h"
#include "kali/math/scalar_math.h"
#include "kali/core/traits.h"

#include <array>
#include <type_traits>

namespace kali::math {

inline void bind_float16(nb::module_& m)
{
    nb::class_<float16_t> float16(m, "float16_t");
    float16.def(nb::init<float>(), "value"_a);
    float16.def(nb::init_implicit<float>(), "value"_a);
}

inline void bind_scalar(nb::module_& m)
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

} // namespace kali::math

KALI_PY_EXPORT(math_scalar)
{
    nb::module_ math = m.attr("math");

    kali::math::bind_float16(math);
    kali::math::bind_scalar(math);

    m.attr("float16_t") = math.attr("float16_t");
}
