#pragma once
#include "gui/View3D.h"

#include "chisel/Chisel.h"

namespace chisel
{
    /**
     * The main level editor 3D view.
     */
    struct Viewport : public View3D
    {
        Viewport();

        const Tool& activeTool = Chisel.activeTool;

        // TODO: One map per viewport
        Map& map = Chisel.map;
        bool isDraggingCube = false;

        bool  draggingBlock  = false;
        vec3  dragStartPos   = vec3(0.f);

        void OnClick(uint2 mouse);
        void OnResizeGrid(vec3& gridSize);
        void DrawHandles(mat4x4& view, mat4x4& proj) override;
        void OnPostDraw() override;
    };
}
