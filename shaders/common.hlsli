#include "cbuffers.hlsli"

#define USE_CBUFFER(Type, Name, Index) cbuffer Type ## Data : register(b ## Index) { Type Name; }

USE_CBUFFER(CameraState, Camera, 0);
