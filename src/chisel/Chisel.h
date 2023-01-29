#pragma once

#include "chisel/Tools.h"

#include "chisel/Chisel.h"
#include "chisel/VMF/VMF.h"

namespace chisel
{
    struct MapRender;

    enum class SelectMode {
        Groups, Objects, Solids
    };

    inline class Chisel
    {
    public:
    // Editing //
        SelectMode selectMode = SelectMode::Groups;

        uint GetSelectionID(MapEntity& ent, Solid& solid)
        {
            switch (selectMode)
            {
                case SelectMode::Groups:
                    return ent.editor.groupid != 0 ? ent.editor.groupid : (ent.id == 0 ? solid.id : ent.id);
                case SelectMode::Objects:
                    return ent.id != 0 ? ent.id : solid.id;
                case SelectMode::Solids: default:
                    return solid.id;
            }
        }

    // Systems //
        MapRender* Renderer;

    // GUI //
        GUI::Window* viewport;

    // Chisel Engine Loop //

        void Run();

    } Chisel;
}
