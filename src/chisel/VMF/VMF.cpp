#include "VMF.h"

#include "KeyValues.h"
#include "console/Console.h"

#include <string_view>

namespace chisel::VMF
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
        solids              (ent["solid"]),
        classname           (ent["classname"]),
        targetname          (ent["targetname"])
    {
        std::string origin_str = ent["origin"];
        if (!origin_str.empty())
        {
            auto coords = str::split(origin_str, " ");
            float x = std::stof(std::string(coords[0]));
            float y = std::stof(std::string(coords[1]));
            float z = std::stof(std::string(coords[2]));
            origin = vec3(x, y, z);
        }

        for (const auto& child : ent)
        {
            if (ent.name == "id" ||
                ent.name == "solid" ||
                ent.name == "classname" ||
                ent.name == "targetname")
                continue;

            if (ent.name == "connections")
            {
                for (const auto& connection : ent)
                    connections.emplace(connection.name, connection);
                continue;
            }

            if (child.type == KeyValues::String)
                kv.emplace(child.name, child);
        }
    }

    World::World(KeyValues& world) : MapEntity(world)
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
