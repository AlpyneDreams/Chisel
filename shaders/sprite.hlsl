#include "common.hlsli"

USE_CBUFFER(ObjectState, Object, 1);

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

struct Output
{
    float4 color : SV_TARGET0;
    uint   id    : SV_TARGET1;
};

Texture2D    s_texture : register(t0);
SamplerState s_sampler : register(s0);

Varyings vs_main(Input i)
{
    float4x4 modelViewProj = mul(Camera.viewProj, Object.model);
    float3x3 invViewAxes = transpose((float3x3)Camera.view);

    Varyings v = (Varyings)0;

    float3 camRight = float3(invViewAxes[0][0], invViewAxes[1][0], invViewAxes[2][0]);
    float3 camUp    = float3(invViewAxes[0][1], invViewAxes[1][1], invViewAxes[2][1]);
	float3 pos      = (camRight * i.position.x) + (camUp * i.position.y);

    v.position = mul(modelViewProj, float4(pos, 1));
    v.uv       = i.uv;

    return v;
}

Output ps_main(Varyings v)
{
    Output o = (Output)0;
    
    o.color = s_texture.Sample(s_sampler, v.uv) * Object.color;
    o.id = Object.id;
    
    return o;
}
