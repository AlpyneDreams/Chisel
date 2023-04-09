// Shared definitions between C++ and HLSL
// - See common.hlsli for cbuffer definitions
// - Make sure your structs are aligned/padded correctly

struct CameraState
{
    float4x4 viewProj;
    float4x4 invView;
    float    farZ;
    float3   padding;
};

struct ObjectState
{
    float4x4 modelViewProj;
    float4x4 modelView;
};
