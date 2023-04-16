#include "../Chisel.h"

namespace chisel
{
    // TODO: Do we ever need to keep a unique ID for stuff like solids + faces ourselves?
    // Cubemap brush faces and shit?
    // Good enough for now.
    static uint32_t s_VMFUniqueID = 0;

    static void WriteKVPair(std::ofstream& out, const std::string_view key, const std::string_view value)
    {
        out << "\"" << key << "\"" << ' ' << "\"" << value << "\"\n";
    }

    // Writes all KV pairs in an entity, including classname and targetname
    static void WriteEntityKVPairs(std::ofstream& out, const Entity& entity)
    {
        // Write classname
        if (entity.classname.empty())
        {
            WriteKVPair(out, "classname", "worldspawn"); // TODO: worldspawn doesn't have a classname! should asset on no classname
        }
        else
        {
            WriteKVPair(out, "classname", entity.classname);
        }

        // Write the origin
        char originString[64];
        snprintf(originString, sizeof(originString), "%g %g %g", entity.origin.x, entity.origin.y, entity.origin.z);
        WriteKVPair(out, "origin", originString);

        // Write targetname
        if (!entity.targetname.empty())
        {
            WriteKVPair(out, "targetname", entity.targetname);
        }

        // Write all keyvalues
        for (const auto& pair : entity.kv)
        {
            WriteKVPair(out, pair.first, pair.second);
        }
    }

    static void WritePointEntity(std::ofstream& out, PointEntity& entity)
    {
        WriteEntityKVPairs(out, entity);
    }

    static void WriteBrushEntity(std::ofstream& out, BrushEntity& entity)
    {
        WriteEntityKVPairs(out, entity);

        for (Solid& solid : entity)
        {
            out << "solid\n";
            out << "{\n";

            char tmp[512];
            snprintf(tmp, sizeof(tmp), "%u", s_VMFUniqueID++);
            WriteKVPair(out, "id", tmp);

            for (const Face& face : solid.GetFaces())
            {
                out << "side\n";
                out << "{\n";

                snprintf(tmp, sizeof(tmp), "%u", s_VMFUniqueID++);
                WriteKVPair(out, "id", tmp);

                std::string_view materialName = face.side->material ? (const char*)face.side->material->GetPath() : "DEFAULT";
                if (materialName.starts_with("materials/") || materialName.starts_with("materials\\"))
                    materialName = materialName.substr(10);
                if (materialName.ends_with(".vmt"))
                    materialName = materialName.substr(0, materialName.length() - 4);
                WriteKVPair(out, "material", materialName);

                snprintf(tmp, sizeof(tmp),
                    "(%g %g %g) (%g %g %g) (%g %g %g)",
                    face.points[0].x, face.points[0].y, face.points[0].z,
                    face.points[1].x, face.points[1].y, face.points[1].z,
                    face.points[2].x, face.points[2].y, face.points[2].z);
                WriteKVPair(out, "plane", tmp);

                snprintf(tmp, sizeof(tmp),
                    "[%g %g %g %g] %g",
                    face.side->textureAxes[0].x, face.side->textureAxes[0].y, face.side->textureAxes[0].z, face.side->textureAxes[0].w, face.side->scale[0]);
                WriteKVPair(out, "uaxis", tmp);

                snprintf(tmp, sizeof(tmp),
                    "[%g %g %g %g] %g",
                    face.side->textureAxes[1].x, face.side->textureAxes[1].y, face.side->textureAxes[1].z, face.side->textureAxes[1].w, face.side->scale[1]);
                WriteKVPair(out, "vaxis", tmp);

                snprintf(tmp, sizeof(tmp), "%g", face.side->rotate);
                WriteKVPair(out, "rotation", tmp);
                snprintf(tmp, sizeof(tmp), "%g", face.side->lightmapScale);
                WriteKVPair(out, "lightmapscale", tmp);
                snprintf(tmp, sizeof(tmp), "%u", face.side->smoothing);
                WriteKVPair(out, "smoothing_groups", tmp);

                out << "}\n";
            }

            out << "}\n";
        }
    }

