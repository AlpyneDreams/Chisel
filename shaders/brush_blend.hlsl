#include "common.hlsli"
#include "brush.hlsli"

// TODO:
// - 4-way blend (Lightmapped_4WayBlend, MultiBlend, etc.)
// - Blend modulation
// - Masked blending
// - Blending of normal maps, etc.

Texture2D    s_texture  : register(t0);
Texture2D    s_texture2 : register(t1);
SamplerState s_sampler  : register(s0);

Output ps_main(Varyings v)
{
    Output o = (Output)0;

    float4 baseColor  = s_texture.Sample(s_sampler, v.uv.xy);
    float4 baseColor2 = s_texture2.Sample(s_sampler, v.uv.xy);

    float blendFactor = v.uv.z;
    baseColor         = lerp(baseColor, baseColor2, blendFactor);
    float alpha       = lerp(baseColor.a, baseColor2.a, blendFactor);

    o.color.rgb = Lighting(v.normal, v.view) * baseColor.rgb * Brush.color.rgb;
    o.color.a   = alpha;
    o.id        = v.id;
    return o;
}
