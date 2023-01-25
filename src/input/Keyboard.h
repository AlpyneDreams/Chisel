#pragma once

#include "console/Console.h"
#include "input/InputMap.h"

namespace chisel
{
    // TODO: All the key codes. Should be basically 1:1 with SDL, maybe with some exceptions?
    enum class Key
    {
        Backspace = '\b', Tab = '\t',
        Return = '\r', Enter = '\r', Escape = 27,
        Spacebar = ' ', Space = ' ',
        Grave = '`', BackQuote = '`', BackTick = '`',
        //Zero = '0', One, Two, Three, Four, Five, Six, Seven, Eight, Nine, Minus = '-', Equals = '=',
        A = 'a', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Delete = 127,
        Max = 512
    };

    inline struct Keyboard
    {
        // True if key is currently held down
        bool GetKey(Key key) const { return keys.GetButton(key); }
        // True if key was depressed this frame
        bool GetKeyDown(Key key) const { return keys.GetButtonDown(key); }
        // True if key was released this frame
        bool GetKeyUp(Key key) const { return keys.GetButtonUp(key); }

        // Called by Window::PreUpdate
        void SetKey(Key key, bool down) { keys.SetButton(key, down); }

        void Update()
        {
            // Keyups and keydowns only persist for one frame.
            keys.Update();
        }
    private:
        PressMap<Key> keys;

    } Keyboard;
}