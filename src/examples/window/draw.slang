RWTexture2D g_output;
uniform uint g_frame;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint2 dims;
    g_output.GetDimensions(dims.x, dims.y);
    if (any(tid.xy >= dims))
        return;

    uint col = g_frame % dims.x;

    float2 uv = (tid.xy + 0.5) / dims;
    float4 color = float4(uv, 0, 1);
    if (tid.x == col)
        color = float4(1);
    g_output[tid.xy] = color;
}
