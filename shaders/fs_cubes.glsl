$input v_normal, v_view

#include "bgfx_shader.sh"

// TEMP: Placeholder shading

vec2 Blinn(vec3 _lightDir, vec3 _normal, vec3 _viewDir)
{
	float ndotl = dot(_normal, _lightDir);
	vec3 reflected = _lightDir - 2.0*ndotl*_normal; // reflect(_lightDir, _normal);
	float rdotv = dot(reflected, _viewDir);
	return vec2(ndotl, rdotv);
}

float Fresnel(float _ndotl, float _bias, float _pow)
{
	float facing = (1.0 - _ndotl);
	return max(_bias + (1.0 - _bias) * pow(facing, _pow), 0.0);
}

vec4 Lit(float _ndotl, float _rdotv, float _m)
{
	float diff = max(0.0, _ndotl);
	float spec = step(0.0, _ndotl) * max(0.0, _rdotv * _m);
	return vec4(1.0, diff, spec, 1.0);
}


void main()
{
    vec3 light 		= normalize(vec3(0.25, 0.75, -0.5));
	vec3 ambient	= vec3(0.07, 0.06, 0.08);
	vec3 color 		= vec3(1, 1, 1);
	vec3 normal		= normalize(v_normal);
	vec3 view 		= normalize(v_view);
	vec2 blinn		= Blinn(light, normal, view);
	vec4 lit		= Lit(blinn.x, blinn.y, 1);
	float fresnel	= Fresnel(blinn.x, 0.2, 5);
    
	gl_FragColor.rgb = pow(ambient + color*lit.y + fresnel*pow(lit.z, 128), vec3(1.0/2.2) );
	gl_FragColor.a = 1;
}
