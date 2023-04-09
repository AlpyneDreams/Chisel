#include "common.hlsli"

USE_CBUFFER(BrushState, Brush, 1);

struct Input
{
    float3 position : POSITION;
    float3 normal   : NORMAL0;
    float2 uv       : TEXCOORD0;
};

struct Varyings
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL0;
    float2 uv       : TEXCOORD0;
    float3 view     : TEXCOORD1;
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
    Varyings v = (Varyings)0;

    v.position = mul(Camera.viewProj, float4(i.position, 1.0));
    v.normal   = i.normal;
    v.view     = mul(Camera.view, float4(i.position, 1.0)).xyz;
    v.uv       = i.uv;

    return v;
}

Output ps_main(Varyings v)
{
    Output o = (Output)0;

    float3 light 		= normalize(float3(1.0, 3.0, 2.0));
	float3 color 		= float3(1, 1, 1); // previously u_color
	float3 normal		= normalize(v.normal);
	float3 view 		= normalize(v.view);

	float NoL		    = saturate(dot(normal, light));

	// Half lambert
	NoL = 0.5 + (0.5 * NoL);
    
    float4 baseColor = s_texture.Sample(s_sampler, v.uv);

    o.color.rgb = color * NoL * baseColor.rgb;
    o.color.a   = baseColor.a;
    o.id = Brush.id;
    return o;
}
