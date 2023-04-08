#include "common.hlsli"

/*
$input a_position, a_texcoord0
$output v_texcoord0, v_color0

#include "common.glsl"

#define v_farz  v_color0.x
#define v_alpha v_texcoord0.y
#define v_z     v_texcoord0.z

uniform vec4 u_gridFarZ = vec4(10000, 0, 0, 0);

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

    float major = a_texcoord0.x;

    // Fade out major gridlines further than minor ones
    v_farz  = u_gridFarZ.x * (major ? 1.0 : 0.5);
    v_alpha = 0.1 + (major * 0.1);
    v_z     = -mul(u_modelView, vec4(a_position, 1.0)).z;
}
===========================================================
$input v_texcoord0, v_color0

#include "common.glsl"

void main()
{
    float alpha = v_alpha * saturate(1 - (v_z / v_farz));
    gl_FragColor = vec4(1, 1, 1, alpha);
}
*/

#define v_farZ    color.x
#define v_alpha   color.y
#define v_z       color.z


vs_out vs_main(vs_in input)
{
    vs_out output = (vs_out)0;
    output.position = float4(input.position, 1.0);//mul(g_modelViewProj, float4(input.position, 1.0));

    float major = input.uv.x;

    // Fade out major gridlines further than minor ones
    output.v_farZ  = g_farZ.x * (major ? 1.0 : 0.5);
    output.v_alpha = 0.1 + (major * 0.1);
    output.v_z     = -mul(g_modelView, float4(input.position, 1.0)).z;

    return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
    return float4( 1.0, 0.0, 1.0, 1.0 );
}
