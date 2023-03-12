#include "error.h"

#include <fmt/format.h>

namespace kali {

std::string get_assert_message(const char* cond, const char* file, int line)
{
    return fmt::format("ASSERT: {}\n{}:{}\n", cond, file, line);
}

} // namespace kali
