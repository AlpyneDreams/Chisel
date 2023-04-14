#pragma once

#include "chisel/Enums.h"
#include "math/Math.h"

namespace chisel::GUI
{
    void ToolPropertiesWindow(Tool tool, Rect viewport, uint instance = 0);
    void ToolProperties(Tool tool);
}
