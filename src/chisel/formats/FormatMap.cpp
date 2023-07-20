#include "../Chisel.h"

namespace chisel
{
    // Writes a single KV pair
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

    // Point entity
    static void WritePointEntity(std::ofstream& out, const PointEntity& entity)
    {
        WriteEntityKVPairs(out, entity);
    }

    // Brush entity
    static void WriteBrushEntity(std::ofstream& out, BrushEntity& entity)
    {
        WriteEntityKVPairs(out, entity);

        for (Solid& solid : entity)
        {
            out << "{\n";

            for (const Face& face : solid.GetFaces())
            {
                assert(face.points.size() >= 3);

                std::string_view materialName = face.side->material != nullptr ? (const char*)face.side->material->GetPath() : "DEFAULT";
                if (materialName.starts_with("materials/") || materialName.starts_with("materials\\"))
                    materialName = materialName.substr(10);
                if (materialName.ends_with(".vmt"))
                    materialName = materialName.substr(0, materialName.length() - 4);

                char str[512];
                snprintf(str, sizeof(str),
                    "( %g %g %g ) ( %g %g %g ) ( %g %g %g ) %.*s [ %g %g %g %g ] [ %g %g %g %g ] %g %g %g \n",
                    face.points[0].x, face.points[0].y, face.points[0].z,
                    face.points[1].x, face.points[1].y, face.points[1].z,
                    face.points[2].x, face.points[2].y, face.points[2].z,
                    int(materialName.length()), materialName.data(),
                    face.side->textureAxes[0].x, face.side->textureAxes[0].y, face.side->textureAxes[0].z, face.side->textureAxes[0].w,
                    face.side->textureAxes[1].x, face.side->textureAxes[1].y, face.side->textureAxes[1].z, face.side->textureAxes[1].w,
                    face.side->rotate,
                    face.side->scale[0],
                    face.side->scale[1]
                );

                out << str;
            }

            out << "}\n";
        }
    }

    static void WriteMap(std::ofstream& out, Map& map)
    {
        // Write the world first
        out << "{\n";
        WriteBrushEntity(out, map);
        out << "}\n";

        // Now write the individual entities
        for (Entity* ent : map.entities)
        {
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

    bool ExportMap(std::string_view filepath, Map& map)
    {
        std::ofstream out = std::ofstream(std::string(filepath));
        if (out.bad())
        {
            return false;
        }

        WriteMap(out, map);
        return true;
    }

}
