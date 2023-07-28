#pragma once

#include "Tool.h"

namespace chisel
{
    //--------------------------------------------------
    //  Raycast Placement Tool
    //--------------------------------------------------
    struct PlacementTool : public Tool
    {
        using Tool::Tool;

        // Casts ray and calls OnMouseOver if hit.
        virtual void DrawHandles(Viewport& viewport) override;

        // Called when the raycast hits a point
        virtual void OnMouseOver(Viewport& viewport, vec3 point, vec3 normal) = 0;
    };

    //--------------------------------------------------
    //  Click and Drag Tool
    //--------------------------------------------------
    struct DragTool : public PlacementTool
    {
        using PlacementTool::PlacementTool;

        // Detects dragging and calls OnFinishDrag.
        virtual void OnMouseOver(Viewport& viewport, vec3 point, vec3 normal) override;

        // Called when the user releases dragging (drag start point in viewport.dragStartPos)
        virtual void OnFinishDrag(Viewport& viewport, vec3 point, vec3 normal) = 0;
    };
}
