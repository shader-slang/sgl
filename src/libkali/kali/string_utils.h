#pragma once

#include "kali/macros.h"

#include <span>
#include <string_view>
#include <string>
#include <vector>

namespace kali {

[[nodiscard]] KALI_API std::string to_lower(std::string_view str);
[[nodiscard]] KALI_API std::string to_upper(std::string_view str);

[[nodiscard]] inline std::wstring to_wstring(std::string_view str)
{
    return std::wstring(str.begin(), str.end());
}

// inline std::string to_string(std::wstring str)
// {
//     return std::string(str.begin(), str.end());
// }

/**
 * Check is a string starts with another string
 * @param[in] str String to check in
 * @param[in] prefix Prefix to check for
 * @param[in] case_sensitive Whether comparison should be case-sensitive
 * @return Returns true if string starts with the specified prefix.
 */
[[nodiscard]] KALI_API bool has_prefix(std::string_view str, std::string_view prefix, bool case_sensitive = true);

/**
 * Check is a string ends with another string
 * @param[in] str String to check in
 * @param[in] suffix Suffix to check for
 * @param[in] case_sensitive Whether comparison should be case-sensitive
 * @return Returns true if string ends with the specified suffix
 */
[[nodiscard]] KALI_API bool has_suffix(std::string_view str, std::string_view suffix, bool case_sensitive = true);

/**
 * Split a string into a vector of strings based on d delimiter
 * @param[in] str String to split
 * @param[in] delim Delimiter to split strings by
 * @return Array of split strings excluding delimiters.
 */
[[nodiscard]] KALI_API std::vector<std::string> split_string(std::string_view str, std::string_view delim);

/**
 * Join an array of strings separated by another set string
 * @param[in] strings Array of strings to join.
 * @param[in] separator String placed between each string to be joined.
 * @return Joined string.
 */
[[nodiscard]] KALI_API std::string join_strings(std::span<const std::string> strings, std::string_view separator);

/**
 * Remove leading whitespace.
 * @param[in] str String to operate on.
 * @param[in] whitespace Whitespace characters.
 * @return String with leading whitespace removed.
 */
[[nodiscard]] KALI_API std::string
remove_leading_whitespace(std::string_view str, std::string_view whitespace = " \n\r\t");

/**
 * Remove trailing whitespace.
 * @param[in] str String to operate on.
 * @param[in] whitespace Whitespace characters.
 * @return String with trailing whitespace removed.
 */
[[nodiscard]] KALI_API std::string
remove_trailing_whitespace(std::string_view str, std::string_view whitespace = " \n\r\t");

/**
 * Remove leading and trailing whitespace.
 * @param[in] str String to operate on.
 * @param[in] whitespace Whitespace characters.
 * @return String with leading and trailing whitespace removed.
 */
[[nodiscard]] KALI_API std::string
remove_leading_trailing_whitespace(std::string_view str, std::string_view whitespace = " \n\r\t");

/**
 * Converts a size in bytes to a human readable string:
 * - prints bytes (B) if size < 1000 bytes
 * - prints kilobytes (KB) if size < 1000 kilobytes
 * - prints megabytes (MB) if size < 1000 megabytes
 * - prints gigabytes (GB) if size < 1000 gigabytes
 * - otherwise prints terabytes (TB)
 * @param[in] size Size in bytes
 * @return Returns a human readable string.
 */
[[nodiscard]] KALI_API std::string format_byte_size(size_t size);

/**
 * Encode data into base 64 encoding.
 */
[[nodiscard]] KALI_API std::string encode_base64(const void* data, size_t len);

/**
 * Encode data into base 64 encoding.
 */
[[nodiscard]] inline std::string encode_base64(std::span<const uint8_t> data)
{
    return encode_base64(data.data(), data.size());
}

/**
 * Decode data from base 64 encoding.
 */
[[nodiscard]] KALI_API std::vector<uint8_t> decode_base64(std::string_view str);

} // namespace kali
