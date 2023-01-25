#pragma once

#include "engine/Engine.h"
#include "common/Common.h"
#include "math/Math.h"
#include "render/Render.h"
#include "imgui/Window.h"
#include "entity/components/Camera.h"
#include "entity/components/Transform.h"
#include "render/RenderContext.h"

namespace engine::editor
{
    /**
     * Tools manages the editor viewport and general rendering.
     */
    inline class Tools
    {
    
    public:
    // Viewport //
        struct EditorCamera {
            Camera camera;
            Transform transform;
        } editorCamera;
    
        RenderSystem& Renderer = Engine.renderSystem;
        
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
        static void DrawSelectionPass(render::RenderContext& ctx);

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
        render::Render& r = Engine.Render;

    } Tools;
    
    inline void Tools::PreDrawSelection(render::Render& r, uint id)
    {
        r.SetShader(editor::Tools.sh_Color);
        float f = std::bit_cast<float>(id + 1); // add 1 as 0 is for background
        r.SetUniform("u_color", vec4(f, 0.f, 0.f, 1.0f));
    }
}