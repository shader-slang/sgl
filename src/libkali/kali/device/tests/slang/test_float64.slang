RWStructuredBuffer<uint64_t> result;

#if USE_UAV
RWStructuredBuffer<float64_t> data;
#else
StructuredBuffer<float64_t> data;
#endif

[shader("compute")]
[numthreads(16, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;

    float64_t value = data[i]; // 64-bit load

    uint2 tmp;
    asuint(value, tmp.x, tmp.y);
    result[i] = uint64_t(tmp.x) | uint64_t(tmp.y) << 32;
}
