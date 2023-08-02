#pragma once

#include <algorithm>
#include <type_traits>

namespace kali {

/// Clamp x to [lo..hi] range.
template<typename T>
constexpr T clamp(const T& x, const T& lo, const T& hi)
{
    return std::min(std::max(x, lo), hi);
}

/// Linearly interpolate between a and b.
template<typename T, typename U>
inline typename std::enable_if<std::is_floating_point_v<U>, T>::type lerp(const T& a, const T& b, const U& t)
{
    return (U(1) - t) * a + t * b;
}

/// Returns true if integer value is a power of two.
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, bool>::type is_power_of_two(T a)
{
    return (a & (a - (T)1)) == 0;
}

/// Divide a by b and round up to the next integer.
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type div_round_up(T a, T b)
{
    return (a + b - T(1)) / b;
}

/// Align an integer value to the given alignment.
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type align_to(T alignment, T value)
{
    return ((value + alignment - T(1)) / alignment) * alignment;
}

} // namespace kali
