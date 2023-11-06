struct Test {
    int a;
    float b;
    StructuredBuffer<int> c;
};

// struct BasicTypes {
//     uint16_t u16;
//     uint16_t2 u16_2;
//     uint16_t3 u16_3;
//     uint16_t4 u16_4;

//     uint32_t u32;
//     uint32_t2 u32_2;
//     uint32_t3 u32_3;
//     uint32_t4 u32_4;

//     uint64_t u64;
//     uint64_t2 u64_2;
//     uint64_t3 u64_3;
//     uint64_t4 u64_4;

//     float4x4 f4x4;
//     float4x3 f4x3;
// }

StructuredBuffer<uint> buffer_a;
StructuredBuffer<uint> buffer_b;
RWStructuredBuffer<uint> buffer_c;

struct Processor {
    StructuredBuffer<uint> a;
    StructuredBuffer<uint> b;
    RWStructuredBuffer<uint> c;
    void process(uint index) { c[index] = a[index] + b[index]; }
};

ParameterBlock<Processor> processor;

// Test g_array[4];

// ConstantBuffer<BasicTypes> cb_basic_types;

// BasicTypes g_basic_types;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint index = tid.x;
    if (index < 1024)
        processor.process(index);
    // if (index < 1024)
    //     buffer_c[index] = buffer_a[index] + buffer_b[index];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void main2(
    uint3 tid: SV_DispatchThreadID,
    uniform uint N,
    uniform StructuredBuffer<uint> a,
    uniform StructuredBuffer<uint> b,
    uniform RWStructuredBuffer<uint> c
)
{
    uint index = tid.x;
    if (index < N)
        c[index] = a[index] + b[index];
}
