#include "string_utils.h"

#include <cctype>

namespace kali {

std::string to_lower(std::string str)
{
    for (char& c : str)
        c = char(::tolower(c));
    return str;
}

std::string to_upper(std::string str)
{
    for (char& c : str)
        c = char(::toupper(c));
    return str;
}

} // namespace kali
