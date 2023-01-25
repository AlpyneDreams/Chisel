#pragma once

#include "common/Common.h"

namespace chisel
{
    using SelectionID = uint32;

    // TODO: Multiple selections.
    inline struct Selection
    {
    private:
        // Should match EntityNull in entity/Common.h
        static constexpr SelectionID Null = ~0;

        SelectionID selected = Null;

    public:
        SelectionID Active() {
            return selected;
        }

        bool Selected(auto ent) {
            return selected == SelectionID(ent);
        }

        bool Empty() {
            return selected == Null;
        }

        void Select(auto ent) {
            selected = SelectionID(ent);
        }

        void Deselect(auto ent) {
            if (selected == SelectionID(ent)) {
                selected = Null;
            }
        }

        void Clear() {
            selected = Null;
        }

        void Toggle(auto ent) {
            if (selected == SelectionID(ent)) {
                selected = Null;
            } else {
                selected = SelectionID(ent);
            }
        }
    } Selection;
}