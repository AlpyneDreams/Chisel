#pragma once

#include "math/Math.h"
#include "input/InputMap.h"

namespace chisel
{
    enum class MouseButton
    {
        Left = 0,       // The user's primary mouse button
                        // (usually, but not always, the left button)

        Right = 1,      // The user's secondary mouse button
                        // (usually, but not always, the right button)

        Middle = 2,     // Middle mouse button or scroll-wheel click

        X1 = 3, X2 = 4, // Additional mouse buttons (typically forward/back)

        Max = 4
    };

    /** Interface to mouse input.
     *
     * To control cursor appearence and behavior
     * use chisel::Cursor in platform/Cursor.h
     */
    inline struct Mouse
    {
        using enum MouseButton;

        // True if mouse button is currently held down
        bool GetButton(MouseButton btn) const { return buttons.GetButton(btn); }
        // True if mouse button was depressed this frame
        bool GetButtonDown(MouseButton btn) const { return buttons.GetButtonDown(btn); }
        // True if mouse button was released this frame
        bool GetButtonUp(MouseButton btn) const { return buttons.GetButtonUp(btn); }

        // Relative (delta) motion this frame
        int2 GetMotion() const { return motion; }

        void SetButton(MouseButton btn, bool down) { buttons.SetButton(btn, down); }
        void SetMotion(int2 delta) { motion = delta; }

        void Update()
        {
            buttons.Update();

            // Clear mouse delta
            motion = int2(0, 0);
        }

    private:
        PressMap<MouseButton> buttons;
        int2 motion;

    } Mouse;

    // TODO: Gamepad(s)
}