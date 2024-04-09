// SPDX-License-Identifier: Apache-2.0

RWStructuredBuffer<uint16_t> result;

#if USE_UAV
RWStructuredBuffer<float16_t> data;
#else
StructuredBuffer<float16_t> data;
#endif

[shader("compute")]
[numthreads(16, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;

    float16_t value = data[i]; // 16-bit load

    result[i] = uint16_t(f32tof16(value));
}
