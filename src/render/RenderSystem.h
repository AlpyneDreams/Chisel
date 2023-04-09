#pragma once

#include "platform/Window.h"
#include "render/Render.h"
#include "gui/Common.h"

#include "core/Camera.h"
#include "common/System.h"

namespace chisel
{
    // Manages remdering device and callbacks
    struct RenderSystem : public System
    {
        Window* window;
        render::RenderContext rctx;

        RenderSystem(Window* win) : window(win) {}

        void Start()
        {
            window->Create("Chisel", 1920, 1080, true, false);
            rctx.Init(window);
            window->OnAttach();
        }

        void Update()
        {
            using namespace render;
            GUI::Update();

            rctx.BeginFrame();
            rctx.EndFrame();

            GUI::Render();
        }

        void Shutdown()
        {
            window->OnDetach();
            rctx.Shutdown();
        }

        ~RenderSystem()
        {
        }

    };
}