// SPDX-License-Identifier: Apache-2.0

extern static const uint NUM_THREADS;
extern static const float VALUE;

RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(NUM_THREADS, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    result[tid.x] = tid.x * VALUE;
}
