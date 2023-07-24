#include "common.hlsli"
#include "brush.hlsli"

Output ps_main(Varyings v, uint pid : SV_PRIMITIVEID)
{
    Output o = (Output)0;

    o.color.rgb = DebugSelectionID(v.id);
    o.color.a   = 1.0f;
    o.id        = v.id;
    return o;
}
