#include "VMF.h"

#include "KeyValues.h"
#include "console/Console.h"

#include <string_view>

namespace chisel
{
    Editor::Editor(KeyValues& editor) : KeyValues(editor),
        color               (editor["color"]),
        visgroupid          (editor["visgroupid"]),
        groupid             (editor["groupid"]),
        visgroupshown       (editor["visgroupshown"]),
        visgroupautoshown   (editor["visgroupautoshown"]),
        comments            (editor["comments"])
    {}

    MapAtom::MapAtom(KeyValues& atom) : KeyValues(atom),
        id                  (atom["id"])
    {}

    MapClass::MapClass(KeyValues& atom) : MapAtom(atom),
        editor              (atom["editor"])
    {}

    MapEntity::MapEntity(KeyValues& ent) : MapClass(ent),
        classname           (ent["classname"]),
        spawnflags          (ent["spawnflags"]),
        solids              (ent["solid"])
    {}

    World::World(KeyValues& world) : MapEntity(world),
        mapversion          (world["mapversion"]),
        skyname             (world["skyname"])
    {}

    Visgroup::Visgroup(KeyValues& visgroup) : KeyValues(visgroup),
        name                (visgroup["name"]),
        id                  (visgroup["visgroupid"]),
        color               (visgroup["color"]),
        children            (visgroup["visgroup"])
    {}

    VMF::VMF(KeyValues &vmf)
    {
        auto& versioninfo = vmf["versioninfo"];
        editorversion   = versioninfo["editorversion"];
        editorbuild     = versioninfo["editorbuild"];
        mapversion      = versioninfo["mapversion"];
        formatversion   = versioninfo["formatversion"];
        prefab          = versioninfo["prefab"];

        auto& viewsettings = vmf["viewsettings"];
        bSnapToGrid       = viewsettings["bSnapToGrid"];
        bShowGrid         = viewsettings["bShowGrid"];
        bShowLogicalGrid  = viewsettings["bShowLogicalGrid"];
        nGridSpacing      = viewsettings["nGridSpacing"];
        bShow3DGrid       = viewsettings["bShow3DGrid"];

        world       = vmf["world"];

        entities    = vmf["entity"];
        visgroups   = vmf["visgroup"];
    }
}
