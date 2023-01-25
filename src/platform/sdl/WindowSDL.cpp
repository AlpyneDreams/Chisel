
#include <cstdio>
#include <stdexcept>
#include <string>

#include "common/Common.h"
#include "input/Input.h"
#include "platform/Cursor.h"
#include "platform/Platform.h"
#include "platform/Window.h"

#include "gui/impl/imgui_impl_sdl.h"

#include <SDL.h>
#include <SDL_syswm.h>

#if defined(_INC_WINDOWS) and defined(CreateWindow)
    #undef CreateWindow
#endif

namespace chisel
{
    class WindowSDL final : public Window
    {
        friend Window* Window::CreateWindow();
        SDL_Window* window;
        bool shouldClose = false;

    public:
        ~WindowSDL() { SDL_DestroyWindow(window); }

        void Create(const char* name, uint width, uint height, bool resizable)
        {
            int x = SDL_WINDOWPOS_CENTERED,
                y = SDL_WINDOWPOS_CENTERED;
            int flags = SDL_WINDOW_SHOWN | (resizable ? SDL_WINDOW_RESIZABLE : 0);

            // Create window!
            window = SDL_CreateWindow(name, x, y, width, height, flags);

            if (!window) {
                throw std::runtime_error("[SDL] Failed to create window!");
            }

            // Force window position
            SDL_SetWindowPosition(window, x, y);
        }

        void OnAttach() {
            // This should work for all render APIs (Vulkan, OpenGL, D3D, etc.)
            ImGui_ImplSDL2_InitForSDLRenderer(window, NULL);
        }

        void OnDetach() {
            ImGui_ImplSDL2_Shutdown();
        }

        bool ShouldClose() {
            return shouldClose;
        }

        constexpr MouseButton GetMouseButton(byte btn)
        {
            switch (btn)
            {
                case SDL_BUTTON_LEFT:   return Mouse.Left;
                case SDL_BUTTON_RIGHT:  return Mouse.Right;
                case SDL_BUTTON_MIDDLE: return Mouse.Middle;
                case SDL_BUTTON_X1:     return Mouse.X1;
                case SDL_BUTTON_X2:     return Mouse.X2;
                default:                return MouseButton(btn);
            }
        }

        void PreUpdate()
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                switch(e.type)
                {
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                        Keyboard.SetKey(Key(e.key.keysym.sym), e.key.state == SDL_PRESSED);
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                        Mouse.SetButton(GetMouseButton(e.button.button), e.button.state == SDL_PRESSED);
                        break;
                    case SDL_MOUSEMOTION:
                    {
                        Mouse.SetMotion(int2(e.motion.xrel, e.motion.yrel));

                        // Don't send mouse event to ImGui if cursor is locked
                        if (Cursor.GetMode() == Cursor.Locked)
                            continue;

                        break;
                    }

                    case SDL_QUIT:
                        shouldClose = true;
                        break;

                    case SDL_WINDOWEVENT:
                    {
                        const SDL_WindowEvent& wev = e.window;
                        switch (wev.event) {
                            case SDL_WINDOWEVENT_SIZE_CHANGED:
                                break;
                            case SDL_WINDOWEVENT_RESIZED:
                                onResize(wev.data1, wev.data2);
                                break;

                            case SDL_WINDOWEVENT_CLOSE:
                                shouldClose = true;
                                break;
                        }
                        break;
                    }
                    default:
                        break;
                }
                ImGui_ImplSDL2_ProcessEvent(&e);
            }

            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

        #if PLATFORM_X11
            // ImGui on X11: Un-capture mouse.
            // Otherwise debugger will be stuck with no mouse clicks
            // if breaking on a frame with SDL_MOUSEBUTTONDOWN
            if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_CAPTURE)
                SDL_CaptureMouse(SDL_FALSE);
        #endif
        }


        uint2 GetSize()
        {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            return uint2(width, height);
        }

        const char* GetTitle()
        {
            return SDL_GetWindowTitle(window);
        }

        void* GetHandle() { return window; }
    #if PLATFORM_X11
        void* GetNativeDisplay() { return GetSysWMInfo().info.x11.display; }
        void* GetNativeWindow() { return (void*)(uintptr_t)GetSysWMInfo().info.x11.window; }
    #elif PLATFORM_WINDOWS
        void* GetNativeDisplay() { return GetSysWMInfo().info.win.window; }
        void* GetNativeWindow() { return GetSysWMInfo().info.win.window; }
    #endif
    private:
        SDL_SysWMinfo GetSysWMInfo()
        {
            SDL_SysWMinfo wmi;
            SDL_VERSION(&wmi.version);
            if (!SDL_GetWindowWMInfo(window, &wmi)) {
                throw std::runtime_error("[SDL] Failed to get Window system WM info!");
            }
            return wmi;
        }
    };

    Window* Window::CreateWindow()
    {
        // Initialize SDL
        static bool initializedSDL = false;
        if (!initializedSDL)
        {
            int result = SDL_Init(0);
            if (result != 0) {
                throw std::runtime_error(std::string("[SDL] Failed to initialize!", SDL_GetError()));
            }
            initializedSDL = true;
        }

        return new WindowSDL();
    }

    void Window::Shutdown()
    {
        SDL_Quit();
    }
}
