$input v_normal, v_view

#include "bgfx_shader.sh"

uniform vec4 u_color;

void main()
{
    vec3 light 		= normalize(vec3(1.0, 3.0, 2.0));
	vec3 color 		= u_color.rgb;
	vec3 normal		= normalize(v_normal);
	vec3 view 		= normalize(v_view);

	float NoL = clamp(dot(normal, light), 0.0, 1.0);
	float ambient = 0.25;

	gl_FragColor.rgb = color * max(NoL, ambient);
	gl_FragColor.a = 1;
}
