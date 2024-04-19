#include "../Chisel.h"
#include "../FGD/FGD.h"

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

    static void WriteKVPair(std::ofstream& out, const std::string_view key, const char* fmt, auto... args)
    {
        char tmp[1024];
        snprintf(tmp, 1024, fmt, args...);
        out << "\"" << key << "\"" << ' ' << "\"" << tmp << "\"\n";
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
        WriteKVPair(out, "origin", "%g %g %g", entity.origin.x, entity.origin.y, entity.origin.z);

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

        for (Solid& solid : entity.Brushes())
        {
            out << "solid\n";
            out << "{\n";

            WriteKVPair(out, "id", "%u", s_VMFUniqueID++);

            for (const Face& face : solid.GetFaces())
            {
                out << "side\n";
                out << "{\n";

                WriteKVPair(out, "id", "%u", s_VMFUniqueID++);

                std::string_view materialName = face.side->material != nullptr ? (const char*)face.side->material->GetPath() : "DEFAULT";
                if (materialName.starts_with("materials/") || materialName.starts_with("materials\\"))
                    materialName = materialName.substr(10);
                if (materialName.ends_with(".vmt"))
                    materialName = materialName.substr(0, materialName.length() - 4);
                WriteKVPair(out, "material", materialName);

                WriteKVPair(out, "plane",
                    "(%g %g %g) (%g %g %g) (%g %g %g)",
                    face.points[0].x, face.points[0].y, face.points[0].z,
                    face.points[1].x, face.points[1].y, face.points[1].z,
                    face.points[2].x, face.points[2].y, face.points[2].z);

                WriteKVPair(out, "uaxis",
                    "[%g %g %g %g] %g",
                    face.side->textureAxes[0].x, face.side->textureAxes[0].y, face.side->textureAxes[0].z, face.side->textureAxes[0].w, face.side->scale[0]);

                WriteKVPair(out, "vaxis",
                    "[%g %g %g %g] %g",
                    face.side->textureAxes[1].x, face.side->textureAxes[1].y, face.side->textureAxes[1].z, face.side->textureAxes[1].w, face.side->scale[1]);

                WriteKVPair(out, "rotation", "%g", face.side->rotate);
                WriteKVPair(out, "lightmapscale", "%g", face.side->lightmapScale);
                WriteKVPair(out, "smoothing_groups", "%u", face.side->smoothing);
                
                if (face.side->disp.has_value())
                {
                    out << "dispinfo\n";
                    out << "{\n";

                    WriteKVPair(out, "power", "%d", face.side->disp->power);
                    WriteKVPair(out, "startposition", "[%g %g %g]",
                        face.side->disp->startPos.x, face.side->disp->startPos.y, face.side->disp->startPos.z);
                    WriteKVPair(out, "elevation", "%g", face.side->disp->elevation);
                    WriteKVPair(out, "subdiv", "%u", face.side->disp->subdiv);
                    WriteKVPair(out, "flags", "%u", face.side->disp->flags);

                    // TODO: Displacement vertex data

                    out << "}\n";
                }


                out << "}\n";
            }

            out << "}\n";
        }
    }

    static void WriteMap(std::ofstream& out, Map& map)
    {
        out << "world\n";
        out << "{\n";

        WriteKVPair(out, "id", "%u", s_VMFUniqueID++);

        WriteBrushEntity(out, map);

        out << "}\n";

        // Now write the individual entities
        for (Entity* ent : map.Entities())
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

    using DispRow1 = std::vector<float>;
    using DispRow3 = std::vector<vec3>;
    using DispField1 = std::vector<DispRow1>;
    using DispField3 = std::vector<DispRow3>;

    static DispRow1 ParseRow1(std::string_view value)
    {
        DispRow1 row;
        auto numbers = str::split(value, " ");
        for (auto i = 0; i < numbers.size(); i++)
            row.push_back(stream::ParseSimple<float>(numbers[i]));
        return row;
    }

    static DispRow3 ParseRow3(std::string_view value)
    {
        DispRow3 row;
        auto numbers = str::split(value, " ");
        for (auto i = 0; i < numbers.size(); i += 3)
        {
            row.push_back(vec3(
                stream::ParseSimple<float>(numbers[i]),
                stream::ParseSimple<float>(numbers[i + 1]),
                stream::ParseSimple<float>(numbers[i + 2])
            ));
        }
        return row;
    }

    static DispField1 ParseField1(kv::KeyValues& obj)
    {
        DispField1 field;
        field.resize(obj.ChildCount());
        for (auto& [key, value] : obj)
        {
            int i = key[3] - '0';
            field[i] = ParseRow1(value);
        }
        return field;
    }

    static DispField3 ParseField3(kv::KeyValues& obj)
    {
        DispField3 field;
        field.resize(obj.ChildCount());
        for (auto& [key, value] : obj)
        {
            int i = key[3] - '0';
            field[i] = ParseRow3(value);
        }
        return field;
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

                if (kvSide.Contains("dispinfo"))
                {
                    kv::KeyValues& kvDisp = kvSide["dispinfo"];
                    thisSide.disp.emplace(int(kvDisp["power"]));
                    thisSide.disp->startPos = kvDisp["startposition"];
                    thisSide.disp->elevation = kvDisp["elevation"];
                    thisSide.disp->subdiv = kvDisp["subdiv"];
                    thisSide.disp->flags = kvDisp["flags"];

                    DispField3 normals = ParseField3(kvDisp["normals"]);
                    DispField1 distances = ParseField1(kvDisp["distances"]);
                    DispField3 offsets = ParseField3(kvDisp["offsets"]);
                    DispField3 offset_normals = ParseField3(kvDisp["offset_normals"]);
                    DispField1 alphas = ParseField1(kvDisp["alphas"]);
                    // TODO: triangle_tags, allowed_verts

                    for (uint y = 0; y < thisSide.disp->length; y++)
                    {
                        for (uint x = 0; x < thisSide.disp->length; x++)
                        {
                            DispVert vert;
                            vert.normal = normals[y][x];
                            vert.dist = distances[y][x];
                            vert.offset = offsets[y][x];
                            vert.offsetNormal = offset_normals[y][x];
                            vert.alpha = alphas[y][x];
                            (*thisSide.disp)[y][x] = vert;
                        }
                    }
                }

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
        std::string classname = std::string(kvEntity["classname"]);
        // Solid can also be the vphysics solid type.
        // Really annoying.
        bool point = solids.first == solids.second || solids.first->second.GetType() != kv::Types::KeyValues;
        bool prop = Chisel.fgd->classes.contains(classname) && Chisel.fgd->classes[classname].isProp;
        Entity* entity = nullptr;
        if (point && prop)
        {
            ModelEntity* model = new ModelEntity(&map);
            model->model = Assets.Load<Mesh>((std::string)kvEntity["model"]);
            entity = model;
        }
        else if (point)
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
        map.AddEntity(entity);
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

        // TODO: Load cameras...

        return true;
    }

}
