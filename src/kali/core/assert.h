#pragma once

#include "format.h"
#include "error.h"

// TODO move to config
#define KALI_ENABLE_ASSERTS 1

#if KALI_ENABLE_ASSERTS

#define KALI_ASSERT(cond)                                                                                              \
    if (!(cond)) {                                                                                                     \
        std::string str = fmt::format("Assertion failed: {}\n{}:{}", #cond, __FILE__, __LINE__);                       \
        KALI_THROW(Exception(str));                                                                                    \
    }

#define KALI_ASSERT_MSG(cond, msg)                                                                                     \
    if (!(cond)) {                                                                                                     \
        std::string str = fmt::format("Assertion failed: {} ({})\n{}:{}", #cond, msg, __FILE__, __LINE__);             \
        KALI_THROW(Exception(str));                                                                                    \
    }

#define KALI_ASSERT_OP(a, b, op)                                                                                       \
    if (!(a op b)) {                                                                                                   \
        std::string str                                                                                                \
            = fmt::format("Assertion failed: {} {} {} ({} {} {})\n{}:{}", #a, #op, #b, a, #op, b, __FILE__, __LINE__); \
        KALI_THROW(Exception(str));                                                                                    \
    }

#else // KALI_ENABLE_ASSERTS

#define KALI_ASSERT(a)                                                                                                 \
    {                                                                                                                  \
    }
#define KALI_ASSERT_MSG(a, msg)                                                                                        \
    {                                                                                                                  \
    }
#define KALI_ASSERT_OP(a, b, op)                                                                                       \
    {                                                                                                                  \
    }

#endif // KALI_ENABLE_ASSERTS

#define KALI_ASSERT_EQ(a, b) KALI_ASSERT_OP(a, b, ==)
#define KALI_ASSERT_NE(a, b) KALI_ASSERT_OP(a, b, !=)
#define KALI_ASSERT_GE(a, b) KALI_ASSERT_OP(a, b, >=)
#define KALI_ASSERT_GT(a, b) KALI_ASSERT_OP(a, b, >)
#define KALI_ASSERT_LE(a, b) KALI_ASSERT_OP(a, b, <=)
#define KALI_ASSERT_LT(a, b) KALI_ASSERT_OP(a, b, <)
