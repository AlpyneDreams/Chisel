#include "common.hlsli"

vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out)0;
    output.position = float4(input.position, 1.0);
    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    return float4( 1.0, 0.0, 1.0, 1.0 );
}
