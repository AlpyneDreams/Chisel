$input v_texcoord0

#include "common.glsl"

SAMPLER2D(s_texColor, 0);

void main()
{
    gl_FragColor = texture2D(s_texColor, vec2(v_texcoord0));
}
