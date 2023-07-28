#pragma once

#include "Tool.h"
#include "math/Plane.h"

namespace chisel
{
    struct PlacementTool : public Tool
    {
        using Tool::Tool;

        virtual void DrawHandles(Viewport& viewport);

        virtual void OnRayHit(Viewport& viewport) = 0;

        Plane plane;
        vec3 point;
        vec3 normal;
    };

    struct DragTool : public PlacementTool
    {
        using PlacementTool::PlacementTool;
        virtual void OnRayHit(Viewport& viewport);
        virtual void OnFinishDrag(Viewport& viewport) = 0;
    };
}
