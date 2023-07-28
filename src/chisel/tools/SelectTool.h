#pragma once

#include "Tool.h"

namespace chisel
{
    struct SelectTool : public Tool
    {
        using Tool::Tool;

        virtual void OnClick(Viewport& viewport, uint2 mouse);
    };
}
