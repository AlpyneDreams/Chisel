#include "common.hlsli"

// TODO: Single unified shader for models and brushes
USE_CBUFFER(ObjectState, Object, 1);

struct Input
{
    float3 position : POSITION;
    float3 normal   : NORMAL0;
    float3 uv       : TEXCOORD0;
    uint   face     : BLENDINDICES0;
};

struct Varyings
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL0;
    float3 uv       : TEXCOORD0;
    float3 view     : TEXCOORD1;
    uint   id       : BLENDINDICES0;
};

Varyings vs_main(Input i)
{
    Varyings v = (Varyings)0;

    float4 pos = mul(Object.model, float4(i.position, 1.0));
    v.position = mul(Camera.viewProj, pos);
    v.normal   = i.normal;
    v.view     = mul(Camera.view, pos).xyz;
    v.uv       = i.uv;
    v.id       = Object.id == 0 ? i.face : Object.id;

    return v;
}

struct Output
{
    float4 color : SV_TARGET0;
    uint   id    : SV_TARGET1;
};
