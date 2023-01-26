#pragma once

#include "../CSG/Types.h"

namespace chisel::CSG
{
    class UserdataProvider
    {
    public:
        template <typename T>
        T GetUserdata() const
        {
            return reinterpret_cast<T>(Userdata);
        }
        mutable uint64_t Userdata = 0;
    };
}
