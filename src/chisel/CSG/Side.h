#pragma once

#include "../CSG/Plane.h"
#include "../CSG/Userdata.h"

namespace chisel::CSG
{
    // A Side is just a Plane with Userdata.
    // A CSG client could use the userdata to store information
    // relating to a texture, UVs, etc.
    struct Side : public UserdataProvider
    {
        Plane plane{};
    };
}
