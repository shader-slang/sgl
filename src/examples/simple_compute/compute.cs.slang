struct Test {
    int a;
    float b;
    StructuredBuffer<int> c;
};

struct BasicTypes {
    uint16_t u16;
    uint16_t2 u16_2;
    uint16_t3 u16_3;
    uint16_t4 u16_4;

    uint32_t u32;
    uint32_t2 u32_2;
    uint32_t3 u32_3;
    uint32_t4 u32_4;

    uint64_t u64;
    uint64_t2 u64_2;
    uint64_t3 u64_3;
    uint64_t4 u64_4;

    float4x4 f4x4;
    float4x3 f4x3;
}

RWStructuredBuffer<uint> g_buffer;

Test g_array[4];

ConstantBuffer<BasicTypes> cb_basic_types;

BasicTypes g_basic_types;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x < 1024)
        g_buffer[tid.x] = tid.x;
}
