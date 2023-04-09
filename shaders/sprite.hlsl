#include "common.hlsli"

struct Input
{
    float3 position : POSITION;
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

    float3 camRight = float3(Camera.invView[0][0], Camera.invView[1][0], Camera.invView[2][0]);
    float3 camUp    = float3(Camera.invView[0][1], Camera.invView[1][1], Camera.invView[2][1]);
	float3 pos      = (camRight * i.position.x) + (camUp * i.position.y);

    v.position = mul(Object.modelViewProj, float4(pos, 1));
    v.uv       = i.uv;

    return v;
}

float4 ps_main(Varyings v) : SV_TARGET
{
    return s_texture.Sample(s_sampler, v.uv);
}
