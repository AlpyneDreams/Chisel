#pragma once

#include "SelectTool.h"
#include "chisel/Enums.h"

namespace chisel
{
    template <TransformType Type>
    struct TransformTool : public SelectTool
    {
        using SelectTool::SelectTool;
        virtual void DrawHandles(Viewport& viewport);
    };
}
