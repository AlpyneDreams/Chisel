#pragma once

#include "../map/Solid.h"
#include "../map/Map.h"
#include "../Chisel.h"

#include "zstd.h"

#include "../submodules/yyjson/src/yyjson.h"

using namespace std::literals;

namespace chisel
{
    ConVar<int> box_compression_level("box_compression_level", 3, "Compression level when saving a box format. 1-9. Default is 3.");

    static vec2 YYJsonToVector2(yyjson_val* vec_val)
    {
        if (!vec_val)
            return vec2(0.0f);
        return vec2(yyjson_get_real(yyjson_arr_get(vec_val, 0)), yyjson_get_real(yyjson_arr_get(vec_val, 1)));
    }

    static vec3 YYJsonToVector3(yyjson_val* vec_val)
    {
        if (!vec_val)
            return vec3(0.0f);
        return vec3(yyjson_get_real(yyjson_arr_get(vec_val, 0)), yyjson_get_real(yyjson_arr_get(vec_val, 1)), yyjson_get_real(yyjson_arr_get(vec_val, 2)));
    }

    static vec4 YYJsonToVector4(yyjson_val* vec_val)
    {
        if (!vec_val)
            return vec4(0.0f);
        return vec4(yyjson_get_real(yyjson_arr_get(vec_val, 0)), yyjson_get_real(yyjson_arr_get(vec_val, 1)), yyjson_get_real(yyjson_arr_get(vec_val, 2)), yyjson_get_real(yyjson_arr_get(vec_val, 3)));
    }

    static Plane ReadPlane(yyjson_val* plane_val)
    {
        yyjson_val* offset = yyjson_obj_get(plane_val, "offset");
        yyjson_val* normal = yyjson_obj_get(plane_val, "normal");

        Plane plane{};
        plane.offset = yyjson_get_real(offset);
        plane.normal = YYJsonToVector3(normal);
        return plane;
    }

    static std::array<vec4, 2> ReadTextureAxis(yyjson_val* axis_val)
    {
        yyjson_val* uaxis = yyjson_arr_get(axis_val, 0);
        yyjson_val* vaxis = yyjson_arr_get(axis_val, 1);

        return std::array<vec4, 2>{ { YYJsonToVector4(uaxis), YYJsonToVector4(vaxis) }};
    }

    static std::array<float, 2> ReadTextureScale(yyjson_val* scale_val)
    {
        yyjson_val* uscale = yyjson_arr_get(scale_val, 0);
        yyjson_val* yscale = yyjson_arr_get(scale_val, 1);

        return std::array<float, 2>{ { (float)yyjson_get_real(uscale), (float)yyjson_get_real(yscale) }};
    }

    static const char* GetStringSafe(yyjson_val* base, const char* name)
    {
        yyjson_val* val = yyjson_obj_get(base, name);
        if (!val)
            return "";
        const char* value = yyjson_get_str(val);
        if (!value)
            return "";
        return value;
    }

    static void AddSolid(BrushEntity& map, yyjson_val* entity_val)
    {
        yyjson_val* solids = yyjson_obj_get(entity_val, "solids");
        size_t solid_idx, solid_max;
        yyjson_val* solid;
        yyjson_arr_foreach(solids, solid_idx, solid_max, solid)
        {
            std::vector<Side> sideData;

            yyjson_val* sides = yyjson_obj_get(solid, "sides");
            size_t side_idx, side_max;
            yyjson_val* side;
            yyjson_arr_foreach(sides, side_idx, side_max, side)
            {
                Side thisSide{};
                thisSide.plane = ReadPlane(yyjson_obj_get(side, "plane"));
                thisSide.material = Assets.Load<Material>(yyjson_get_str(yyjson_obj_get(side, "material")));
                thisSide.textureAxes = ReadTextureAxis(yyjson_obj_get(side, "texture_axis"));
                thisSide.scale = ReadTextureScale(yyjson_obj_get(side, "scale"));
                thisSide.rotate = yyjson_get_real(yyjson_obj_get(side, "rotate"));
                thisSide.lightmapScale = yyjson_get_real(yyjson_obj_get(side, "lightmap_scale"));
                thisSide.smoothing = yyjson_get_int(yyjson_obj_get(side, "smoothing_groups"));

                sideData.emplace_back(thisSide);
            }

            auto& brush = map.AddBrush(std::move(sideData));
            brush.UpdateMesh();
            sideData.clear();
        }
    }

