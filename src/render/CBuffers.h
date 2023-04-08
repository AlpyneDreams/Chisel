#pragma once

#include "math/Math.h"

namespace chisel::cbuffers
{
    // See shaders/common.hlsli

    struct Scene
    {
        float farZ;
        float padding1, padding2, padding3;
    };

    struct Object
    {
        mat4x4 modelViewProj;
        mat4x4 modelView;
    };
}
