
#include "chisel/Chisel.h"
#include "chisel/MapRender.h"
#include "common/String.h"
#include "formats/KeyValues.h"
#include "chisel/FGD/FGD.h"

#include "console/Console.h"
#include "gui/ConsoleWindow.h"
#include "gui/AssetPicker.h"
#include "gui/Layout.h"
#include "gui/Inspector.h"
#include "gui/Viewport.h"
#include "gui/Keybinds.h"

#include "common/Filesystem.h"
#include "render/Render.h"

#include <cstring>
#include <vector>

namespace chisel
{
    void Chisel::Run()
    {
        Tools.Init();

        fgd = new FGD("core/test.fgd");

        // Add chisel systems...
        Renderer = &Tools.systems.AddSystem<MapRender>();
        Tools.systems.AddSystem<Keybinds>();
        Tools.systems.AddSystem<Layout>();
        Tools.systems.AddSystem<MainToolbar>();
        Tools.systems.AddSystem<SelectionModeToolbar>();
        Tools.systems.AddSystem<Inspector>();
        mainAssetPicker = &Tools.systems.AddSystem<AssetPicker>();
        viewport        = &Tools.systems.AddSystem<Viewport>();

        Tools.Loop();
        Tools.Shutdown();
    }

    Chisel::~Chisel()
    {
        delete fgd;
    }

    void Chisel::CloseMap()
    {
        Selection.Clear();
        map.Clear();
    }

    static Plane ParsePlane(std::string_view string)
    {
        auto points = str::split(string, ")");

        std::array<vec3, 3> pointTrio{};
        // Parse points
        for (int i = 0; i < 3; i++)
        {
            // TODO: Why can't we remove the '(' with str::trim
            auto xyz = str::trim(points[i]);
            xyz.remove_prefix(1);

            auto coords = str::split(xyz, " ");
            float x = std::stof(std::string(coords[0]));
            float y = std::stof(std::string(coords[1]));
            float z = std::stof(std::string(coords[2]));

            pointTrio[i] = vec3(x, y, z);
        }

        return Plane(pointTrio[0], pointTrio[1], pointTrio[2]);
    }

    static void ParseAxis(std::string_view value, vec4& axis, float& scale)
    {
        auto values = str::split(value, "]");

        auto axis_part = values[0];
        axis_part.remove_prefix(1);
        axis_part = str::trim(axis_part);
        auto coords = str::split(axis_part, " ");
        float x = std::stof(std::string(coords[0]));
        float y = std::stof(std::string(coords[1]));
        float z = std::stof(std::string(coords[2]));
        float w = std::stof(std::string(coords[3]));
        axis = vec4(x, y, z, w);

        auto scale_part = values[1];
        scale_part.remove_prefix(1);
        scale_part = str::trim(scale_part);
        scale = std::stof(std::string(scale_part));
    }

    bool AddSolid(BrushEntity& map, kv::KeyValues& kvWorld, std::string& matNameScratch)
    {
        std::vector<Side> sideData;

        auto solids = kvWorld.FindAll("solid");
        while (solids.first != solids.second)
        {
            auto& solid = solids.first->second;
            if (solid.GetType() != kv::Types::KeyValues)
                return false;

            kv::KeyValues& kvSolid = (kv::KeyValues&)solid;
            auto sides = kvSolid.FindAll("side");
            sideData.clear();
            while (sides.first != sides.second)
            {
                auto& side = sides.first->second;
                if (side.GetType() != kv::Types::KeyValues)
                    return false;

                kv::KeyValues& kvSide = (kv::KeyValues&)side;

                Side thisSide{};
                thisSide.plane = ParsePlane(kvSide["plane"]);
                matNameScratch = "materials/";
                matNameScratch += (std::string_view)kvSide["material"];
                matNameScratch += ".vmt";

                thisSide.material = Assets.Load<Material>(matNameScratch);
                ParseAxis(kvSide["uaxis"], thisSide.textureAxes[0], thisSide.scale[0]);
                ParseAxis(kvSide["vaxis"], thisSide.textureAxes[1], thisSide.scale[1]);
                thisSide.rotate = kvSide["rotate"];
                thisSide.lightmapScale = kvSide["lightmapscale"];
                thisSide.smoothing = kvSide["smoothing_groups"];
                sideData.emplace_back(thisSide);

                sides.first++;
            }

            auto& brush = map.AddBrush(std::move(sideData));
            brush.UpdateMesh();
            sideData.clear();

            solids.first++;
        }
        return true;
    }

