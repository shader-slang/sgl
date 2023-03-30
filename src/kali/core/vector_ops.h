#pragma once

#include "vector_types.h"
#include "traits.h"
#include "format.h"

#include <string>

namespace kali {

// ----------------------------------------------------------------------------
// Boolean reductions
// ----------------------------------------------------------------------------

// clang-format off
[[nodiscard]] constexpr bool any(const bool1 v) noexcept { return v.x; }
[[nodiscard]] constexpr bool any(const bool2 v) noexcept { return v.x || v.y; }
[[nodiscard]] constexpr bool any(const bool3 v) noexcept { return v.x || v.y || v.z; }
[[nodiscard]] constexpr bool any(const bool4 v) noexcept { return v.x || v.y || v.z || v.w; }

[[nodiscard]] constexpr bool all(const bool1 v) noexcept { return v.x; }
[[nodiscard]] constexpr bool all(const bool2 v) noexcept { return v.x && v.y; }
[[nodiscard]] constexpr bool all(const bool3 v) noexcept { return v.x && v.y && v.z; }
[[nodiscard]] constexpr bool all(const bool4 v) noexcept { return v.x && v.y && v.z && v.w; }

[[nodiscard]] constexpr bool none(const bool1 v) noexcept { return !any(v); }
[[nodiscard]] constexpr bool none(const bool2 v) noexcept { return !any(v); }
[[nodiscard]] constexpr bool none(const bool3 v) noexcept { return !any(v); }
[[nodiscard]] constexpr bool none(const bool4 v) noexcept { return !any(v); }
// clang-format on

// ----------------------------------------------------------------------------
// Unary operators
// ----------------------------------------------------------------------------

/// Unary plus operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator+(const vector<T, N> v) noexcept
{
    return v;
}

#if KALI_MSVC
#pragma warning(push)
#pragma warning(disable : 4146) // unary minus operator applied to unsigned type, result still unsigned
#endif
/// Unary minus operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator-(const vector<T, N> v) noexcept
{
    if constexpr (N == 1)
        return vector<T, N>{-v.x};
    else if constexpr (N == 2)
        return vector<T, N>{-v.x, -v.y};
    else if constexpr (N == 3)
        return vector<T, N>{-v.x, -v.y, -v.z};
    else
        return vector<T, N>{-v.x, -v.y, -v.z, -v.w};
}
#if KALI_MSVC
#pragma warning(pop)
#endif

/// Unary not operator
template<typename T, size_t N>
[[nodiscard]] constexpr auto operator!(const vector<T, N> v) noexcept
{
    if constexpr (N == 1)
        return bool1{!v.x};
    else if constexpr (N == 2)
        return bool2{!v.x, !v.y};
    else if constexpr (N == 3)
        return bool3{!v.x, !v.y, !v.z};
    else
        return bool4{!v.x, !v.y, !v.z, !v.w};
}

/// Unary not operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr auto operator~(const vector<T, N> v) noexcept
{
    if constexpr (N == 1)
        return vector<T, N>{~v.x};
    else if constexpr (N == 2)
        return vector<T, N>{~v.x, ~v.y};
    else if constexpr (N == 3)
        return vector<T, N>{~v.x, ~v.y, ~v.z};
    else
        return vector<T, N>{~v.x, ~v.y, ~v.z, ~v.w};
}

// ----------------------------------------------------------------------------
// Binary operators
// ----------------------------------------------------------------------------

/* <<<PYMACRO
def print_binary_operator(op, enable_if):
    print(f"""/// Binary {op} operator
template<typename T, size_t N, std::enable_if_t<{enable_if}, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator{op}(const vector<T, N>& lhs, const vector<T, N>& rhs)
{{
    if constexpr (N == 1)
        return vector<T, N>{{lhs.x {op} rhs.x}};
    else if constexpr (N == 2)
        return vector<T, N>{{lhs.x {op} rhs.x, lhs.y {op} rhs.y}};
    else if constexpr (N == 3)
        return vector<T, N>{{lhs.x {op} rhs.x, lhs.y {op} rhs.y, lhs.z {op} rhs.z}};
    else if constexpr (N == 4)
        return vector<T, N>{{lhs.x {op} rhs.x, lhs.y {op} rhs.y, lhs.z {op} rhs.z, lhs.w {op} rhs.w}};
}}

/// Binary {op} operator
template<typename T, size_t N, std::enable_if_t<{enable_if}, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator{op}(const vector<T, N>& lhs, T rhs)
{{
    return lhs {op} vector<T, N>(rhs);
}}

/// Binary {op} operator
template<typename T, size_t N, std::enable_if_t<{enable_if}, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator{op}(T lhs, const vector<T, N>& rhs)
{{
    return vector<T, N>(lhs) {op} rhs;
}}
""")

print_binary_operator("+", "std::negation_v<is_boolean<T>>")
print_binary_operator("-", "std::negation_v<is_boolean<T>>")
print_binary_operator("*", "std::negation_v<is_boolean<T>>")
print_binary_operator("/", "std::negation_v<is_boolean<T>>")
print_binary_operator("%", "std::is_integral_v<T>")
print_binary_operator("<<", "std::is_integral_v<T>")
print_binary_operator(">>", "std::is_integral_v<T>")
print_binary_operator("|", "std::negation_v<std::is_floating_point<T>>")
print_binary_operator("&", "std::negation_v<std::is_floating_point<T>>")
print_binary_operator("^", "std::negation_v<std::is_floating_point<T>>")
>>> */
/// Binary + operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator+(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x + rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x + rhs.x, lhs.y + rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w};
}

