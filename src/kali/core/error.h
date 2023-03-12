#pragma once

#include "platform.h"

#include <stdexcept>
#include <string>

namespace kali {

KALI_API std::string get_assert_message(const char* cond, const char* file, int line);

#define KALI_THROW(msg)                                                                                                \
    {                                                                                                                  \
        std::string str{msg};                                                                                          \
        fprintf(stderr, str.c_str());                                                                                  \
        throw std::runtime_error(msg);                                                                                 \
    }

#define KALI_ASSERT(cond)                                                                                              \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
            KALI_THROW(::kali::get_assert_message(#cond, __FILE__, __LINE__));                                         \
    }

} // namespace kali
