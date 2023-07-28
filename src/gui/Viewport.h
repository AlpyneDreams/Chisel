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

        // TODO: One map per viewport
        Map& map = Chisel.map;
        bool isDraggingCube = false;

        bool  draggingBlock  = false;
        vec3  dragStartPos   = vec3(0.f);

        Rc<render::RenderTarget> rt_SceneView;
        Rc<render::DepthStencil> ds_SceneView;
        Rc<render::RenderTarget> rt_ObjectID;

    // Rendering //
        void  Render() override;
        void* GetMainTexture() override;

        void Start() override;
        void OnClick(uint2 mouse) override;
        void OnResize(uint width, uint height) override;
        void DrawHandles(mat4x4& view, mat4x4& proj) override;
        void OnDrawMenu() override;
        void OnPostDraw() override;

    // Draw Modes //

        enum class DrawMode {
            Shaded, Wireframe, Depth, ObjectID
        };

        static inline const char* drawModes[] = { "Shaded", "Wireframe", "Depth", "Object ID" };

        DrawMode drawMode = DrawMode::Shaded;

        Texture* GetTexture(DrawMode mode);
    };
}
