$input a_position, a_normal
$output v_normal, v_view

#include "bgfx_shader.sh"

void main()
{
	vec3 pos = a_position;
	vec3 normal = a_normal.xyz*2.0 - 1.0;

	gl_Position = mul(u_modelViewProj, vec4(pos, 1));
	v_normal = mul(u_model[0], vec4(normal, 0)).xyz;
	v_view = mul(u_modelView, vec4(pos, 1)).xyz;
}
