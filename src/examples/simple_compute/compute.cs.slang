RWStructuredBuffer<uint> g_buffer;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x < 1024)
        g_buffer[tid.x] = tid.x;
}
