#include "cbuffers.hlsli"

cbuffer CBCameraState : register(b0)
{
    CameraState Camera;
};

cbuffer CBObjectState : register(b1)
{
    ObjectState Object;
};
