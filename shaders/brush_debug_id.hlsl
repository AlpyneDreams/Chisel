#include "common.hlsli"
#include "brush.hlsli"

Output ps_main(Varyings v, uint pid : SV_PRIMITIVEID)
{
    Output o = (Output)0;

    o.color.rgb = DebugSelectionID(Brush.id);
    o.color.a   = 1.0f;
    o.id        = Brush.id;
    return o;
}
