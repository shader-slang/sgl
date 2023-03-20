#pragma once

#include <cstdint>

namespace kali {

using uint = uint32_t;

template<typename T>
struct ScalarTraits { };

template<>
struct ScalarTraits<float> {
    static constexpr const char* name{"float"};
};

template<>
struct ScalarTraits<uint> {
    static constexpr const char* name{"uint"};
};

template<>
struct ScalarTraits<int> {
    static constexpr const char* name{"int"};
};

template<>
struct ScalarTraits<bool> {
    static constexpr const char* name{"bool"};
};


} // namespace kali
