// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <type_traits>

namespace kali {

template<typename T>
struct is_boolean : std::is_same<T, bool> { };

template<typename T>
inline constexpr bool is_boolean_v = is_boolean<T>::value;

} // namespace kali
