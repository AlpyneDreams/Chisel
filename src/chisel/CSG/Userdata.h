#pragma once

#include "../CSG/Types.h"

namespace chisel::CSG
{
    struct UserdataProvider
    {
        template <typename T>
        T GetUserdata() const
        {
            return reinterpret_cast<T>(userdata);
        }

        mutable uint64_t userdata = 0;
    };
}
