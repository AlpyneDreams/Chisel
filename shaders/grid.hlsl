#include "common.hlsli"

USE_CBUFFER(ObjectState, Object, 1);

struct Input
{
    float3 position : POSITION;
    float  major    : TEXCOORD0;
};

struct Varyings
{
    float4 position : SV_POSITION;
    float  farZ     : TEXCOORD0;
    float  alpha    : TEXCOORD1;
    float  z        : TEXCOORD2;
};

Varyings vs_main(Input i)
{
    float4x4 modelViewProj = mul(Camera.viewProj, Object.model);
    float4x4 modelView = mul(Camera.view, Object.model);

    Varyings v = (Varyings)0;

    v.position = mul(modelViewProj, float4(i.position, 1.0));

    // Fade out major gridlines further than minor ones
    v.farZ  = Camera.farZ * (i.major ? 1.0 : 0.5);
    v.alpha = 0.1 + (i.major * 0.1);
    v.z     = -mul(modelView, float4(i.position, 1.0)).z;

    return v;
}

float4 ps_main(Varyings v) : SV_TARGET
{
    float alpha = v.alpha * saturate(1 - (v.z / v.farZ));
    return float4(1, 1, 1, alpha);
}
