#pragma once

#include "error.h"
#include "format.h"

#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <algorithm>
#include <vector>

namespace kali {

// Helper using ADL to find EnumInfo in other namespaces.
template<typename T>
using EnumInfo = decltype(find_enum_info_adl(std::declval<T>()));

template<typename, typename = void>
struct has_enum_info : std::false_type { };

template<typename T>
struct has_enum_info<T, std::void_t<decltype(EnumInfo<T>::items)>> : std::true_type { };

template<typename T>
inline constexpr bool has_enum_info_v = has_enum_info<T>::value;

/**
 * Convert an enum value to a string.
 * Throws if the enum value is not found in the registered enum information.
 */
template<typename T, std::enable_if_t<has_enum_info_v<T>, bool> = true>
inline const std::string& enum_to_string(T value)
{
    const auto& items = EnumInfo<T>::items();
    auto it = std::find_if(items.begin(), items.end(), [value](const auto& item) { return item.first == value; });
    if (it == items.end())
        KALI_THROW(RuntimeError("Invalid enum value {}", int(value)));
    return it->second;
}

/**
 * Convert a string to an enum value.
 * Throws if the string is not found in the registered enum information.
 */
template<typename T, std::enable_if_t<has_enum_info_v<T>, bool> = true>
inline T string_to_enum(std::string_view name)
{
    const auto& items = EnumInfo<T>::items();
    auto it = std::find_if(items.begin(), items.end(), [name](const auto& item) { return item.second == name; });
    if (it == items.end())
        KALI_THROW(RuntimeError("Invalid enum name '{}'", name));
    return it->first;
}

/**
 * Check if an enum has a value with the given name.
 */
template<typename T, std::enable_if_t<has_enum_info_v<T>, bool> = true>
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
template<typename T, std::enable_if_t<has_enum_info_v<T>, bool> = true>
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
        KALI_THROW(RuntimeError("Invalid enum flags value {}", int(flags)));
    return list;
}

/**
 * Convert a list of strings to a flags enum value.
 * Throws if any of the strings are not found in the registered enum information.
 */
template<typename T, std::enable_if_t<has_enum_info_v<T>, bool> = true>
inline T string_list_to_flags(const std::vector<std::string>& list)
{
    T flags = T(0);
    for (const auto& name : list)
        flags |= string_to_enum<T>(name);
    return flags;
}

} // namespace kali

/**
 * Define enum information. This is expected to be used as follows:
 *
 * enum class Foo { A, B, C };
 * KALI_ENUM_INFO(Foo, {
 *     { Foo::A, "A" },
 *     { Foo::B, "B" },
 *     { Foo::C, "C" },
 * })
 */
#define KALI_ENUM_INFO(T, ...)                                                                                         \
    struct T##_info {                                                                                                  \
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
 *     KALI_ENUM_INFO(Foo, ...)
 * };
 *
 * KALI_ENUM_REGISTER(Bar::Foo)
 * } // namespace ns
 *
 * Registered enums can be converted to/from strings using:
 * - enum_to_string<Enum>(Enum value)
 * - string_to_enum<Enum>(std::string_view name)
 */
#define KALI_ENUM_REGISTER(T)                                                                                          \
    constexpr T##_info find_enum_info_adl [[maybe_unused]] (T) noexcept                                                \
    {                                                                                                                  \
        return T##_info{};                                                                                             \
    }

/// Enum formatter.
template<typename T>
struct fmt::formatter<T, std::enable_if_t<kali::has_enum_info_v<T>, char>> : formatter<std::string> {
    template<typename FormatContext>
    auto format(const T& e, FormatContext& ctx)
    {
        return formatter<std::string>::format(kali::enum_to_string(e), ctx);
    }
};
