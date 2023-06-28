#pragma once

#include "platform.h"

#include <string>

namespace kali {

KALI_API std::string to_lower(std::string str);
KALI_API std::string to_upper(std::string str);

inline std::wstring to_wstring(std::string str)
{
    return std::wstring(str.begin(), str.end());
}

// inline std::string to_string(std::wstring str)
// {
//     return std::string(str.begin(), str.end());
// }

} // namespace kali
