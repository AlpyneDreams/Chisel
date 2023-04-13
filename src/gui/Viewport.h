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

        render::RenderTarget rt_SceneView;
        render::DepthStencil ds_SceneView;
        render::RenderTarget rt_ObjectID;

        void Start() override;
        void OnClick(uint2 mouse) override;
        void OnResize(uint width, uint height) override;
        void OnResizeGrid(vec3& gridSize) override;
        void PresentView() override;
        void DrawHandles(mat4x4& view, mat4x4& proj) override;
        void OnDrawMenuBar() override;
        void OnPostDraw() override;

    // Draw Modes //

        enum class DrawMode {
            Shaded, Depth, ObjectID
        };

        static inline const char* drawModes[] = { "Shaded", "Depth", "Object ID" };

        DrawMode drawMode = DrawMode::Shaded;

        Texture GetTexture(DrawMode mode);
    };
}
