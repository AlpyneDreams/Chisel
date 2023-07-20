#ifndef COMMON_HLSLI
#define COMMON_HLSLI

#include "cbuffers.hlsli"

#define USE_CBUFFER(Type, Name, Index) cbuffer Type ## Data : register(b ## Index) { Type Name; }

USE_CBUFFER(CameraState, Camera, 0);

// Simple editor lighting
float3 Lighting(float3 normal, float3 view)
{
    float3 light 		= normalize(float3(1.0, 3.0, 2.0));
	float3 color 		= float3(1, 1, 1); // previously u_color
	normal		        = normalize(normal);
	view 		        = normalize(view);

	float NoL		    = saturate(dot(normal, light));

	// Half lambert
	NoL = 0.5 + (0.5 * NoL);
    
    return color * NoL;
}

#endif // COMMON_HLSLI
