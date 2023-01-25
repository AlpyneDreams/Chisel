#pragma once

#include "hammer/Tools.h"

#include "hammer/Hammer.h"
#include "hammer/VMF.h"

namespace engine::hammer
{
    inline class editor::Tools& Tools = editor::Tools;
    
    struct MapRender;
    
    enum class SelectMode {
        Groups, Objects, Solids
    };
        
    inline class Hammer
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
    
    // File I/O //
        // TODO: multiple open maps
        VMF map;
        void Open(const char* path);
    
    // Systems //
        MapRender* Renderer;
        
    // GUI //
        GUI::Window* viewport;

    // Hammer Engine Loop //

        void Run();

    } Hammer;
}
