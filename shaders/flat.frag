$input v_normal, v_view

#include "common.glsl"

uniform vec4 u_color;

void main()
{
    vec3 light 		= normalize(vec3(1.0, 3.0, 2.0));
	vec3 ambient	= vec3_splat(0.05);
	vec3 color 		= u_color.rgb;
	vec3 normal		= normalize(v_normal);
	vec3 view 		= normalize(v_view);

	float NoL		= saturate(dot(normal, light));

	// Half lambert
	NoL = 0.5 + (0.5 * NoL);

	gl_FragColor.rgb = ambient + (color * NoL);
	gl_FragColor.a = 1;
}
