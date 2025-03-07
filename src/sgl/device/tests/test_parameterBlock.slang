
struct NestedStruct
{
    float aa;
    int bb;
    RWStructuredBuffer<float> nc;
}

struct MyStruct
{
    NestedStruct a;
    uint b;
    ParameterBlock<NestedStruct> nestParamBlock;
    RWStructuredBuffer<float> c;
};

ParameterBlock<MyStruct> inputStruct;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    inputStruct.c[0] = inputStruct.a.aa + inputStruct.a.bb + inputStruct.b +
        inputStruct.nestParamBlock.aa + inputStruct.nestParamBlock.bb + inputStruct.nestParamBlock.nc[0];
}