    vec3 ParseVector(std::string_view vec)
    {
        if (!vec.empty())
        {
            auto coords = str::split(vec, " ");
            // SUCKS!
            float x = std::stof(std::string(coords[0]));
            float y = std::stof(std::string(coords[1]));
            float z = std::stof(std::string(coords[2]));
            return vec3(x, y, z);
        }
        return vec3(0);
    }

    bool AddEntity(Map& map, kv::KeyValues& kvEntity, std::string& matNameScratch)
    {
        auto solids = kvEntity.FindAll("solid");
        bool point = solids.first == solids.second;
        Entity* entity = nullptr;
        if (point)
        {
            PointEntity* point = new PointEntity();
            point->origin = ParseVector(kvEntity["origin"]);
            entity = point;
        }
        else
        {
            BrushEntity* brush = new BrushEntity();
            AddSolid(*brush, kvEntity, matNameScratch);
            entity = brush;
        }

        entity->classname = (std::string_view)kvEntity["classname"];
        entity->targetname = (std::string_view)kvEntity["targetname"];
        map.entities.push_back(entity);
        return true;
    }
    
    bool Chisel::LoadVMF(std::string_view path)
    {
        auto text = fs::readTextFile(path);
        if (!text)
            return false;
        
        auto kv = kv::KeyValues::ParseFromUTF8(StringView{ (std::string)*text });
        if (!kv)
            return false;

        auto& world = (*kv)["world"];
        if (world.GetType() != kv::Types::KeyValues)
            return false;

        // Add solids.
        brushAllocator->open();
        {
            std::string matname;
            kv::KeyValues& kvWorld = (kv::KeyValues&)world;
            if (!AddSolid(map, kvWorld, matname))
            {
                brushAllocator->close();
                return false;
            }

            auto entities = kv->FindAll("entity");
            while (entities.first != entities.second)
            {
                auto& entity = entities.first->second;
                if (entity.GetType() != kv::Types::KeyValues)
                    return false;

                kv::KeyValues& kvEntity = (kv::KeyValues&)entity;
                if (!AddEntity(map, kvEntity, matname))
                {
                    brushAllocator->close();
                    return false;
                }

                entities.first++;
            }
        }
        brushAllocator->close();

        return true;
    }

    void Chisel::CreateEntityGallery()
    {
        PointEntity* obsolete = map.AddPointEntity("obsolete");
        obsolete->origin = vec3(-8, -8, 0);

        vec3 origin = vec3(-7, -8, 0);
        for (auto& [name, cls] : fgd->classes)
        {
            if (cls.type == FGD::SolidClass || cls.type == FGD::BaseClass)
                continue;

            PointEntity* ent = map.AddPointEntity(name.c_str());
            ent->origin = origin * 128.f;
            if (++origin.x > 8)
            {
                origin.x = -8;
                origin.y++;
            }
        }
    }
}

namespace chisel::commands
{
    static ConCommand quit("quit", "Quit the application", []() {
        Tools.Shutdown();
        exit(0);
    });
    
    static ConCommand open_vmf("open_vmf", "Load a VMF from a file path.", [](ConCmd& cmd)
    {
        if (cmd.argc != 1)
            return Console.Error("Usage: open_vmf <path>");

        if (!Chisel.LoadVMF(cmd.argv[0]))
            Console.Error("Failed to load VMF '{}'", cmd.argv[0]);
    });
}

int main(int argc, char* argv[])
{
    using namespace chisel;
    Chisel.Run();
}
