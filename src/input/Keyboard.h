#pragma once

#include "input/InputMap.h"

namespace chisel
{
    /**
     * Key codes.
     * - Values 0-127 (ASCII characters) are the same as in SDL_KeyCode.
     * - Values 128+ are SDL_Scancode (USB) values, but offset by 128. An offset
     *   is used to allow all possible codes to be less than 512 for a small EnumSet.
     * TODO: Add every key code
     */
    enum class Key
    {
        Backspace       = '\b',
        Tab             = '\t',
        Enter           = '\r', Return = Enter,
        Escape          = 27,
        Space           = ' ',  Spacebar = Space,

        LeftBracket     = '[',
        RightBracket    = ']',
        Grave           = '`',
        //Zero = '0', One, Two, Three, Four, Five, Six, Seven, Eight, Nine, Minus = '-', Equals = '=',
        A = 'a', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Delete          = 127,

        Scancode        = 128,
        LeftCtrl        = Scancode + 0xE0,
        LeftShift       = Scancode + 0xE1,
        LeftAlt         = Scancode + 0xE2,
        LeftMeta        = Scancode + 0xE3,
        RightCtrl       = Scancode + 0xE4,
        RightShift      = Scancode + 0xE5,
        RightAlt        = Scancode + 0xE6,
        RightMeta       = Scancode + 0xE7,
        Max             = 512
    };

    inline struct Keyboard
    {
        // True if key is currently held down
        bool GetKey(Key key) const { return keys.GetButton(key); }
        // True if key was depressed this frame
        bool GetKeyDown(Key key) const { return keys.GetButtonDown(key); }
        // True if key was released this frame
        bool GetKeyUp(Key key) const { return keys.GetButtonUp(key); }

        // True if any of the keys are currently held down
        bool AnyKey(auto... keys) const { return (GetKey(keys) || ...); }

        // Modifier keys
        bool ctrl  = false;
        bool shift = false;
        bool alt   = false;
        bool meta  = false;

        // Called by Window::PreUpdate
        void SetKey(Key key, bool down) { keys.SetButton(key, down); }

        void Update()
        {
            // Keyups and keydowns only persist for one frame.
            keys.Update();
            ctrl  = AnyKey(Key::LeftCtrl, Key::RightCtrl);
            shift = AnyKey(Key::LeftShift, Key::RightShift);
            alt   = AnyKey(Key::LeftAlt, Key::RightAlt);
            meta  = AnyKey(Key::LeftMeta, Key::RightMeta);
        }
    private:
        PressMap<Key> keys;

    } Keyboard;
}