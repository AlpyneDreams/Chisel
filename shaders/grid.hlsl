#include "cbuffers.hlsli"

struct vs_in
{
    float3 position : POSITION;
    float  major    : TEXCOORD0;
};

struct vs_out
{
    float4 position : SV_POSITION;
};

vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out)0;

    output.position = mul(g_viewProj, float4(input.position, 1.0));

    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    return float4( 1.0, 1.0, 1.0, 1.0 );
}
