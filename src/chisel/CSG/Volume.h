#pragma once

#include "../CSG/Types.h"

namespace chisel::CSG
{
    using VolumeID = uint32_t;
    static constexpr VolumeID DefaultVolume = VolumeID(0);

    using VolumeOperation = std::function<VolumeID(VolumeID)>;

    inline VolumeOperation CreateFillOperation(VolumeID target)
    {
        return [=](VolumeID current) { return target; };
    }

    inline VolumeOperation CreateReplaceOperation(VolumeID from, VolumeID to)
    {
        return [=](VolumeID current) { return (current == from) ? to : current; };
    }
}
