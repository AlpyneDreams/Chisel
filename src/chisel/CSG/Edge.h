#pragma once

#include "../CSG/Types.h"

namespace chisel::CSG
{
    struct Edge
    {
        std::array<Face*, 2> faces;
    };
}
