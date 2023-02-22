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
