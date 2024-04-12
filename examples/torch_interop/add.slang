// SPDX-License-Identifier: Apache-2.0

[shader("compute")]
[numthreads(64, 1, 1)]
void main(
    uint tid: SV_DispatchThreadID,
    StructuredBuffer<float> a,
    StructuredBuffer<float> b,
    RWStructuredBuffer<float> c
)
{
    c[tid.x] = a[tid.x] + b[tid.x];
}