/// Binary + operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator+(const vector<T, N>& lhs, T rhs)
{
    return lhs + vector<T, N>(rhs);
}

/// Binary + operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator+(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) + rhs;
}

/// Binary - operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator-(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x - rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x - rhs.x, lhs.y - rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w};
}

/// Binary - operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator-(const vector<T, N>& lhs, T rhs)
{
    return lhs - vector<T, N>(rhs);
}

/// Binary - operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator-(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) - rhs;
}

/// Binary * operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator*(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x * rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x * rhs.x, lhs.y * rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w};
}

/// Binary * operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator*(const vector<T, N>& lhs, T rhs)
{
    return lhs * vector<T, N>(rhs);
}

/// Binary * operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator*(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) * rhs;
}

/// Binary / operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator/(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x / rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x / rhs.x, lhs.y / rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w};
}

/// Binary / operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator/(const vector<T, N>& lhs, T rhs)
{
    return lhs / vector<T, N>(rhs);
}

/// Binary / operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator/(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) / rhs;
}

/// Binary % operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator%(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x % rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x % rhs.x, lhs.y % rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x % rhs.x, lhs.y % rhs.y, lhs.z % rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x % rhs.x, lhs.y % rhs.y, lhs.z % rhs.z, lhs.w % rhs.w};
}

/// Binary % operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator%(const vector<T, N>& lhs, T rhs)
{
    return lhs % vector<T, N>(rhs);
}

/// Binary % operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator%(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) % rhs;
}

/// Binary << operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator<<(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x << rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x << rhs.x, lhs.y << rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x << rhs.x, lhs.y << rhs.y, lhs.z << rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x << rhs.x, lhs.y << rhs.y, lhs.z << rhs.z, lhs.w << rhs.w};
}

/// Binary << operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator<<(const vector<T, N>& lhs, T rhs)
{
    return lhs << vector<T, N>(rhs);
}

/// Binary << operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator<<(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) << rhs;
}

/// Binary >> operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator>>(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x >> rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x >> rhs.x, lhs.y >> rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x >> rhs.x, lhs.y >> rhs.y, lhs.z >> rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x >> rhs.x, lhs.y >> rhs.y, lhs.z >> rhs.z, lhs.w >> rhs.w};
}

/// Binary >> operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator>>(const vector<T, N>& lhs, T rhs)
{
    return lhs >> vector<T, N>(rhs);
}

/// Binary >> operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator>>(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) >> rhs;
}

/// Binary | operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator|(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x | rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x | rhs.x, lhs.y | rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x | rhs.x, lhs.y | rhs.y, lhs.z | rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x | rhs.x, lhs.y | rhs.y, lhs.z | rhs.z, lhs.w | rhs.w};
}

/// Binary | operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator|(const vector<T, N>& lhs, T rhs)
{
    return lhs | vector<T, N>(rhs);
}

/// Binary | operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator|(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) | rhs;
}

/// Binary & operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator&(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x & rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x & rhs.x, lhs.y & rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x & rhs.x, lhs.y & rhs.y, lhs.z & rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x & rhs.x, lhs.y & rhs.y, lhs.z & rhs.z, lhs.w & rhs.w};
}

