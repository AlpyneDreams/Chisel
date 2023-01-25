#pragma once

#include "math/Math.h"
#include "math/Color.h"

#include "core/Mesh.h"

#include "KeyValues.h"

#include <vector>

namespace engine::hammer
{
    struct Editor : KeyValues
    {
        Color color;
        int visgroupid;
        int groupid;
        bool visgroupshown;
        bool visgroupautoshown;
        const char* comments = nullptr;
        // logicalpos

        Editor() = default;
        Editor(KeyValues& editor);
    };

    struct MapAtom : KeyValues
    {
        int id;

        MapAtom() = default;
        MapAtom(KeyValues& atom);
    };

    struct MapClass : MapAtom
    {
        Editor editor;

        MapClass() = default;
        MapClass(KeyValues& atom);
    };

    struct Plane
    {
        // bottom left, top left, top right
        vec3 tri[3];

        Plane() = default;
        Plane(std::string_view plane);
    };

    struct Side : MapAtom
    {
        Plane plane;
        const char* material;
        // uaxis
        // vaxis
        float rotation;
        float lightmapscale;
        float smoothing_groups;
        // dispinfo {}

        Side(KeyValues& side);
    };

    struct Solid : MapClass
    {
        std::vector<Side> sides;

        Mesh mesh;

        Solid(KeyValues& solid);
    };

    struct MapEntity : MapClass
    {
        const char* classname = nullptr;
        int spawnflags = 0;
        // connections {}
        std::vector<Solid> solids;
        // hidden {}

        MapEntity() = default;
        MapEntity(KeyValues& ent);
    };
    
    struct World : MapEntity
    {
        int mapversion = 0;
        const char* skyname = nullptr;

        World() = default;
        World(KeyValues& world);
    };

    struct Visgroup : KeyValues
    {
        const char* name;
        int id;
        Color color;
        std::vector<Visgroup> children;

        Visgroup(KeyValues& visgroup);
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
        VMF(KeyValues &vmf);
    };
}