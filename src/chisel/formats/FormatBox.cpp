#pragma once

#include "../map/Solid.h"
#include "../map/Map.h"
#include "../Chisel.h"

#include "../submodules/yyjson/src/yyjson.h"

namespace chisel
{
    static yyjson_mut_val* VectorToYYJson(yyjson_mut_doc* doc, const vec2& vec)
    {
        yyjson_mut_val* vec_arr = yyjson_mut_arr(doc);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[0]);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[1]);
        return vec_arr;
    }

    static yyjson_mut_val* VectorToYYJson(yyjson_mut_doc* doc, const vec3& vec)
    {
        yyjson_mut_val* vec_arr = yyjson_mut_arr(doc);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[0]);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[1]);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[2]);
        return vec_arr;
    }

    static yyjson_mut_val* VectorToYYJson(yyjson_mut_doc* doc, const vec4& vec)
    {
        yyjson_mut_val* vec_arr = yyjson_mut_arr(doc);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[0]);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[1]);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[2]);
        yyjson_mut_arr_add_real(doc, vec_arr, vec[3]);
        return vec_arr;
    }

    static void WriteKVPair(yyjson_mut_doc* doc, yyjson_mut_val* val, const char *key, const kv::KeyValuesVariant& kv)
    {
        switch (kv.GetType())
        {
            case kv::Types::None:
            {
                return;
            }
            case kv::Types::String:
            {
                auto str = (std::string_view)kv;
                yyjson_mut_obj_add_strn(doc, val, key, str.data(), str.length());
                return;
            }
            case kv::Types::Int:
            {
                yyjson_mut_obj_add_int(doc, val, key, (int64_t)kv);
                return;
            }
            case kv::Types::Float:
            {
                yyjson_mut_obj_add_real(doc, val, key, (double)kv);
                return;
            }
            case kv::Types::Ptr:
            {

                Console.Log("wtf ptr!!!!");
                abort();
                return;
            }
            case kv::Types::Vector2:
            {
                yyjson_mut_obj_add_val(doc, val, key, VectorToYYJson(doc, (vec2)kv));
                return;
            }
            case kv::Types::Vector3:
            {
                yyjson_mut_obj_add_val(doc, val, key, VectorToYYJson(doc, (vec3)kv));
                return;
            }
            case kv::Types::Vector4:
            {
                yyjson_mut_obj_add_val(doc, val, key, VectorToYYJson(doc, (vec4)kv));
                return;
            }
            case kv::Types::KeyValues:
            {
                Console.Log("wtf kv!!!!");
                //abort();
                return;
            }
        }
    }

    static void WriteEntityKVPairs(yyjson_mut_doc* doc, yyjson_mut_val* val, const Entity& entity)
    {
        // Write classname
        if (entity.classname.empty())
        {
            yyjson_mut_obj_add_str(doc, val, "classname", "worldspawn");  // TODO: worldspawn doesn't have a classname! should asset on no classname
        }
        else
        {
            yyjson_mut_obj_add_str(doc, val, "classname", entity.classname.c_str());
        }

        // Write targetname
        if (!entity.targetname.empty())
        {
            yyjson_mut_obj_add_str(doc, val, "targetname", entity.targetname.c_str());
        }

        // Write the origin
        {
            kv::KeyValuesVariant variant;
            variant.Set(entity.origin);
            WriteKVPair(doc, val, "origin", variant);
        }

        // Write all keyvalues
        for (const auto& pair : entity.kv)
        {
            WriteKVPair(doc, val, pair.first.c_str(), pair.second);
        }
    }

    static void WriteBrushEntity(yyjson_mut_doc* doc, yyjson_mut_val* val, BrushEntity& entity)
    {
        WriteEntityKVPairs(doc, val, entity);

        yyjson_mut_val* solid_arr = yyjson_mut_arr(doc);
        for (Solid& solid : entity)
        {
            yyjson_mut_val* solid_val = yyjson_mut_arr_add_obj(doc, solid_arr);

            yyjson_mut_val* side_arr = yyjson_mut_arr(doc);
            for (const Side& side : solid.GetSides())
            {
                yyjson_mut_val* side_val = yyjson_mut_arr_add_obj(doc, side_arr);

                const char* materialName = side.material ? (const char*)side.material->GetPath() : "DEFAULT";

                yyjson_mut_obj_add_str(doc, side_val, "material", materialName);

                // Plane
                {
                    yyjson_mut_val* plane_val = yyjson_mut_obj(doc);
                    yyjson_mut_obj_add_val(doc, plane_val, "normal", VectorToYYJson(doc, side.plane.normal));
                    yyjson_mut_obj_add_real(doc, plane_val, "offset", side.plane.offset);
                    yyjson_mut_obj_add_val(doc, side_val, "plane", plane_val);
                }

                // Texture Axes
                {
                    yyjson_mut_val* axis_arr = yyjson_mut_arr(doc);
                    yyjson_mut_arr_append(axis_arr, VectorToYYJson(doc, side.textureAxes[0]));
                    yyjson_mut_arr_append(axis_arr, VectorToYYJson(doc, side.textureAxes[1]));
                    yyjson_mut_obj_add_val(doc, side_val, "texture_axis", axis_arr);
                }

                // Scale
                {
                    yyjson_mut_val* scale_arr = yyjson_mut_arr(doc);
                    yyjson_mut_arr_add_real(doc, scale_arr, side.scale[0]);
                    yyjson_mut_arr_add_real(doc, scale_arr, side.scale[1]);
                    yyjson_mut_obj_add_val(doc, side_val, "scale", scale_arr);
                }

                yyjson_mut_obj_add_real(doc, side_val, "rotate", side.rotate);
                yyjson_mut_obj_add_real(doc, side_val, "lightmap_scale", side.lightmapScale);
                yyjson_mut_obj_add_int(doc, side_val, "smoothing_groups", side.smoothing);
            }
            yyjson_mut_obj_add_val(doc, solid_val, "sides", side_arr);
        }
        yyjson_mut_obj_add_val(doc, val, "solids", solid_arr);
    }

    static void WritePointEntity(yyjson_mut_doc* doc, yyjson_mut_val* val, const PointEntity& entity)
    {
        WriteEntityKVPairs(doc, val, entity);
    }

    static void WriteMap(yyjson_mut_doc* doc, yyjson_mut_val* map_val, Map& map)
    {
        // Write the world first
        WriteBrushEntity(doc, map_val, map);

        // Now write the individual entities
        yyjson_mut_val *ent_arr = yyjson_mut_arr(doc);
        for (Entity* ent : map.entities)
        {
            yyjson_mut_val* ent_val = yyjson_mut_arr_add_obj(doc, ent_arr);
            if (ent->IsBrushEntity())
            {
                WriteBrushEntity(doc, ent_val, static_cast<BrushEntity&>(*ent));
            }
            else
            {
                WritePointEntity(doc, ent_val, static_cast<PointEntity&>(*ent));
            }
        }
        yyjson_mut_obj_add_val(doc, map_val, "entities", ent_arr);
    }

    bool ExportBox(std::string_view filepath, Map& map)
    {
        bool success = false;

        yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val* root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);

        yyjson_mut_val* map_val = yyjson_mut_obj(doc);
        WriteMap(doc, map_val, map);
        yyjson_mut_obj_add_val(doc, root, "world", map_val);

        const char* json = yyjson_mut_write(doc, 0, NULL);
        success = json != nullptr;

        if (success)
        {
            std::ofstream out = std::ofstream(std::string(filepath));
            success = out && out.good();

            if (success)
            {
                out << json;
            }
            free((void *)json);
        }

        yyjson_mut_doc_free(doc);
        return success;
    }
}
