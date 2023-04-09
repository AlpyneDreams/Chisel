cbuffer CameraState : register(b0)
{
    float4x4 g_viewProj;
    float g_farZ;
};

cbuffer ObjectState : register(b1)
{
    float4x4 g_modelViewProj;
    float4x4 g_modelView;
};