/// Binary & operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator&(const vector<T, N>& lhs, T rhs)
{
    return lhs & vector<T, N>(rhs);
}

/// Binary & operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator&(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) & rhs;
}

/// Binary ^ operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator^(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return vector<T, N>{lhs.x ^ rhs.x};
    else if constexpr (N == 2)
        return vector<T, N>{lhs.x ^ rhs.x, lhs.y ^ rhs.y};
    else if constexpr (N == 3)
        return vector<T, N>{lhs.x ^ rhs.x, lhs.y ^ rhs.y, lhs.z ^ rhs.z};
    else if constexpr (N == 4)
        return vector<T, N>{lhs.x ^ rhs.x, lhs.y ^ rhs.y, lhs.z ^ rhs.z, lhs.w ^ rhs.w};
}

/// Binary ^ operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator^(const vector<T, N>& lhs, T rhs)
{
    return lhs ^ vector<T, N>(rhs);
}

/// Binary ^ operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator^(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) ^ rhs;
}

/* <<<PYMACROEND>>> */

// ----------------------------------------------------------------------------
// Binary logic operators
// ----------------------------------------------------------------------------

/* <<<PYMACRO
def print_binary_operator(op, enable_if):
    print(f"""/// Binary {op} operator
template<typename T, size_t N, std::enable_if_t<{enable_if}, bool> = false>
[[nodiscard]] constexpr auto operator{op}(const vector<T, N>& lhs, const vector<T, N>& rhs)
{{
    if constexpr (N == 1)
        return bool1{{lhs.x {op} rhs.x}};
    else if constexpr (N == 2)
        return bool2{{lhs.x {op} rhs.x, lhs.y {op} rhs.y}};
    else if constexpr (N == 3)
        return bool3{{lhs.x {op} rhs.x, lhs.y {op} rhs.y, lhs.z {op} rhs.z}};
    else if constexpr (N == 4)
        return bool4{{lhs.x {op} rhs.x, lhs.y {op} rhs.y, lhs.z {op} rhs.z, lhs.w {op} rhs.w}};
}}

/// Binary {op} operator
template<typename T, size_t N, std::enable_if_t<{enable_if}, bool> = false>
[[nodiscard]] constexpr auto operator{op}(const vector<T, N>& lhs, T rhs)
{{
    return lhs {op} vector<T, N>(rhs);
}}

/// Binary {op} operator
template<typename T, size_t N, std::enable_if_t<{enable_if}, bool> = false>
[[nodiscard]] constexpr auto operator{op}(T lhs, const vector<T, N>& rhs)
{{
    return vector<T, N>(lhs) {op} rhs;
}}
""")

print_binary_operator("||", "is_boolean_v<T>")
print_binary_operator("&&", "is_boolean_v<T>")
print_binary_operator("==", "true")
print_binary_operator("!=", "true")
print_binary_operator("<", "std::negation_v<is_boolean<T>>")
print_binary_operator(">", "std::negation_v<is_boolean<T>>")
print_binary_operator("<=", "std::negation_v<is_boolean<T>>")
print_binary_operator(">=", "std::negation_v<is_boolean<T>>")
>>> */
/// Binary || operator
template<typename T, size_t N, std::enable_if_t<is_boolean_v<T>, bool> = false>
[[nodiscard]] constexpr auto operator||(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return bool1{lhs.x || rhs.x};
    else if constexpr (N == 2)
        return bool2{lhs.x || rhs.x, lhs.y || rhs.y};
    else if constexpr (N == 3)
        return bool3{lhs.x || rhs.x, lhs.y || rhs.y, lhs.z || rhs.z};
    else if constexpr (N == 4)
        return bool4{lhs.x || rhs.x, lhs.y || rhs.y, lhs.z || rhs.z, lhs.w || rhs.w};
}

/// Binary || operator
template<typename T, size_t N, std::enable_if_t<is_boolean_v<T>, bool> = false>
[[nodiscard]] constexpr auto operator||(const vector<T, N>& lhs, T rhs)
{
    return lhs || vector<T, N>(rhs);
}

/// Binary || operator
template<typename T, size_t N, std::enable_if_t<is_boolean_v<T>, bool> = false>
[[nodiscard]] constexpr auto operator||(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) || rhs;
}

