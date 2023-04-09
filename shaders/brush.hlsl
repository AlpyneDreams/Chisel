#include "common.hlsli"

struct vs_in
{
    float3 position : POSITION;
    float3 normal   : NORMAL0;
    float2 uv       : TEXCOORD0;
};

struct vs_out
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

Texture2D s_texture: register(t0);
SamplerState s_sampler: register(s0);

vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out)0;

    output.position = mul(Camera.viewProj, float4(input.position, 1.0));
    output.uv = input.uv;

    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    return float4(s_texture.Sample(s_sampler, input.uv).xyz, 1.0);
}
