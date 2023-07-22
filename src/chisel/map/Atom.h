#pragma once

#include "Types.h"
#include "../Selection.h"

namespace chisel
{
    class Atom : public Selectable
    {
    public:
        Atom(BrushEntity* parent)
            : m_parent(parent)
        {
        }

        BrushEntity *GetParent() const
        {
            return m_parent;
        }

    protected:
        BrushEntity* m_parent;
    };

}
