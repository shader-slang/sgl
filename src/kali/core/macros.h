#pragma once

#define KALI_UNUSED(x) (void)x

/**
 * Preprocessor stringification.
 */
#define KALI_STRINGIZE(a) #a
#define KALI_CONCAT_STRINGS_(a, b) a##b
#define KALI_CONCAT_STRINGS(a, b) KALI_CONCAT_STRINGS_(a, b)

// clang-format off
/// Implement logical operators on a class enum for making it usable as a flags enum.
#define KALI_ENUM_CLASS_OPERATORS(e_) \
    inline e_ operator& (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)& static_cast<int>(b)); } \
    inline e_ operator| (e_ a, e_ b) { return static_cast<e_>(static_cast<int>(a)| static_cast<int>(b)); } \
    inline e_& operator|= (e_& a, e_ b) { a = a | b; return a; }; \
    inline e_& operator&= (e_& a, e_ b) { a = a & b; return a; }; \
    inline e_  operator~ (e_ a) { return static_cast<e_>(~static_cast<int>(a)); } \
    inline bool is_set(e_ val, e_ flag) { return (val & flag) != static_cast<e_>(0); } \
    inline void flip_bit(e_& val, e_ flag) { val = is_set(val, flag) ? (val & (~flag)) : (val | flag); }
// clang-format on
