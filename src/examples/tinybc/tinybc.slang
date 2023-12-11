/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/

import bctypes;

static bool USE_ADAM = CONFIG_USE_ADAM;
static uint OPT_STEPS = CONFIG_OPT_STEPS;

uniform float lr;
uniform float adam_beta_1;
uniform float adam_beta_2;

// Uncompressed input texture
Texture2D<float4> input_tex;

// Decoded BC texture
RWTexture2D<float4> decoded_tex;

[BackwardDifferentiable]
void l2_loss(BCTile tile, no_diff float4 target[16], out float loss[16])
{
    [ForceUnroll]
    for (int i = 0; i < 16; i++) {
        let delta = target[i] - tile.decode(i);
        loss[i] = dot(delta, delta);
    }
}

[shader("compute")]
[numthreads(4, 8, 1)]
void main(int3 tid: SV_DispatchThreadID)
{
    uint2 dim;
    input_tex.GetDimensions(dim.x, dim.y);

    let texel = 4 * tid.xy;
    if (any(texel >= dim))
        return;

    // Adam moments
    BCTile m1, m2;

    // Load target values and initialize with mid-point interpolation
    // and minmax endpoints computed over the tile
    float4 target[16];
    BCTile tile = BCTile(0.5, float4(1), float4(0));
    for (int i = 0; i < 16; i++) {
        target[i] = input_tex[texel + int2(i % 4, i / 4)];
        tile.min_end_points = min(tile.min_end_points, target[i]);
        tile.max_end_points = max(tile.max_end_points, target[i]);
    }

    // To compress the texture we perform a number of gradient descent steps
    // to joint-optimize interpolation weights and endpoints for each tile.
    // Eventually the interpolation weights are quantized and frozen,
    // while the endpoints are fine tuned for a few more steps.
    bool frozen_weight = false;
    let end_points_fine_tuninig_thres = 0.85f;
    for (uint s = 0; s < OPT_STEPS; s++) {
        // Forward pass - compute error
        float loss[16];
        l2_loss(tile, target, loss);

        // Backward pass - compute gradients
        var dp_tile = diffPair(tile);
        __bwd_diff(l2_loss)(dp_tile, target, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 });

        // Use Adam optimizer to update gradients
        BCTile grad = BCTile(dp_tile.d);
        if (USE_ADAM) {
            m1 = adam_beta_1 * m1 + (1 - adam_beta_1) * grad;
            m2 = adam_beta_2 * m2 + (1 - adam_beta_2) * (grad * grad);
            let unbiased_m1 = rcp(1.f - pow(adam_beta_1, 1 + s)) * m1;
            let unbiased_m2 = rcp(1.f - pow(adam_beta_2, 1 + s)) * m2;
            grad = unbiased_m1 * (1E-10f + unbiased_m2).rsqrt();
        }

        // If weights are frozen zero their gradient
        if (frozen_weight)
            grad.weights = BCWeights(0);

        // Update BC tile via gradient descent & clamp its values to a valid range
        tile = tile - lr * grad;
        tile.clamp();

        // Time to freeze the BC weights?
        if (frozen_weight == false && float(s) / OPT_STEPS >= end_points_fine_tuninig_thres) {
            // Quantize & freeze weights
            // We will continue to fine tune the endpoints for a few more steps
            frozen_weight = true;
            tile.weights.quantize();
        }
    }

    // Quantize & write out decoded tile
    tile.quantize();
    for (int i = 0; i < 16; i++)
        decoded_tex[texel + int2(i % 4, i / 4)] = tile.decode_exact(i);
}
