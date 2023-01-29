#pragma once

#include "common/Event.h"
#include "platform/Window.h"
#include "render/Render.h"
#include "render/RenderContext.h"
#include "gui/Common.h"

#include "core/Camera.h"
#include "common/System.h"

namespace chisel
{
    // Manages remdering device and callbacks
    struct RenderSystem : public System
    {
        Window* window;
        render::Render* render = render::Render::Create();

        Event<render::Render&> OnBeginFrame;
        Event<render::Render&> OnEndFrame;
        Event<render::RenderContext&> OnBeginCamera;
        Event<render::RenderContext&> OnEndCamera;

        RenderSystem(Window* win) : window(win) {}

        void Start()
        {
            window->Create("Chisel", 1920, 1080, true, false);
            render->Init(window);
            window->OnAttach();
        }

        void Update()
        {
            using namespace render;
            GUI::Update();

            Render& r = *render;

            r.BeginFrame();
            OnBeginFrame(r);
            
            OnEndFrame(r);
            r.EndFrame();

            GUI::Render();
        }

        void DrawCamera(Camera& camera)
        {
            using namespace render;
            RenderContext ctx = RenderContext(*render, camera);

            OnBeginCamera(ctx);
            
            OnEndCamera(ctx);
        }

        void Shutdown()
        {
            window->OnDetach();
            render->Shutdown();
        }

        ~RenderSystem() {
            delete render;
        }

    };
}