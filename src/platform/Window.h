#pragma once

#include <functional>
#include <utility>
#include <set>

#include "common/Common.h"
#include "math/Math.h"
#include "input/Input.h"

namespace chisel
{
    /** Interface to a window.
     *  Backend could be SDL, for example.
     */
    class Window
    {
    protected:
        Window()
        {
            windows.insert(this);

            // First window created is the main window
            if (main == nullptr)
                main = this;
        }

        std::function<void(uint, uint)> onResize;
    public:
        virtual ~Window()
        {
            windows.erase(this);
            if (main == this)
                main = nullptr;
        }
        static Window* CreateWindow();
        static void Shutdown();

        virtual void Create(const char* name, uint width, uint height, bool resizable, bool borderless) = 0;

        // Run after Render::Init. Typically confgiures ImGui
        virtual void OnAttach() {}
        // Run before Render::Shutdown. Typically detaches ImGui
        virtual void OnDetach() {}

        virtual bool ShouldClose() = 0;
        // Read input and events. Begin frames.
        // Should call Mouse.SetButton, Keyboard.SetKey, etc.
        virtual void PreUpdate() = 0;
        // Present if necessary.
        virtual void Update() { }

        virtual uint2 GetSize() = 0;
        virtual const char* GetTitle() = 0;

        virtual void* GetHandle() = 0;

        virtual void* GetNativeDisplay() = 0;
        virtual void* GetNativeWindow() = 0;

    public:
        void SetResizeCallback(std::function<void(uint, uint)> callback) { onResize = callback; }

        static inline Window* main = nullptr;
        static inline std::set<Window*> windows;
    };
}