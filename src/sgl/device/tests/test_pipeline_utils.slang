
[shader("compute")]
[numthreads(16, 16, 1)]
void clear(uint3 tid: SV_DispatchThreadID, RWTexture2D<float4> render_texture)
{
    uint2 dim;
    render_texture.GetDimensions(dim.x, dim.y);
    if (any(tid.xy >= dim))
        return;
    render_texture[tid.xy] = float4(0, 0, 0, 0);
}

[shader("compute")]
[numthreads(16, 16, 1)]
void count(uint3 tid: SV_DispatchThreadID, Texture2D<float4> render_texture, RWByteAddressBuffer count_buffer)
{
    uint2 dim;
    render_texture.GetDimensions(dim.x, dim.y);
    if (any(tid.xy >= dim))
        return;
    float4 val = render_texture[tid.xy];
    count_buffer.InterlockedAdd(0, val.x > 0 ? 1 : 0);
    count_buffer.InterlockedAdd(4, val.y > 0 ? 1 : 0);
    count_buffer.InterlockedAdd(8, val.z > 0 ? 1 : 0);
    count_buffer.InterlockedAdd(12, val.w > 0 ? 1 : 0);
}

[shader("compute")]
[numthreads(16, 16, 1)]
void setcolor(
    uint3 tid: SV_DispatchThreadID,
    RWTexture2D<float4> render_texture,
    uniform int2 pos,
    uniform int2 size,
    uniform float4 color
)
{
    if (any(tid.xy >= size))
        return;
    render_texture[tid.xy + pos] = color;
}


struct V2F {
    float4 pos : SV_Position;
};

float2 vert_offset;
float2 vert_scale;
float vert_z;

[shader("vertex")]
V2F vertex_main(float3 pos: POSITION)
{
    V2F o;
    o.pos = float4(pos.xy * vert_scale.xy + vert_offset.xy, vert_z, 1);
    return o;
}

float4 frag_color;

[shader("fragment")]
float4 fragment_main(V2F v)
    : SV_Target
{
    return frag_color;
}

[shader("fragment")]
float4 fragment_tex_main(V2F v)
    : SV_Target
{
    return frag_texture.Sample(frag_sampler_state, v.pos.xy, 0);
}

[shader("compute")]
[numthreads(16, 16, 1)]
void raygrid(
    uint3 tid: SV_DispatchThreadID,
    RWTexture2D<float4> render_texture,
    RaytracingAccelerationStructure tlas,
    uniform int2 pos,
    uniform int2 size,
    uniform float dist
)
{
    if (any(tid.xy >= size))
        return;

    RayDesc ray;
    ray.Origin = float3(tid.xy, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 2;

    RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(tlas, 0, 0xff, ray);
    q.Proceed();
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
        float4 color = float4(0, 0, 0, 1);
        color[q.CommittedInstanceID() % 3] = 1;
        render_texture[tid.xy] = color;
    } else {
        render_texture[tid.xy] = float4(0, 0, 0, 1);
    }
}

struct Payload {
    float4 color;
}

[shader("miss")]
void rt_miss(inout Payload payload)
{
    payload.color = float4(0, 0, 0, 1);
}

[shader("closesthit")]
void rt_closest_hit(inout Payload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    float4 col = float4(0, 0, 0, 1);
    col[InstanceID() % 3] = 1;
    payload.color = col;
}

RWTexture2D<float4> rt_render_texture;
RaytracingAccelerationStructure rt_tlas;

[shader("raygeneration")]
void rt_ray_gen()
{
    uint2 pixel = DispatchRaysIndex().xy;

    RayDesc ray;
    ray.Origin = float3(pixel.xy, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 2;

    Payload payload = {};

    TraceRay(
        rt_tlas,
        0,
        0xff,
        0 /* RayContributionToHitGroupIndex */,
        0 /* MultiplierForGeometryContributionHitGroupIndex */,
        0 /* MissShaderIndex */,
        ray,
        payload
    );

    rt_render_texture[pixel] = payload.color;
}
