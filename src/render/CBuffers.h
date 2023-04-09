#pragma once

#include "math/Math.h"

namespace chisel::cbuffers
{
    struct CameraState
    {
        mat4x4 viewProj;
        float farZ;
        float padding[3];
    };

    struct ObjectState
    {
        mat4x4 modelViewProj;
        mat4x4 modelView;
    };

}
