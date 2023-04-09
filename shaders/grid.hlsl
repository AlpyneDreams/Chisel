#include "common.hlsli"

struct vs_in
{
    float3 position : POSITION;
    float  major    : TEXCOORD0;
};

struct vs_out
{
    float4 position : SV_POSITION;
    float  farZ     : TEXCOORD0;
    float  alpha    : TEXCOORD1;
    float  z        : TEXCOORD2;
};

vs_out vs_main(vs_in i)
{
    vs_out o = (vs_out)0;

    o.position = mul(Object.modelViewProj, float4(i.position, 1.0));

    // Fade out major gridlines further than minor ones
    o.farZ  = Camera.farZ * (i.major ? 1.0 : 0.5);
    o.alpha = 0.1 + (i.major * 0.1);
    o.z     = -mul(Object.modelView, float4(i.position, 1.0)).z;

    return o;
}

float4 ps_main(vs_out i) : SV_TARGET
{
    float alpha = i.alpha * saturate(1 - (i.z / i.farZ));
    return float4( 1, 1, 1, alpha );
}