/// Binary && operator
template<typename T, size_t N, std::enable_if_t<is_boolean_v<T>, bool> = false>
[[nodiscard]] constexpr auto operator&&(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return bool1{lhs.x && rhs.x};
    else if constexpr (N == 2)
        return bool2{lhs.x && rhs.x, lhs.y && rhs.y};
    else if constexpr (N == 3)
        return bool3{lhs.x && rhs.x, lhs.y && rhs.y, lhs.z && rhs.z};
    else if constexpr (N == 4)
        return bool4{lhs.x && rhs.x, lhs.y && rhs.y, lhs.z && rhs.z, lhs.w && rhs.w};
}

/// Binary && operator
template<typename T, size_t N, std::enable_if_t<is_boolean_v<T>, bool> = false>
[[nodiscard]] constexpr auto operator&&(const vector<T, N>& lhs, T rhs)
{
    return lhs && vector<T, N>(rhs);
}

/// Binary && operator
template<typename T, size_t N, std::enable_if_t<is_boolean_v<T>, bool> = false>
[[nodiscard]] constexpr auto operator&&(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) && rhs;
}

/// Binary == operator
template<typename T, size_t N, std::enable_if_t<true, bool> = false>
[[nodiscard]] constexpr auto operator==(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return bool1{lhs.x == rhs.x};
    else if constexpr (N == 2)
        return bool2{lhs.x == rhs.x, lhs.y == rhs.y};
    else if constexpr (N == 3)
        return bool3{lhs.x == rhs.x, lhs.y == rhs.y, lhs.z == rhs.z};
    else if constexpr (N == 4)
        return bool4{lhs.x == rhs.x, lhs.y == rhs.y, lhs.z == rhs.z, lhs.w == rhs.w};
}

/// Binary == operator
template<typename T, size_t N, std::enable_if_t<true, bool> = false>
[[nodiscard]] constexpr auto operator==(const vector<T, N>& lhs, T rhs)
{
    return lhs == vector<T, N>(rhs);
}

/// Binary == operator
template<typename T, size_t N, std::enable_if_t<true, bool> = false>
[[nodiscard]] constexpr auto operator==(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) == rhs;
}

/// Binary != operator
template<typename T, size_t N, std::enable_if_t<true, bool> = false>
[[nodiscard]] constexpr auto operator!=(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return bool1{lhs.x != rhs.x};
    else if constexpr (N == 2)
        return bool2{lhs.x != rhs.x, lhs.y != rhs.y};
    else if constexpr (N == 3)
        return bool3{lhs.x != rhs.x, lhs.y != rhs.y, lhs.z != rhs.z};
    else if constexpr (N == 4)
        return bool4{lhs.x != rhs.x, lhs.y != rhs.y, lhs.z != rhs.z, lhs.w != rhs.w};
}

/// Binary != operator
template<typename T, size_t N, std::enable_if_t<true, bool> = false>
[[nodiscard]] constexpr auto operator!=(const vector<T, N>& lhs, T rhs)
{
    return lhs != vector<T, N>(rhs);
}

/// Binary != operator
template<typename T, size_t N, std::enable_if_t<true, bool> = false>
[[nodiscard]] constexpr auto operator!=(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) != rhs;
}

/// Binary < operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator<(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return bool1{lhs.x < rhs.x};
    else if constexpr (N == 2)
        return bool2{lhs.x < rhs.x, lhs.y < rhs.y};
    else if constexpr (N == 3)
        return bool3{lhs.x < rhs.x, lhs.y < rhs.y, lhs.z < rhs.z};
    else if constexpr (N == 4)
        return bool4{lhs.x < rhs.x, lhs.y < rhs.y, lhs.z < rhs.z, lhs.w < rhs.w};
}

/// Binary < operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator<(const vector<T, N>& lhs, T rhs)
{
    return lhs < vector<T, N>(rhs);
}

/// Binary < operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator<(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) < rhs;
}

/// Binary > operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator>(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return bool1{lhs.x > rhs.x};
    else if constexpr (N == 2)
        return bool2{lhs.x > rhs.x, lhs.y > rhs.y};
    else if constexpr (N == 3)
        return bool3{lhs.x > rhs.x, lhs.y > rhs.y, lhs.z > rhs.z};
    else if constexpr (N == 4)
        return bool4{lhs.x > rhs.x, lhs.y > rhs.y, lhs.z > rhs.z, lhs.w > rhs.w};
}

/// Binary > operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator>(const vector<T, N>& lhs, T rhs)
{
    return lhs > vector<T, N>(rhs);
}

/// Binary > operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator>(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) > rhs;
}

