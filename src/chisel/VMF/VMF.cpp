#include "VMF.h"

#include "formats/KeyValues.h"
#include "console/Console.h"

#include "chisel/map/Map.h"
#include "assets/Assets.h"

#include <string_view>

namespace chisel::VMF
{
    Editor::Editor(kv::KeyValues& editor)
        : color               (editor["color"])
        , visgroupid          (editor["visgroupid"])
        , groupid             (editor["groupid"])
        , visgroupshown       (editor["visgroupshown"])
        , visgroupautoshown   (editor["visgroupautoshown"])
        , comments            (std::string_view(editor["comments"]))
    {} 

    MapAtom::MapAtom(KeyValues& atom) : KeyValues(atom),
        id                  (atom["id"])
    {}

    MapClass::MapClass(KeyValues& atom) : MapAtom(atom),
        editor              (atom["editor"])
    {}

    MapEntity::MapEntity(KeyValues& ent) : MapClass(ent),
        classname           (std::string_view(ent["classname"])),
        targetname          (std::string_view(ent["targetname"]))
    {
        auto range = ent.FindAll("solid");
        while (range.first != range.second)
        {
            solids.emplace_back(range.first->second);
            range.first++;
        }

        std::string_view origin_str = ent["origin"];
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
            if (child.first == "id" ||
                child.first == "solid" ||
                child.first == "classname" ||
                child.first == "targetname")
                continue;

            if (child.first == "connections")
            {
                //for (const auto& connection : child.second)
                //    connections.emplace(connection.name, connection);
                continue;
            }

            if (child.second.GetType() == kv::Types::String)
                kv.emplace(child.first, (std::string_view)child.second);
        }
    }

    World::World(KeyValues& world) : MapEntity(world)
    {}

    Visgroup::Visgroup(kv::KeyValues& visgroup)
        : name                ((std::string_view)visgroup["name"])
        , id                  (visgroup["visgroupid"])
        , color               (visgroup["color"])
        , children            (visgroup["visgroup"])
    {}

    VMF::VMF(kv::KeyValues &vmf)
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

        world       = World(vmf["world"]);

        {
            auto range = vmf.FindAll("entity");
            while (range.first != range.second)
            {
                entities.emplace_back(range.first->second);
                range.first++;
            }
        }

        {
            auto range = vmf.FindAll("visgroup");
            while (range.first != range.second)
            {
                visgroups.emplace_back(range.first->second);
                range.first++;
            }
        }
    }

    static void AddSolid(BrushEntity& ent, const Solid& solid)
    {
        std::vector<CSG::Side> sides;
        std::vector<SideData> side_data;

        for (uint64 index = 0; const auto& side : solid.sides)
        {
            sides.emplace_back(CSG::Side{ { .userdata = index++ }, CSG::Plane{ side.plane.point_trio[0], side.plane.point_trio[1], side.plane.point_trio[2] } });

            // WIP, good enough for now.
            std::string material_path = "materials/";
            material_path += side.material;
            material_path += ".vmt";

            if (!Assets.IsLoaded(material_path))
                Console.Log("Loading material: {}", material_path);

            SideData data =
            {
                .material      = Assets.Load<Material>(material_path),
                .textureAxes   = {{ side.axis[0],  side.axis[1] }},
                .scale         = {{ side.scale[0], side.scale[1] }},
                .rotate        = side.rotation,
                .lightmapScale = side.lightmapscale,
                .smoothing     = side.smoothing_groups,
            };
            side_data.emplace_back( data );
        }

        chisel::Solid& brush = ent.AddBrush();
        brush.SetSides(&sides.front(), &sides.back() + 1, &side_data.front(), &side_data.back() + 1);
    }

    void VMF::Import(Map& map)
    {
        for (uint64_t i = 0; const auto& solid : world.solids)
        {
            AddSolid(map, solid);
        }

        for (auto& entity : entities)
        {
            Entity* ent = nullptr;

            if (entity.solids.empty())
            {
                PointEntity* point = new PointEntity();
                ent = point;
                point->origin = entity.origin;
            }
            else
            {
                BrushEntity* brush = new BrushEntity();
                ent = brush;

                for (const auto& solid : entity.solids)
                {
                    AddSolid(*brush, solid);
                }
            }

            ent->classname = entity.classname;
            ent->targetname = entity.targetname;
            ent->kv = std::move(entity.kv);
            ent->connections = std::move(entity.connections);
            
            map.entities.push_back(ent);
        }
    }
}
