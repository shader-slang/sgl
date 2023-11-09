#pragma once

#include "kali/core/macros.h"

#include <span>
#include <string_view>
#include <string>
#include <vector>

namespace kali::string {

/**
 * Convert a string to lower case.
 * @param str Input string.
 * @return Lower case string.
 */
[[nodiscard]] KALI_API std::string to_lower(std::string_view str);

/**
 * Convert a string to upper case.
 * @param str Input string.
 * @return Upper case string.
 */
[[nodiscard]] KALI_API std::string to_upper(std::string_view str);

/**
 * Convert a string to wide string.
 * @param str Input string.
 * @return Wide string.
 */
[[nodiscard]] inline std::wstring to_wstring(std::string_view str)
{
    return std::wstring(str.begin(), str.end());
}

// inline std::string to_string(std::wstring str)
// {
//     return std::string(str.begin(), str.end());
// }

/**
 * Check is a string has the specified prefix.
 * @param str Input string.
 * @param prefix Prefix to check for.
 * @param case_sensitive Whether comparison should be case-sensitive.
 * @return True if the input string has the specified prefix.
 */
[[nodiscard]] KALI_API bool has_prefix(std::string_view str, std::string_view prefix, bool case_sensitive = true);

/**
 * Check is a string has the specified suffix.
 * @param str Input string.
 * @param suffix Suffix to check for.
 * @param case_sensitive Whether comparison should be case-sensitive.
 * @return True if the input string has the specified suffix.
 */
[[nodiscard]] KALI_API bool has_suffix(std::string_view str, std::string_view suffix, bool case_sensitive = true);

/**
 * Split a string into a list of strings based on a list of delimiter characters.
 * @param str Input string.
 * @param delimiters Delimiter characters.
 * @return List of split strings excluding the delimiter characters.
 */
[[nodiscard]] KALI_API std::vector<std::string> split(std::string_view str, std::string_view delimiters);

/**
 * Join an list of strings separated by a specified seperator.
 * @param strings Input strings.
 * @param separator String placed between each string to be joined.
 * @return Joined string.
 */
[[nodiscard]] KALI_API std::string join(std::span<const std::string> strings, std::string_view separator);

/**
 * Replace every newline with the specified indentation.
 * @param str Input string.
 * @param indentation Indentation string.
 * @return The indented string.
 */
[[nodiscard]] KALI_API std::string indent(std::string_view str, std::string_view indentation = "    ");

/**
 * Convert a list of objects that have a to_string() method to a string. *
 * @tparam T Type of objects.
 * @param list List of objects.
 * @return The list of objects as a string.
 */
template<typename T>
inline std::string list_to_string(std::span<T> list, std::string_view indentation = "    ")
{
    if (list.empty())
        return "[]";
    std::string result = "[\n";
    for (const auto& item : list) {
        result += indentation;
        if constexpr (std::is_pointer_v<T>)
            result += string::indent(item->to_string());
        else
            result += string::indent(item.to_string());
        result += ",\n";
    }
    result += "]";
    return result;
}

/**
 * Remove leading whitespace.
 * @param str Input string.
 * @param whitespace Whitespace characters.
 * @return String with leading whitespace removed.
 */
[[nodiscard]] KALI_API std::string
remove_leading_whitespace(std::string_view str, std::string_view whitespace = " \n\r\t");

/**
 * Remove trailing whitespace.
 * @param str Input string.
 * @param whitespace Whitespace characters.
 * @return String with trailing whitespace removed.
 */
[[nodiscard]] KALI_API std::string
remove_trailing_whitespace(std::string_view str, std::string_view whitespace = " \n\r\t");

/**
 * Remove leading and trailing whitespace.
 * @param str Input string.
 * @param whitespace Whitespace characters.
 * @return String with leading and trailing whitespace removed.
 */
[[nodiscard]] KALI_API std::string
remove_leading_trailing_whitespace(std::string_view str, std::string_view whitespace = " \n\r\t");

/**
 * Convert a size in bytes to a human readable string:
 * - prints bytes (B) if size < 1000 bytes
 * - prints kilobytes (KB) if size < 1000 kilobytes
 * - prints megabytes (MB) if size < 1000 megabytes
 * - prints gigabytes (GB) if size < 1000 gigabytes
 * - otherwise prints terabytes (TB)
 * @param size Size in bytes.
 * @return Size as a human readable string.
 */
[[nodiscard]] KALI_API std::string format_byte_size(size_t size);

/**
 * Encode data into base 64 encoding.
 * @param data Input data.
 * @param len Length of input data.
 * @return Base 64 encoded string.
 */
[[nodiscard]] KALI_API std::string encode_base64(const void* data, size_t len);

/**
 * Encode data into base 64 encoding.
 * @param data Input data.
 * @return Base 64 encoded string.
 */
[[nodiscard]] inline std::string encode_base64(std::span<const uint8_t> data)
{
    return encode_base64(data.data(), data.size());
}

/**
 * Decode data from base 64 encoding.
 * @param str Base 64 encoded string.
 * @return Decoded data.
 */
[[nodiscard]] KALI_API std::vector<uint8_t> decode_base64(std::string_view str);

} // namespace kali::string
