#pragma once

#include <cstdlib>

namespace kali {

template<typename T, size_t N>
struct vector { };

template<typename T>
struct vector<T, 2> {
    T x, y;
};

template<typename T>
struct vector<T, 3> {
    T x, y, z;
};

template<typename T>
struct vector<T, 4> {
    T x, y, z, w;
};

template<typename T>
vector<T, 2> operator+(const vector<T, 2>& lhs, const vector<T, 2>& rhs)
{
    return vector<T, 2>{lhs.x + rhs.x, lhs.y + lhs.y};
}

using float2 = vector<float, 2>;
using float3 = vector<float, 3>;
using float4 = vector<float, 4>;

} // namespace kali
