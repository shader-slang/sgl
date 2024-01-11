RWTexture2D g_output;
uniform uint g_frame;
uniform float2 g_mouse_pos;
uniform bool g_mouse_down;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint2 dims;
    uint2 pixel = tid.xy;
    g_output.GetDimensions(dims.x, dims.y);
    if (any(pixel >= dims))
        return;

    float3 color = float3(smoothstep(500, 0, distance(pixel, g_mouse_pos)) * 0.5);

    uint col = g_frame % dims.x;
    if (abs(tid.x - col) < 3)
        color = float3(1);
    if (distance(pixel, g_mouse_pos) < (g_mouse_down ? 20 : 10))
        color = float3(1);
    g_output[pixel] = float4(color, 1);
}