    static void WriteMap(std::ofstream& out, Map& map)
    {
        out << "world\n";
        out << "{\n";

        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%u", s_VMFUniqueID++);
        WriteKVPair(out, "id", tmp);

        WriteBrushEntity(out, map);

        out << "}\n";

        // Now write the individual entities
        for (Entity* ent : map.entities)
        {
            out << "entity\n";
            out << "{\n";

            if (ent->IsBrushEntity())
            {
                WriteBrushEntity(out, static_cast<BrushEntity&>(*ent));
            }
            else
            {
                WritePointEntity(out, static_cast<PointEntity&>(*ent));
            }

            out << "}\n";
        }
    }

    bool ExportVMF(std::string_view filepath, Map& map)
    {
        s_VMFUniqueID = 0;

        std::ofstream out = std::ofstream(std::string(filepath));
        if (out.bad())
        {
            return false;
        }

        WriteMap(out, map);
        return true;
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
            float x = stream::ParseSimple<float>(coords[0]);
            float y = stream::ParseSimple<float>(coords[1]);
            float z = stream::ParseSimple<float>(coords[2]);

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
        float x = stream::ParseSimple<float>(coords[0]);
        float y = stream::ParseSimple<float>(coords[1]);
        float z = stream::ParseSimple<float>(coords[2]);
        float w = stream::ParseSimple<float>(coords[3]);
        axis = vec4(x, y, z, w);

        auto scale_part = values[1];
        scale_part.remove_prefix(1);
        scale_part = str::trim(scale_part);
        scale = stream::ParseSimple<float>(scale_part);
    }

    static bool AddSolid(BrushEntity& map, kv::KeyValues& kvWorld, std::string& matNameScratch)
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

    static bool AddEntity(Map& map, kv::KeyValues& kvEntity, std::string& matNameScratch)
    {
        auto solids = kvEntity.FindAll("solid");
        // Solid can also be the vphysics solid type.
        // Really annoying.
        bool point = solids.first == solids.second || solids.first->second.GetType() != kv::Types::KeyValues;
        Entity* entity = nullptr;
        if (point)
        {
            PointEntity* point = new PointEntity(&map);
            entity = point;
        }
        else
        {
            BrushEntity* brush = new BrushEntity(&map);
            AddSolid(*brush, kvEntity, matNameScratch);
            entity = brush;
        }

        entity->classname = (std::string_view)kvEntity["classname"];
        entity->targetname = (std::string_view)kvEntity["targetname"];
        entity->origin = kvEntity["origin"];
        kvEntity.RemoveAll("origin");
        kvEntity.RemoveAll("classname");
        kvEntity.RemoveAll("targetname");
        kvEntity.RemoveAll("id");
        kvEntity.RemoveAllWithType("editor", kv::Types::KeyValues);
        kvEntity.RemoveAllWithType("solid", kv::Types::KeyValues);
        entity->kv = std::move(kvEntity);
        map.entities.push_back(entity);
        return true;
    }

    bool ImportVMF(std::string_view filepath, Map& map)
    {
        auto text = fs::readTextFile(filepath);
        if (!text)
            return false;

        auto kv = kv::KeyValues::ParseFromUTF8(StringView{ (std::string)*text });
        if (!kv)
            return false;

        auto& world = (*kv)["world"];
        if (world.GetType() != kv::Types::KeyValues)
            return false;

        // Add solids.
        Chisel.brushAllocator->open();
        {
            std::string matname;
            kv::KeyValues& kvWorld = (kv::KeyValues&)world;
            // TODO: Do we want to parse the other "worldspawn" KVs?
            if (!AddSolid(map, kvWorld, matname))
            {
                Chisel.brushAllocator->close();
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
                    Chisel.brushAllocator->close();
                    return false;
                }

                entities.first++;
            }
        }
        Chisel.brushAllocator->close();
        return true;
    }

}
