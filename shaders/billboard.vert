$input a_position, a_texcoord0
$output v_texcoord0

#include "common.glsl"

void main()
{
    vec3 camRight = vec3(u_invView[0][0], u_invView[1][0], u_invView[2][0]);
    vec3 camUp    = vec3(u_invView[0][1], u_invView[1][1], u_invView[2][1]);
	vec3 pos      = (camRight * a_position.x) + (camUp * a_position.y);

    gl_Position = mul(u_modelViewProj, vec4(pos, 1));
    v_texcoord0 = a_texcoord0;
}
