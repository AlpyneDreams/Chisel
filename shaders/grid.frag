$input v_texcoord0, v_color0

#include "common.glsl"

#define v_farz  v_color0.x
#define v_alpha v_texcoord0.y
#define v_z     v_texcoord0.z

void main()
{
    float alpha = v_alpha * saturate(1 - (v_z / v_farz));
    gl_FragColor = vec4(1, 1, 1, alpha);
}
