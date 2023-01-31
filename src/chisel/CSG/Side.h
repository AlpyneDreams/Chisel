#pragma once

#include "../CSG/Plane.h"
#include "../CSG/Userdata.h"

namespace chisel::CSG
{
    struct Side : public UserdataProvider
    {
        Plane plane{};
    };
}
