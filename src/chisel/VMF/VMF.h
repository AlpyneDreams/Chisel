#pragma once

#include "math/Math.h"
#include "math/Color.h"

#include "core/Mesh.h"

#include "formats/KeyValues.h"

#include <vector>
#include <array>

namespace chisel { struct Map; }

namespace chisel::VMF
{
    struct Editor
    {
        Color255 color;
        int visgroupid;
        int groupid;
        bool visgroupshown;
        bool visgroupautoshown;
        std::string comments;
        // logicalpos

        Editor() = default;
        Editor(kv::KeyValues& editor);
    };

    struct MapAtom : kv::KeyValues
    {
        int id;

        MapAtom() = default;
        MapAtom(kv::KeyValues& atom);
    };

    struct MapClass : MapAtom
    {
        Editor editor;

        MapClass() = default;
        MapClass(kv::KeyValues& atom);
    };

    struct Plane
    {
        std::array<vec3, 3> point_trio;

        Plane() = default;
        Plane(std::string_view plane);
    };

    struct Side : MapAtom
    {
        Plane plane;
        std::string material;
        vec4 axis[2];
        float scale[2];
        float rotation;
        float lightmapscale;
        uint32 smoothing_groups;
        // dispinfo {}

        Side() = default;
        Side(kv::KeyValues& side);
    };

    struct Solid : MapClass
    {
        std::vector<Side> sides;

        Solid() = default;
        Solid(kv::KeyValues& solid);
    };

    struct MapEntity : MapClass
    {
        std::vector<Solid> solids;
        std::string classname;
        std::string targetname;
        vec3 origin;

        std::unordered_map<std::string, std::string> kv;
        std::unordered_map<std::string, std::string> connections;

        MapEntity() = default;
        MapEntity(kv::KeyValues& ent);
    };

    struct World : MapEntity
    {
        World() = default;
        World(kv::KeyValues& world);
    };

    struct Visgroup
    {
        std::string name;
        int id;
        Color255 color;
        std::vector<Visgroup> children;

        Visgroup() = default;
        Visgroup(kv::KeyValues& visgroup);
    };

    struct VMF
    {
        // versioninfo {}
        int editorversion = 400;
        int editorbuild = 9001;
        int mapversion = 0;
        int formatversion = 100;
        bool prefab = false;

        // viewsettings {}
        bool bSnapToGrid = true;
        bool bShowGrid = true;
        bool bShowLogicalGrid = false;
        int nGridSpacing = 64;
        bool bShow3DGrid = false;

        World world;
        std::vector<MapEntity> entities;
        std::vector<Visgroup> visgroups;
        // hidden {}
        // cameras {}
        // cordon {}

        VMF() = default;
        VMF(kv::KeyValues &vmf);

        void Import(Map& map);
    };
}