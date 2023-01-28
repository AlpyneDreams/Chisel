#pragma once

#include "common/Common.h"
#include "console/Console.h"

namespace chisel
{
    using SelectionID = uint32;

    // TODO: Multiple selections.
    inline struct Selection
    {
    private:
        SelectionID selected = Null;

    public:
        static constexpr SelectionID Null = ~0;

        bool Any() {
            return selected != Null;
        }

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
                Select(Null);
            }
        }

        void Clear() {
            Select(Null);
        }

        void Toggle(auto ent) {
            if (selected == SelectionID(ent)) {
                Select(Null);
            } else {
                Select(ent);
            }
        }
    } Selection;
}