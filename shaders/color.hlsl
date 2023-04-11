#include "common.hlsli"

USE_CBUFFER(ObjectState, Object, 1);

struct Output
{
    float4 color : SV_TARGET0;
    uint   id    : SV_TARGET1;
};

float4 vs_main(float3 pos : POSITION) : SV_POSITION
{
    return mul(mul(Camera.viewProj, Object.model), float4(pos, 1.0));
}

Output ps_main(float4 pos : SV_POSITION)
{
    Output o = (Output)0;
    o.color = Object.color;
    o.id = Object.id;
    return o;
}
