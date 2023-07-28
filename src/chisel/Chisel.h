#pragma once

#include "chisel/Enums.h"
#include "chisel/Engine.h"

#include "chisel/Chisel.h"
#include "chisel/map/Map.h"

namespace chisel
{
    struct MapRender;
    struct Tool;

    inline class Chisel
    {
    public:
    // Game Data //
        class FGD* fgd;

    // Editing //
        // TODO: Multiple maps.
        Map map;

        Tool*       tool;
        Space       transformSpace = Space::World;
        SelectMode  selectMode     = SelectMode::Groups;

        Rc<Material> activeMaterial   = nullptr;

        std::unique_ptr<BrushGPUAllocator> brushAllocator;

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
        bool HasUnsavedChanges() { return !map.Empty(); }
        void Save(std::string_view path);
        void CloseMap();
        bool LoadMap(std::string_view path);
        void CreateEntityGallery();

    // Systems //
        MapRender* Renderer;

    // GUI //
        GUI::Window* console;
        GUI::Window* mainAssetPicker;

    // Chisel Engine Loop //

        void Run();

        ~Chisel();
    } Chisel;
}
