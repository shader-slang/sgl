// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/error.h"
#include "sgl/core/format.h"

#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <concepts>

namespace sgl {

// Helper using ADL to find EnumInfo in other namespaces.
template<typename T>
using EnumInfo = decltype(find_enum_info_adl(std::declval<T>()));

template<typename T>
concept has_enum_info = requires(T v) {
    {
        EnumInfo<T>::name()
    } -> std::same_as<const std::string&>;
    {
        EnumInfo<T>::items()
    } -> std::same_as<std::span<std::pair<T, std::string>>>;
};

/**
 * Convert an enum value to a string.
 * Throws if the enum value is not found in the registered enum information.
 */
template<has_enum_info T>
inline const std::string& enum_to_string(T value)
{
    const auto& items = EnumInfo<T>::items();
    auto it = std::find_if(items.begin(), items.end(), [value](const auto& item) { return item.first == value; });
    if (it == items.end())
        SGL_THROW("Invalid enum value {}", int(value));
    return it->second;
}

/**
 * Convert a string to an enum value.
 * Throws if the string is not found in the registered enum information.
 */
template<has_enum_info T>
inline T string_to_enum(std::string_view name)
{
    const auto& items = EnumInfo<T>::items();
    auto it = std::find_if(items.begin(), items.end(), [name](const auto& item) { return item.second == name; });
    if (it == items.end())
        SGL_THROW("Invalid enum name \"{}\"", name);
    return it->first;
}

/**
 * Check if an enum has a value with the given name.
 */
template<has_enum_info T>
inline bool enum_has_value(std::string_view name)
{
    const auto& items = EnumInfo<T>::items();
    auto it = std::find_if(items.begin(), items.end(), [name](const auto& item) { return item.second == name; });
    return it != items.end();
}

/**
 * Convert an flags enum value to a list of strings.
 * Throws if any of the flags are not found in the registered enum information.
 */
template<has_enum_info T>
inline std::vector<std::string> flags_to_string_list(T flags)
{
    std::vector<std::string> list;
    const auto& items = EnumInfo<T>::items();
    for (const auto& item : items) {
        if (is_set(flags, item.first)) {
            list.push_back(item.second);
            flip_bit(flags, item.first);
        }
    }
    if (flags != T(0))
        SGL_THROW("Invalid enum flags value {}", int(flags));
    return list;
}

/**
 * Convert a list of strings to a flags enum value.
 * Throws if any of the strings are not found in the registered enum information.
 */
template<has_enum_info T>
inline T string_list_to_flags(const std::vector<std::string>& list)
{
    T flags = T(0);
    for (const auto& name : list)
        flags |= string_to_enum<T>(name);
    return flags;
}

namespace detail {

    /// Format an enum value.
    /// First, we check for a single value and return "name::value" if it succeeds.
    /// Otherwise, we check for flags and return "name::(value1 | value2 | ...)".
    /// Any bits that are not found in the enum information are formatted as hex.
    template<has_enum_info T>
    inline std::string format_enum(T value)
    {
        const auto& items = EnumInfo<T>::items();
        // Check for single value.
        for (const auto& item : items)
            if (item.first == value)
                return item.second;
        // Check for flags.
        std::string str = "(";
        uint64_t bits = uint64_t(value);
        bool first = true;
        for (const auto& item : items) {
            if (bits & uint64_t(item.first)) {
                if (!first)
                    str += " | ";
                str += item.second;
                bits &= ~uint64_t(item.first);
                first = false;
            }
        }
        if (bits != 0) {
            if (!first)
                str += " | ";
            str += fmt::format("0x{:x}", bits);
        }
        str += ")";
        return str;
    }

} // namespace detail
} // namespace sgl

/**
 * Define enum information. This is expected to be used as follows:
 *
 * enum class Foo { A, B, C };
 * SGL_ENUM_INFO(Foo, {
 *     { Foo::A, "A" },
 *     { Foo::B, "B" },
 *     { Foo::C, "C" },
 * })
 */
#define SGL_ENUM_INFO(T, ...)                                                                                          \
    struct T##_info {                                                                                                  \
        static const std::string& name()                                                                               \
        {                                                                                                              \
            static const std::string name = #T;                                                                        \
            return name;                                                                                               \
        }                                                                                                              \
        static std::span<std::pair<T, std::string>> items()                                                            \
        {                                                                                                              \
            static std::pair<T, std::string> items[] = __VA_ARGS__;                                                    \
            return {std::begin(items), std::end(items)};                                                               \
        }                                                                                                              \
    };

/**
 * Register enum information to be used with helper functions.
 * This needs to be placed outside of any structs but within the
 * namespace of the enum:
 *
 * namespace ns
 * {
 * struct Bar
 * {
 *     enum class Foo { A, B, C };
 *     SGL_ENUM_INFO(Foo, ...)
 * };
 *
 * SGL_ENUM_REGISTER(Bar::Foo)
 * } // namespace ns
 *
 * Registered enums can be converted to/from strings using:
 * - enum_to_string<Enum>(Enum value)
 * - string_to_enum<Enum>(std::string_view name)
 */
#define SGL_ENUM_REGISTER(T)                                                                                           \
    constexpr T##_info find_enum_info_adl [[maybe_unused]] (T) noexcept                                                \
    {                                                                                                                  \
        return T##_info{};                                                                                             \
    }

/// Enum formatter.
template<sgl::has_enum_info T>
struct fmt::formatter<T> : formatter<std::string> {
    template<typename FormatContext>
    auto format(const T& e, FormatContext& ctx) const
    {
        return formatter<std::string>::format(sgl::detail::format_enum(e), ctx);
    }
};
