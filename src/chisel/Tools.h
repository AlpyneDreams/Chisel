#pragma once

#include "common/Common.h"
#include "math/Math.h"
#include "render/Render.h"
#include "gui/Window.h"
#include "core/Camera.h"
#include "core/Transform.h"
#include "render/RenderSystem.h"
#include "render/RenderContext.h"

namespace chisel
{
    /**
     * Tools manages the editor viewport and general rendering.
     */
    inline class Tools
    {
    protected:
        Window* window          = Window::CreateWindow();
    public:
        SystemGroup systems;

        RenderSystem Renderer   = RenderSystem(window);
        render::Render& Render  = *Renderer.render;

    public:
    // Viewport //
        struct EditorCamera {
            Camera camera;
            Transform transform;
        } editorCamera;

        render::Shader* sh_Color;
        render::Shader* sh_Grid;

        render::RenderTarget* rt_SceneView;
        render::RenderTarget* rt_ObjectID;

        void ResizeViewport(uint width, uint height)
        {
            rt_SceneView->Resize(width, height);
            rt_ObjectID->Resize(width, height);
        }

        // Read object ID from scene view render target and update selection
        void PickObject(uint2 mouse);

        // Draw object ID to the selection buffer
        static void BeginSelectionPass(render::RenderContext& ctx);
        static void PreDrawSelection(render::Render& r, uint id);

        // Draw wireframe outline of selected object
        void DrawSelectionOutline(Mesh* mesh);
        void DrawSelectionOutline(Mesh* mesh, Transform& transform);

    // GUI //
        GUI::Window* console;

    // Tools Engine Loop //

        void Init();
        void Loop();
        void Shutdown();

    private:
        render::Render& r = Render;

    } Tools;

    inline void Tools::PreDrawSelection(render::Render& r, uint id)
    {
        r.SetShader(chisel::Tools.sh_Color);
        float f = std::bit_cast<float>(id + 1); // add 1 as 0 is for background
        r.SetUniform("u_color", vec4(f, 0.f, 0.f, 1.0f));
    }
}