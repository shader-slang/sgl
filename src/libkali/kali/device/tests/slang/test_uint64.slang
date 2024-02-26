// SPDX-License-Identifier: Apache-2.0

RWStructuredBuffer<uint> result;

#if USE_UAV
RWStructuredBuffer<uint64_t> data;
#else
StructuredBuffer<uint64_t> data;
#endif

[shader("compute")]
[numthreads(16, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;

    uint64_t value = data[i]; // 64-bit load

    uint lo = (uint)value;
    uint hi = (uint)(value >> 32);
    result[2 * i] = lo;
    result[2 * i + 1] = hi;
}
