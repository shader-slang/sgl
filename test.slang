[format("rgba8")]
RWTexture2D<float4> output;

[shader("compute")]
[numthreads(16, 16, 1)]
void main2(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = tid.xy / float2(512);
    output[tid.xy] = float4(uv, 0, 255);
}
