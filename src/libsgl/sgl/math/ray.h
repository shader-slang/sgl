// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/math/vector.h"

#include <limits>

namespace sgl {

/**
 * Ray type.
 * This should match the layout of DXR RayDesc.
 */
struct Ray {
    float3 origin;
    float t_min;
    float3 dir;
    float t_max;

    /// Default constructor (uninitialized).
    Ray() = default;

    /// Constructor.
    explicit Ray(float3 origin_, float3 dir_, float t_min_ = 0.f, float t_max_ = std::numeric_limits<float>::max())
        : origin(origin_)
        , t_min(t_min_)
        , dir(dir_)
        , t_max(t_max_)
    {
    }

    /// Evaluate position on the ray at t.
    [[nodiscard]] float3 eval(float t) const { return origin + t * dir; }

    /// Evaluate position on the ray at t.
    [[nodiscard]] float3 operator()(float t) const { return eval(t); }
};

// These are to ensure that the struct Ray match DXR RayDesc.
static_assert(offsetof(Ray, origin) == 0);
static_assert(offsetof(Ray, t_min) == sizeof(float3));
static_assert(offsetof(Ray, dir) == offsetof(Ray, t_min) + sizeof(float));
static_assert(offsetof(Ray, t_max) == offsetof(Ray, dir) + sizeof(float3));
static_assert(sizeof(Ray) == 32);

} // namespace sgl
