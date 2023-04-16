
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

#ifdef _WIN32
#include <dwmapi.h>
#endif

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

        void Create(const char* name, uint width, uint height, bool resizable, bool borderless)
        {
#ifdef _WIN32
            // Don't show us as grey'ed out, not responding when loading.
            DisableProcessWindowsGhosting();
#endif

            int x = SDL_WINDOWPOS_CENTERED,
                y = SDL_WINDOWPOS_CENTERED;
            int flags = SDL_WINDOW_SHOWN | (resizable ? SDL_WINDOW_RESIZABLE : 0) | (borderless ? SDL_WINDOW_BORDERLESS : 0);

#if !defined( _WIN32 ) // JOSH: DXVK HACK HACK
            flags |= SDL_WINDOW_VULKAN;
#endif

            // Create window!
            window = SDL_CreateWindow(name, x, y, width, height, flags);

            if (!window) {
                throw std::runtime_error("[SDL] Failed to create window!");
            }

            EnableDarkMode(true);

            // Force window position
            SDL_SetWindowPosition(window, x, y);
        }

        void EnableDarkMode(bool dark)
        {
#ifdef _WIN32
            HMODULE hModule = LoadLibraryA("dwmapi");
            if (hModule)
            {
                auto pDwmSetWindowAttribute = (decltype(DwmSetWindowAttribute)*) GetProcAddress(hModule, "DwmSetWindowAttribute");
                if (hModule)
                {
                    SDL_SysWMinfo wmInfo{};
                    SDL_VERSION(&wmInfo.version);
                    SDL_GetWindowWMInfo(window, &wmInfo);

                    HWND hWnd = wmInfo.info.win.window;

                    // Enable the dark titlebar
                    const BOOL DarkMode = dark ? TRUE : FALSE;
                    HRESULT hr = pDwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &DarkMode, sizeof(DarkMode));
                    if (FAILED(hr))
                    {
                        // 19 is the old value of DWMWA_USE_IMMERSIVE_DARK_MODE
                        hr = pDwmSetWindowAttribute(hWnd, 19, &dark, sizeof(dark));
                    }
                }
                FreeLibrary(hModule);
            }
#endif
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
                    {
                        SDL_Keycode code = e.key.keysym.sym;
                        if (code & SDLK_SCANCODE_MASK)
                            code = (code & ~SDLK_SCANCODE_MASK) + int(Key::Scancode);
                        Keyboard.SetKey(Key(code), e.key.state == SDL_PRESSED);
                        break;
                    }
                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                        Mouse.SetButton(GetMouseButton(e.button.button), e.button.state == SDL_PRESSED);
                        break;
                    case SDL_MOUSEMOTION:
                    {
                        Mouse.AccumMotion(int2(e.motion.xrel, e.motion.yrel));

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
        //void* GetNativeDisplay() { return GetSysWMInfo().info.x11.display; }
        //void* GetNativeWindow() { return (void*)(uintptr_t)GetSysWMInfo().info.x11.window; }
        void* GetNativeDisplay() { return window; }
        void* GetNativeWindow() { return window; }
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
