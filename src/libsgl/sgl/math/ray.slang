// SPDX-License-Identifier: Apache-2.0

import constants;

/**
 * Ray type.
 * This has equal layout to DXR RayDesc but adds additional functionality.
 */
struct Ray {
    float3 origin;
    float t_min;
    float3 dir;
    float t_max;

    /// Initializer.
    __init(float3 origin, float3 dir, float t_min = 0.f, float t_max = FLT_MAX)
    {
        this.origin = origin;
        this.dir = dir;
        this.t_min = t_min;
        this.t_max = t_max;
    }

    /// Convert to DXR RayDesc.
    RayDesc to_ray_desc() { return { origin, t_min, dir, t_max }; }

    /// Evaluate position on the ray at t.
    float3 eval(float t) { return origin + t * dir; }
};