/// Binary <= operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator<=(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return bool1{lhs.x <= rhs.x};
    else if constexpr (N == 2)
        return bool2{lhs.x <= rhs.x, lhs.y <= rhs.y};
    else if constexpr (N == 3)
        return bool3{lhs.x <= rhs.x, lhs.y <= rhs.y, lhs.z <= rhs.z};
    else if constexpr (N == 4)
        return bool4{lhs.x <= rhs.x, lhs.y <= rhs.y, lhs.z <= rhs.z, lhs.w <= rhs.w};
}

/// Binary <= operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator<=(const vector<T, N>& lhs, T rhs)
{
    return lhs <= vector<T, N>(rhs);
}

/// Binary <= operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator<=(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) <= rhs;
}

/// Binary >= operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator>=(const vector<T, N>& lhs, const vector<T, N>& rhs)
{
    if constexpr (N == 1)
        return bool1{lhs.x >= rhs.x};
    else if constexpr (N == 2)
        return bool2{lhs.x >= rhs.x, lhs.y >= rhs.y};
    else if constexpr (N == 3)
        return bool3{lhs.x >= rhs.x, lhs.y >= rhs.y, lhs.z >= rhs.z};
    else if constexpr (N == 4)
        return bool4{lhs.x >= rhs.x, lhs.y >= rhs.y, lhs.z >= rhs.z, lhs.w >= rhs.w};
}

/// Binary >= operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator>=(const vector<T, N>& lhs, T rhs)
{
    return lhs >= vector<T, N>(rhs);
}

/// Binary >= operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr auto operator>=(T lhs, const vector<T, N>& rhs)
{
    return vector<T, N>(lhs) >= rhs;
}

/* <<<PYMACROEND>>> */

// ----------------------------------------------------------------------------
// Assignment operator
// ----------------------------------------------------------------------------

/* <<<PYMACRO
def print_assignment_operator(op, enable_if):
    print(f"""/// {op} assignment operator
template<typename T, size_t N, std::enable_if_t<{enable_if}, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator{op}(vector<T, N>& lhs, const vector<T, N>& rhs)
{{
    lhs.x {op} rhs.x;
    if constexpr (N >= 2)
        lhs.y {op} rhs.y;
    if constexpr (N >= 3)
        lhs.z {op} rhs.z;
    if constexpr (N >= 4)
        lhs.w {op} rhs.w;
    return lhs;
}}

/// {op} assignment operator
template<typename T, size_t N, std::enable_if_t<{enable_if}, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator{op}(vector<T, N>& lhs, T rhs)
{{
    return (lhs {op} vector<T, N>(rhs));
}}
""")

print_assignment_operator("+=", "std::negation_v<is_boolean<T>>")
print_assignment_operator("-=", "std::negation_v<is_boolean<T>>")
print_assignment_operator("*=", "std::negation_v<is_boolean<T>>")
print_assignment_operator("/=", "std::negation_v<is_boolean<T>>")
print_assignment_operator("%=", "std::is_integral_v<T>")
print_assignment_operator("<<=", "std::is_integral_v<T>")
print_assignment_operator(">>=", "std::is_integral_v<T>")
print_assignment_operator("|=", "std::negation_v<std::is_floating_point<T>>")
print_assignment_operator("&=", "std::negation_v<std::is_floating_point<T>>")
print_assignment_operator("^=", "std::negation_v<std::is_floating_point<T>>")
>>> */
/// += assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator+=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x += rhs.x;
    if constexpr (N >= 2)
        lhs.y += rhs.y;
    if constexpr (N >= 3)
        lhs.z += rhs.z;
    if constexpr (N >= 4)
        lhs.w += rhs.w;
    return lhs;
}

/// += assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator+=(vector<T, N>& lhs, T rhs)
{
    return (lhs += vector<T, N>(rhs));
}

/// -= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator-=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x -= rhs.x;
    if constexpr (N >= 2)
        lhs.y -= rhs.y;
    if constexpr (N >= 3)
        lhs.z -= rhs.z;
    if constexpr (N >= 4)
        lhs.w -= rhs.w;
    return lhs;
}

/// -= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator-=(vector<T, N>& lhs, T rhs)
{
    return (lhs -= vector<T, N>(rhs));
}

/// *= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator*=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x *= rhs.x;
    if constexpr (N >= 2)
        lhs.y *= rhs.y;
    if constexpr (N >= 3)
        lhs.z *= rhs.z;
    if constexpr (N >= 4)
        lhs.w *= rhs.w;
    return lhs;
}

