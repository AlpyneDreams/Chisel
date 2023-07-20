#pragma once

#include "common/Common.h"
#include "common/Event.h"
#include "console/ConCommand.h"
#include "math/Math.h"
#include "render/Render.h"
#include "gui/Window.h"
#include "core/Camera.h"
#include "core/Transform.h"
#include "render/Render.h"
#include "core/Mesh.h"

#include <charconv>
#include <type_traits>

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
        render::RenderContext rctx;

        Event<render::RenderContext&> OnEndFrame;

    public:
    // Viewport //

        render::Shader sh_Color;
        render::ComputeShader cs_ObjectID;

        // Read object ID from given selection buffer render target and update selection
        void PickObject(uint2 mouse, const Rc<render::RenderTarget>& rt_ObjectID);

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

    } Tools;

#if 0 // FIXME
    inline ConCommand getpos("getpos", "Prints current camera position", [](ConCmd& cmd) {
        Camera& camera = Tools.editorCamera.camera;
        Console.Log("setpos {}; setang {}", camera.position, math::degrees(camera.angles));
    });

    inline ConCommand setpos("setpos", "Sets current camera position", [](ConCmd& cmd) {
        Camera& camera = Tools.editorCamera.camera;
        if (cmd.argc != 3)
        {
            Console.Error("Not enough args.");
            return;
        }

        vec3 position = {};
        for (size_t i = 0; i < 3; i++)
        {
            auto [ptr, err] = std::from_chars(&cmd.argv[i].front(), &cmd.argv[i].back() + 1, position[i]);
            switch (err)
            {
                case std::errc::invalid_argument:
                    Console.Error("Invalid value.");
                    return;
                case std::errc::result_out_of_range:
                    Console.Error("Out of range.");
                    return;
                default:
                    break;
            }
        }
        camera.position = position;
    });
#endif
}