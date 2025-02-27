struct MyStruct
{
    float a;
    int b;
    uint c;
    RWStructuredBuffer<float> d;
};

ParameterBlock<MyStruct> inputStruct;

struct Output
{
    RWStructuredBuffer<float> d;
}

uniform Output output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    inputStruct.d[0] = inputStruct.a + inputStruct.b + inputStruct.c;
}
