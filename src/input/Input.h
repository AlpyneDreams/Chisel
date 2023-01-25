#pragma once

#include "input/Mouse.h"
#include "input/Keyboard.h"

namespace chisel
{
    inline struct Input
    {
        // TODO: Actions, axes, etc.

        void Update()
        {
            Keyboard.Update();
            Mouse.Update();
        }
    } Input;
}