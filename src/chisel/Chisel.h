#pragma once

#include "chisel/Enums.h"
#include "chisel/Tools.h"

#include "chisel/Chisel.h"
#include "chisel/map/Map.h"

namespace chisel
{
    struct MapRender;

    inline class Chisel
    {
    public:
    // Game Data //
        class FGD* fgd;

    // Editing //
        // TODO: Multiple maps.
        Map map;

        Tool        activeTool = Tool::Translate;
        SelectMode  selectMode = SelectMode::Groups;

        // TODO: Store tool properties per-mapdoc
        struct EntTool {
            std::string className = "info_player_start";
            bool        randomYaw  = "info_player_start";
        } entTool;

        struct BlockTool {
            PrimitiveType type = PrimitiveType::Block;
        } blockTool;
        

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
        void Save() {} // TODO
        void CloseMap();
        bool LoadVMF(std::string_view path);
        void CreateEntityGallery();

    // Systems //
        MapRender* Renderer;

    // GUI //
        GUI::Window* viewport;
        GUI::Window* mainAssetPicker;

    // Chisel Engine Loop //

        void Run();

        ~Chisel();
    } Chisel;
}