    static void AddEntity(Map& map, yyjson_val* entity_val)
    {
        yyjson_val* solids = yyjson_obj_get(entity_val, "solids");
        bool point = solids == nullptr;
        Entity* entity = nullptr;
        if (point)
        {
            PointEntity* point = new PointEntity(&map);
            entity = point;
        }
        else
        {
            BrushEntity* brush = new BrushEntity(&map);
            AddSolid(*brush, entity_val);
            entity = brush;
        }

        entity->classname = GetStringSafe(entity_val, "classname");
        entity->targetname = GetStringSafe(entity_val, "targetname");
        entity->origin = YYJsonToVector3(yyjson_obj_get(entity_val, "origin"));

        yyjson_val* properties_val = yyjson_obj_get(entity_val, "properties");
        if (properties_val)
        {
            size_t idx, max;
            yyjson_val* key, * val;
            yyjson_obj_foreach(properties_val, idx, max, key, val)
            {
                std::string_view key_name = yyjson_get_str(key);
                if (yyjson_is_str(val))
                {
                    entity->kv.CreateTypedChild(key_name, (std::string_view)yyjson_get_str(val));
                }
                else if (yyjson_is_real(val))
                {
                    entity->kv.CreateTypedChild(key_name, yyjson_get_real(val));
                }
                else if (yyjson_is_int(val))
                {
                    entity->kv.CreateTypedChild(key_name, (int64_t)yyjson_get_int(val));
                }
                else if (yyjson_is_arr(val))
                {
                    switch (yyjson_arr_size(val))
                    {
                    case 1:
                        entity->kv.CreateTypedChild(key_name, yyjson_get_real(yyjson_arr_get(val, 0)));
                        break;
                    case 2:
                        entity->kv.CreateTypedChild(key_name, YYJsonToVector2(val));
                        break;
                    case 3:
                        entity->kv.CreateTypedChild(key_name, YYJsonToVector3(val));
                        break;
                    case 4:
                        entity->kv.CreateTypedChild(key_name, YYJsonToVector4(val));
                        break;
                    default:
                        abort();
                        break;
                    }
                }
                else
                {
                    abort();
                }
            }
        }

        map.entities.push_back(entity);
    }

    bool ImportBox(std::string_view filepath, Map& map)
    {
        auto file = fs::readFile(filepath);
        if (!file)
            return false;

        std::unique_ptr<uint8_t[]> raw_data;
        unsigned long long raw_size = ZSTD_getFrameContentSize(file->data(), file->size());
        assert(raw_size != ZSTD_CONTENTSIZE_UNKNOWN);
        if (raw_size != ZSTD_CONTENTSIZE_ERROR)
        {
            raw_data = std::make_unique<uint8_t[]>(size_t(raw_size));
            size_t size = ZSTD_decompress(raw_data.get(), size_t(raw_size), file->data(), file->size());
            if (size != raw_size)
                return false;
        }

        const char* json = raw_data ? (const char*)raw_data.get() : (const char *) file->data();
        size_t json_size = raw_data ? raw_size : file->size();

        Chisel.brushAllocator->open();

        yyjson_doc* doc = yyjson_read(json, json_size, 0);

        yyjson_val* root = yyjson_doc_get_root(doc);
        yyjson_val* world = yyjson_obj_get(root, "world");
        AddSolid(map, world);

        yyjson_val* entities = yyjson_obj_get(world, "entities");
        size_t entity_idx, entity_max;
        yyjson_val* entity;
        yyjson_arr_foreach(entities, entity_idx, entity_max, entity)
        {
            AddEntity(map, entity);
        }

        Chisel.brushAllocator->close();

        yyjson_doc_free(doc);
        return true;
    }

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

        yyjson_mut_val* kv_val = yyjson_mut_obj(doc);
        // Write all keyvalues
        for (const auto& pair : entity.kv)
        {
            WriteKVPair(doc, kv_val, pair.first.c_str(), pair.second);
        }
        yyjson_mut_obj_add_val(doc, val, "properties", kv_val);
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
        std::string path_string = std::string(filepath);
        FILE* file = fopen(path_string.c_str(), "wb");
        if (!file)
        {
            return false;
        }

        bool success = false;

        yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val* root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);

        yyjson_mut_val* map_val = yyjson_mut_obj(doc);
        WriteMap(doc, map_val, map);
        yyjson_mut_obj_add_val(doc, root, "world", map_val);

        size_t len = 0;
        const char* json = yyjson_mut_write(doc, 0, &len);
        success = json != nullptr;

        if (success)
        {
            size_t bound = std::max<size_t>(ZSTD_compressBound(len) * 2, 128 * 1024 * 1024);
            auto buffer = std::make_unique<uint8_t[]>(bound);

            if (box_compression_level != 0)
            {
                size_t compressed_size = ZSTD_compress(buffer.get(), bound, json, len, box_compression_level);
                free((void*)json);
                fwrite(buffer.get(), 1, compressed_size, file);
            }
            else
            {
                fwrite(buffer.get(), 1, len, file);
                free((void*)json);
            }
        }

        yyjson_mut_doc_free(doc);
        fclose(file);
        return success;
    }
}
