#pragma once

#include "common/Enum.h"

namespace engine
{
    // Represents a set of digital buttons or keys
    template <EnumType E, E Size = E::Max>
    struct PressMap
    {
        // True if button is currently held down
        bool GetButton(E key) const { return keys[key]; }
        // True if button was depressed this frame
        bool GetButtonDown(E key) const { return keysDown[key]; }
        // True if button was released this frame
        bool GetButtonUp(E key) const { return keysUp[key]; }

        void SetButton(E key, bool down)
        {
            // Cannot process this key
            if (key > Size)
                return;

            if (down)
                keysDown[key] = true;
            else
                keysUp[key] = true;

            keys[key] = down;
        }

        void Update()
        {
            // Keyups and keydowns only persist for one frame.
            keysDown.Clear();
            keysUp.Clear();
        }

    private:
        EnumSet<E> keys;
        EnumSet<E> keysDown;
        EnumSet<E> keysUp;

    };
}