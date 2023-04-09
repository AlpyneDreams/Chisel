#include "common.hlsli"

struct Input
{
    float3 position : POSITION;
    float3 normal   : NORMAL0;
    float2 uv       : TEXCOORD0;
};

struct Varyings
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

Texture2D    s_texture : register(t0);
SamplerState s_sampler : register(s0);

Varyings vs_main(Input i)
{
    Varyings v = (Varyings)0;

    v.position = mul(Camera.viewProj, float4(i.position, 1.0));
    v.uv       = i.uv;

    return v;
}

float4 ps_main(Varyings v) : SV_TARGET
{
    return float4(s_texture.Sample(s_sampler, v.uv).xyz, 1.0);
}