/// *= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator*=(vector<T, N>& lhs, T rhs)
{
    return (lhs *= vector<T, N>(rhs));
}

/// /= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator/=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x /= rhs.x;
    if constexpr (N >= 2)
        lhs.y /= rhs.y;
    if constexpr (N >= 3)
        lhs.z /= rhs.z;
    if constexpr (N >= 4)
        lhs.w /= rhs.w;
    return lhs;
}

/// /= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<is_boolean<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator/=(vector<T, N>& lhs, T rhs)
{
    return (lhs /= vector<T, N>(rhs));
}

/// %= assignment operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator%=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x %= rhs.x;
    if constexpr (N >= 2)
        lhs.y %= rhs.y;
    if constexpr (N >= 3)
        lhs.z %= rhs.z;
    if constexpr (N >= 4)
        lhs.w %= rhs.w;
    return lhs;
}

/// %= assignment operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator%=(vector<T, N>& lhs, T rhs)
{
    return (lhs %= vector<T, N>(rhs));
}

/// <<= assignment operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator<<=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x <<= rhs.x;
    if constexpr (N >= 2)
        lhs.y <<= rhs.y;
    if constexpr (N >= 3)
        lhs.z <<= rhs.z;
    if constexpr (N >= 4)
        lhs.w <<= rhs.w;
    return lhs;
}

/// <<= assignment operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator<<=(vector<T, N>& lhs, T rhs)
{
    return (lhs <<= vector<T, N>(rhs));
}

/// >>= assignment operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator>>=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x >>= rhs.x;
    if constexpr (N >= 2)
        lhs.y >>= rhs.y;
    if constexpr (N >= 3)
        lhs.z >>= rhs.z;
    if constexpr (N >= 4)
        lhs.w >>= rhs.w;
    return lhs;
}

/// >>= assignment operator
template<typename T, size_t N, std::enable_if_t<std::is_integral_v<T>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator>>=(vector<T, N>& lhs, T rhs)
{
    return (lhs >>= vector<T, N>(rhs));
}

/// |= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator|=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x |= rhs.x;
    if constexpr (N >= 2)
        lhs.y |= rhs.y;
    if constexpr (N >= 3)
        lhs.z |= rhs.z;
    if constexpr (N >= 4)
        lhs.w |= rhs.w;
    return lhs;
}

/// |= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator|=(vector<T, N>& lhs, T rhs)
{
    return (lhs |= vector<T, N>(rhs));
}

/// &= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator&=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x &= rhs.x;
    if constexpr (N >= 2)
        lhs.y &= rhs.y;
    if constexpr (N >= 3)
        lhs.z &= rhs.z;
    if constexpr (N >= 4)
        lhs.w &= rhs.w;
    return lhs;
}

/// &= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator&=(vector<T, N>& lhs, T rhs)
{
    return (lhs &= vector<T, N>(rhs));
}

/// ^= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator^=(vector<T, N>& lhs, const vector<T, N>& rhs)
{
    lhs.x ^= rhs.x;
    if constexpr (N >= 2)
        lhs.y ^= rhs.y;
    if constexpr (N >= 3)
        lhs.z ^= rhs.z;
    if constexpr (N >= 4)
        lhs.w ^= rhs.w;
    return lhs;
}

/// ^= assignment operator
template<typename T, size_t N, std::enable_if_t<std::negation_v<std::is_floating_point<T>>, bool> = false>
[[nodiscard]] constexpr vector<T, N> operator^=(vector<T, N>& lhs, T rhs)
{
    return (lhs ^= vector<T, N>(rhs));
}

/* <<<PYMACROEND>>> */

/// Convert vector to string.
template<typename T, size_t N>
std::string to_string(const vector<T, N>& v)
{
    return ::fmt::format("{}", v);
}

} // namespace kali

/// Vector string formatter.
template<typename T, size_t N>
struct ::fmt::formatter<::kali::vector<T, N>> : formatter<T> {
    template<typename FormatContext>
    auto format(const ::kali::vector<T, N>& v, FormatContext& ctx) const
    {
        auto out = ctx.out();
        out = ::fmt::format_to(out, "{}{}", kali::ScalarTraits<T>::name, N);
        for (int i = 0; i < N; ++i) {
            out = ::fmt::format_to(out, "{}", (i == 0) ? "(" : ", ");
            out = formatter<T>::format(v[i], ctx);
        }
        out = ::fmt::format_to(out, ")");
        return out;
    }
};
