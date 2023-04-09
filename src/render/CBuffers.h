#pragma once

#include "math/Math.h"

namespace chisel::cbuffers
{
    struct CameraState
    {
        mat4x4 viewProj;
    };

    struct ObjectState
    {
        mat4x4 modelViewProj;
        mat4x4 modelView;
    };

}
