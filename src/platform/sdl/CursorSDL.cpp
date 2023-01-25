#include "SDL_events.h"
#include "SDL_mouse.h"
#include "SDL_stdinc.h"
#include "SDL_video.h"
#include "platform/Cursor.h"
#include "platform/Window.h"

#include <SDL.h>
#include <SDL_mouse.h>

namespace chisel
{
    static Cursor::Mode currentMode = Cursor::Normal;

    void Cursor::SetMode(Cursor::Mode mode)
    {
        auto lastMode = currentMode;
        currentMode = mode;

        if (mode == lastMode) return;

        // Update mouse grab (confined mode) for all windows
        for (Window* win : Window::windows)
        {
            SDL_Window* window = (SDL_Window*)win->GetHandle();
            if (!window) continue;
            SDL_SetWindowMouseGrab(window, SDL_bool(mode != Cursor::Normal));
        }

        static int lastX = 0;
        static int lastY = 0;

        if (mode == Cursor::Locked)
        {
            // Record cursor position
            SDL_GetGlobalMouseState(&lastX, &lastY);

            // FIXME: This also hides the mouse cursor which is not ideal.
            // Lock mouse
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }

        // Unlocked
        if (mode != Cursor::Locked && lastMode == Cursor::Locked)
        {
            // Unlock mouse
            SDL_SetRelativeMouseMode(SDL_FALSE);
            // Restore cursor position from before it was locked
            SDL_WarpMouseGlobal(lastX, lastY);
        }
    }

    Cursor::Mode Cursor::GetMode() const {
        return currentMode;
    }
}