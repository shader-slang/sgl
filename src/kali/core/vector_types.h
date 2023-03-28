#pragma once

#include "scalar_types.h"

#include <cstdlib>

namespace kali {

// ----------------------------------------------------------------------------
// Vector types
// ----------------------------------------------------------------------------

template<typename T, size_t N>
struct vector_storage { };

template<typename T>
struct vector_storage<T, 1> {
    T x;
    // clang-format off
    constexpr vector_storage(T x = {}) noexcept : x{x} {}
    template<typename U>
    constexpr vector_storage(U x) noexcept : x{T(x)} {}
    // clang-format on
};

template<typename T>
struct vector_storage<T, 2> {
    T x, y;
    // clang-format off
    explicit constexpr vector_storage(T s = {}) noexcept : x{s}, y{s} {}
    constexpr vector_storage(T x, T y) noexcept : x{x}, y{y} {}
    template<typename U>
    constexpr vector_storage(U s) noexcept : x{T(s)}, y{T(s)} {}
    template<typename U>
    constexpr vector_storage(U x, U y) noexcept : x{T(x)}, y{T(y)} {}
    // clang-format on
#include "swizzle_2.inl.h"
};

template<typename T>
struct vector_storage<T, 3> {
    T x, y, z;
    // clang-format off
    explicit constexpr vector_storage(T s = {}) noexcept : x{s}, y{s}, z{s} {}
    constexpr vector_storage(T x, T y, T z) noexcept : x{x}, y{y}, z{z} {}
    template<typename U>
    constexpr vector_storage(U s) noexcept : x{T(s)}, y{T(s)}, z{T(s)} {}
    template<typename U>
    constexpr vector_storage(U x, U y, U z) noexcept : x{T(x)}, y{T(y)}, z{T(z)} {}
    // clang-format on
#include "swizzle_3.inl.h"
};

template<typename T>
struct vector_storage<T, 4> {
    T x, y, z, w;
    // clang-format off
    explicit constexpr vector_storage(T s = {}) noexcept : x{s}, y{s}, z{s}, w{s} {}
    constexpr vector_storage(T x, T y, T z, T w) noexcept : x{x}, y{y}, z{z}, w{w} {}
    template<typename U>
    constexpr vector_storage(U s) noexcept : x{T(s)}, y{T(s)}, z{T(s)}, w{T(s)} {}
    template<typename U>
    constexpr vector_storage(U x, U y, U z, U w) noexcept : x{T(x)}, y{T(y)}, z{T(z)}, w{T(w)} {}
    // clang-format on
#include "swizzle_4.inl.h"
};

template<typename T, size_t N>
struct vector : public vector_storage<T, N> {
    static constexpr auto dimension = N;
    using value_type = T;
    using storage = vector_storage<T, N>;
    static_assert(
        std::disjunction_v<std::is_same<T, bool>, std::is_same<T, float>, std::is_same<T, int>, std::is_same<T, uint>>,
        "Invalid vector type"
    );
    static_assert(N == 1 || N == 2 || N == 3 || N == 4, "Invalid vector dimension");
    using storage::vector_storage;
    [[nodiscard]] constexpr T& operator[](size_t index) noexcept { return (&(this->x))[index]; }
    [[nodiscard]] constexpr const T& operator[](size_t index) const noexcept { return (&(this->x))[index]; }
};

/* <<<PYMACRO
for t in ["float", "uint", "int", "bool"]:
    for n in [1, 2, 3, 4]:
        print(f"using {t}{n} = vector<{t}, {n}>;")
>>> */
using float1 = vector<float, 1>;
using float2 = vector<float, 2>;
using float3 = vector<float, 3>;
using float4 = vector<float, 4>;
using uint1 = vector<uint, 1>;
using uint2 = vector<uint, 2>;
using uint3 = vector<uint, 3>;
using uint4 = vector<uint, 4>;
using int1 = vector<int, 1>;
using int2 = vector<int, 2>;
using int3 = vector<int, 3>;
using int4 = vector<int, 4>;
using bool1 = vector<bool, 1>;
using bool2 = vector<bool, 2>;
using bool3 = vector<bool, 3>;
using bool4 = vector<bool, 4>;
/* <<<PYMACROEND>>> */

} // namespace kali
