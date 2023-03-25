#pragma once

#include "chisel/Tools.h"

#include "chisel/Chisel.h"
#include "chisel/map/Map.h"

namespace chisel
{
    struct MapRender;

    enum class Tool {
        Select, Translate, Rotate, Scale, Universal, Bounds,
        Entity, Block
    };

    enum class SelectMode {
        Groups, Objects, Solids
    };

    inline class Chisel
    {
    public:
    // Game Data //
        class FGD* fgd;

    // Editing //
        Map map;

        Tool activeTool = Tool::Translate;
        SelectMode selectMode = SelectMode::Groups;

        /*
        uint GetSelectionID(VMF::MapEntity& ent, VMF::Solid& solid)
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
        */

    // File I/O //
        bool LoadVMF(std::string_view path);

    // Systems //
        MapRender* Renderer;

    // GUI //
        GUI::Window* viewport;

    // Chisel Engine Loop //

        void Run();

        ~Chisel();
    } Chisel;
}
