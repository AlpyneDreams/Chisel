struct vs_in {
    float3 position : POS;
    float2 uv       : TEXCOORD0;
};

struct vs_out
{
    float4 position : SV_POSITION; // clip space position
    float3 color    : COLOR0;      // color
    float2 uv       : TEXCOORD0;   // texture coordinates
};

// See src/render/CBuffers.h

cbuffer Scene : register(b0)
{
    float g_farZ;
};

cbuffer Object : register(b1)
{
    float4x4 g_modelViewProj;
    float4x4 g_modelView;
};
