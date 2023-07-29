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

        // Casts ray and calls callbacks
        virtual void DrawHandles(Viewport& viewport) override;

        // Called when the raycast hits a point
        virtual void OnMouseOver(Viewport& viewport, vec3 point, vec3 normal) = 0;

        // Called when clicking in world space
        virtual void OnClick(Viewport& viewport, vec3 point, vec3 normal) {}

    protected:
        void SetLocalGrid(vec3 position, vec3 normal) { localGrid = true; gridOffset = position; gridNormal = normal; }
        void ClearLocalGrid() { localGrid = false; }

    private:
        bool localGrid = false;
        vec3 gridOffset = Vectors.Zero;
        vec3 gridNormal = Vectors.Up;

    protected:
        uint traceMethod = 0;
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
