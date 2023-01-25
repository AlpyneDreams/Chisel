#pragma once

#include "platform/Window.h"

#include "Time.h"
#include "System.h"
#include "render/RenderSystem.h"

#include "console/Console.h"
#include "imgui/ConsoleWindow.h"
#include "console/ConsoleCommands.h"


namespace engine
{
    // Forward declarations.
    namespace editor { class Tools; }
    
    inline class Engine
    {
        friend class editor::Tools;
    protected:
        Window* window      = Window::CreateWindow();
        RenderSystem renderSystem = RenderSystem(window);

    public:
        SystemGroup systems;

        render::Render& Render = *renderSystem.render;

        void Run()
        {
            systems.AddSystem<GUI::ConsoleWindow>();
            
            Init();
            Loop();
            Shutdown();
        }

        // Immediately termintes the application cleanly
        void Quit()
        {
            Shutdown();
            exit(0);
        }

    protected:
        void Init()
        {
            // (Load app info and configs)
            renderSystem.Start();
            // (Load important resources)
            // (Load main scenes)
        }

        bool ShouldQuit()
        {
            return window->ShouldClose(); 
        }

        void Loop()
        {
            systems.Start();

            Time::Seconds lastTime    = Time::GetTime();
            Time::Seconds accumulator = 0;

            while (!ShouldQuit())
            {
                auto currentTime = Time::GetTime();
                auto deltaTime   = currentTime - lastTime;
                lastTime = currentTime;

                Time.Advance(deltaTime);

                // TODO: Whether we use deltaTime or unscaled.deltaTime affects
                // whether fixed.deltaTime needs to be changed with timeScale.
                // Perhaps the accumulator logic could go into Time.
                accumulator += Time.deltaTime;

                while (accumulator >= Time.fixed.deltaTime)
                {
                    // Perform fixed updates
                    systems.Tick();

                    accumulator     -= Time.fixed.deltaTime;
                    Time.fixed.time += Time.fixed.deltaTime;
                    Time.tickCount++;
                }

                // Amount to lerp between physics steps
                [[maybe_unused]] double alpha = accumulator / Time.fixed.deltaTime;
                
                // Clear buffered input
                Input.Update();

                // Process input
                window->PreUpdate();

                // Perform system updates
                systems.Update();

                // Render
                renderSystem.Update();

                // Present
                window->Update();

                Time.frameCount++;
            }
        }

        void Shutdown()
        {
            renderSystem.Shutdown();
            delete window;
            Window::Shutdown();
        }
    } Engine;

    namespace commands
    {
        inline ConCommand quit("quit", "Quit the application", []() {
            Engine.Quit();
        }); 
    }

}