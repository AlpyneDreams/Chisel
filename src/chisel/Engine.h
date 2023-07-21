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
    inline class Engine
    {
    protected:
        Window* window          = Window::CreateWindow();
    public:
        SystemGroup systems;
        render::RenderContext rctx;

        Event<render::RenderContext&> OnEndFrame;

    public:
    // Viewport //
        render::ComputeShader cs_ObjectID;

        // Read object ID from given selection buffer render target and update selection
        void PickObject(uint2 mouse, const Rc<render::RenderTarget>& rt_ObjectID);

    // Main Engine Loop //

        void Init();
        void Loop();
        void Shutdown();

    private:

    } Engine;
}