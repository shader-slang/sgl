// SPDX-License-Identifier: Apache-2.0

#pragma once

// ImGui configuration file. See imconfig.h for more details.

#include "kali/math/vector_types.h"

#define IM_VEC2_CLASS_EXTRA                                                                                            \
    constexpr ImVec2(const ::kali::float2& f)                                                                          \
        : x(f.x)                                                                                                       \
        , y(f.y)                                                                                                       \
    {                                                                                                                  \
    }                                                                                                                  \
    operator ::kali::float2() const                                                                                    \
    {                                                                                                                  \
        return ::kali::float2(x, y);                                                                                   \
    }

#define IM_VEC3_CLASS_EXTRA                                                                                            \
    constexpr ImVec3(const ::kali::float3& f)                                                                          \
        : x(f.x)                                                                                                       \
        , y(f.y)                                                                                                       \
        , z(f.z)                                                                                                       \
    {                                                                                                                  \
    }                                                                                                                  \
    operator ::kali::float3() const                                                                                    \
    {                                                                                                                  \
        return ::kali::float3(x, y, z);                                                                                \
    }

#define IM_VEC4_CLASS_EXTRA                                                                                            \
    constexpr ImVec4(const ::kali::float4& f)                                                                          \
        : x(f.x)                                                                                                       \
        , y(f.y)                                                                                                       \
        , z(f.z)                                                                                                       \
        , w(f.w)                                                                                                       \
    {                                                                                                                  \
    }                                                                                                                  \
    operator ::kali::float4() const                                                                                    \
    {                                                                                                                  \
        return ::kali::float4(x, y, z, w);                                                                             \
    }
