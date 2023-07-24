#include "common.hlsli"
#include "brush.hlsli"

Texture2D    s_texture  : register(t0);
SamplerState s_sampler  : register(s0);

Output ps_main(Varyings v)
{
    Output o = (Output)0;

    float4 baseColor  = s_texture.Sample(s_sampler, v.uv.xy);

    o.color.rgb = Lighting(v.normal, v.view) * baseColor.rgb * Brush.color.rgb;
    o.color.a   = baseColor.a;
    o.id        = v.id;
    return o;
}
