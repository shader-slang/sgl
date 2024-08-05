// SPDX-License-Identifier: Apache-2.0

SamplerState sampler;
Texture2D texture;
uniform float2 scale;
uniform float2 offset;
uniform bool is_srgb_format;

struct VsIn {
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 col : COLOR;
};

struct VsOut {
    float4 col : COLOR;
    float2 uv : TEXCOORD0;
    float4 pos : SV_Position;
};

float srgb_to_linear(float srgb)
{
    return srgb < 0.04045f ? srgb * (1.f / 12.92f) : pow((srgb + 0.055f) * (1.f / 1.055f), 2.4f);
}

float3 srgb_to_linear(float3 srgb)
{
    return float3(srgb_to_linear(srgb.r), srgb_to_linear(srgb.g), srgb_to_linear(srgb.b));
}

[shader("vertex")]
VsOut vs_main(VsIn vs_in)
{
    VsOut vs_out;
    vs_out.col = vs_in.col;
    vs_out.uv = vs_in.uv;
    vs_out.pos = float4(vs_in.pos * scale + offset, 0.f, 1.f);
    return vs_out;
}

[shader("fragment")]
float4 fs_main(VsOut vs_out)
    : SV_Target
{
    float4 col = vs_out.col * texture.Sample(sampler, vs_out.uv);
    if (is_srgb_format)
        col.rgb = srgb_to_linear(col.rgb);
    return col;
}
