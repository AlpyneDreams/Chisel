#pragma once

#include "entity/Component.h"
#include "math/Math.h"
#include "core/Mesh.h"

namespace engine
{
    struct MeshRenderer : Component
    {
        Mesh* mesh;
    };
}