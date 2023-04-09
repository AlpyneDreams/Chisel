// Shared definitions between C++ and HLSL
// - See common.hlsli for cbuffer definitions
// - Make sure your structs are aligned/padded correctly

struct CameraState
{
    float4x4 viewProj;
    float4x4 view;
    float    farZ;
    float3   padding;
};

struct ObjectState
{
    float4x4 model;
};

struct BrushState
{
    uint id;
    float3 padding;
};
