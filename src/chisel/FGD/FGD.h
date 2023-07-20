#pragma once

#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include "common/Hash.h"
#include "math/Math.h"
#include "render/Render.h"

namespace chisel
{
    struct Texture;

    class FGD
    {
        template <class T>
        using Dict = std::unordered_map<std::string, T>;

        template <class T>
        using HashMap = std::unordered_map<Hash, T>;

        template <class T>
        using List = std::vector<T>;


        friend struct FGDParser;
    public:
        enum ClassType
        {
            BaseClass, PointClass, SolidClass, NPCClass, KeyframeClass, MoveClass, FilterClass
        };

        enum VarType : Hash
        {
            BadType = 0,
            Angle               = "angle"_hash,
            TargetDest          = "target_destination"_hash,
            TargetNameOrClass   = "target_name_or_class"_hash,
            TargetSrc           = "target_source"_hash,
            Integer             = "integer"_hash,
            String              = "string"_hash,
            StringInstanced,
            Choices             = "choices"_hash,
            Flags               = "flags"_hash,
            Decal               = "decal"_hash,
            Color255            = "color255"_hash,
            Color1              = "color1"_hash,
            StudioModel         = "studio"_hash,
            Sprite              = "sprite"_hash,
            Sound               = "sound"_hash,
            Vector              = "vector"_hash,
            NPCEntityClass      = "npcclass"_hash,
            FilterEntityClass   = "filterclass"_hash,
            Float               = "float"_hash,
            Material            = "material"_hash,
            Scene               = "scene"_hash,
            Side,
            SideList            = "sidelist"_hash,
            Origin              = "origin"_hash,
            VecLine             = "vecline"_hash,
            Axis                = "axis"_hash,
            PointEntityClass    = "pointentityclass"_hash,
            NodeDest            = "node_dest"_hash,
            Script              = "script"_hash,
            ScriptList          = "scriptlist"_hash,
            ParticleSystem      = "particlesystem"_hash,
            InstanceFile        = "instance_file"_hash,
            AngleNegativePitch  = "angle_negative_pitch"_hash,
            InstanceVariable    = "instance_variable"_hash,
            InstanceParm        = "instance_parm"_hash,
            Boolean             = "boolean"_hash,
            NodeID              = "node_id"_hash,
            AngleLocal,
        };

        enum class HelperType : uint32
        {
            Base            = "base"_hash,
            Axis            = "axis"_hash,
            BBox            = "bbox"_hash,
            Color           = "color"_hash,
            Cylinder        = "cylinder"_hash,
            Frustum         = "frustum"_hash,
            HalfGridSnap    = "halfgridsnap"_hash,
            IconSprite      = "iconsprite"_hash,
            LightCone       = "lightcone"_hash,
            LightProp       = "lightprop"_hash,
            Line            = "line"_hash,
            OBB             = "obb"_hash,
            Origin          = "origin"_hash,
            SideList        = "sidelist"_hash,
            Size            = "size"_hash,
            Sphere          = "sphere"_hash,
            Studio          = "studio"_hash,
            StudioProp      = "studioprop"_hash,
            VecLine         = "vecline"_hash,
            WireBox         = "wirebox"_hash,

            Decal               = "decal"_hash,
            Overlay             = "overlay"_hash,
            OverlayTransition   = "overlay_transition"_hash,
            Light               = "light"_hash,
            Sprite              = "sprite"_hash,
            SweptPlayerHull     = "sweptplayerhull"_hash,
            Instance            = "instance"_hash,
            QuadBounds          = "quadbounds"_hash,
            WorldText           = "worldtext"_hash,
        };

        FGD(const char* path);

        struct Base
        {
            std::string name;
            Hash hash;
            std::string description;
        };

        struct Var : Base
        {
            VarType type;
            std::string displayName;
            std::string defaultValue;
            bool report = false;
            bool readOnly = false;
            bool intChoices = false;
            List<std::pair<std::string, std::string>> choices;
        };

        struct InputOutput : Base
        {
            std::string type; // TODO: Enum
        };

        struct Helper : Base
        {
            HelperType type;
            List<std::string> params;
        };

        struct Class : Base
        {
            ClassType type;
            List<Var> variables;
            HashMap<size_t> varMap;
            Dict<InputOutput> inputs;
            Dict<InputOutput> outputs;
            List<Class*> bases;
            List<Helper> helpers;
            Rc<Texture> texture;
            int3 bbox[2] = {int3(-8, -8, -8), int3(8, 8, 8)};
            int3 color = int3(255, 255, 255);

            Class* Get(Hash className)
            {
                if (hash == className)
                    return this;

                for (auto base : bases)
                    if (Class* c = base->Get(className))
                        return c;
                return nullptr;
            }

            void AddBase(Class* base)
            {
                bases.push_back(base);
            }

            const Var* GetVar(Hash varName) const
            {
                if (varMap.contains(varName))
                    return &variables[varMap.at(varName)];
                for (auto base : bases)
                    if (const Var* var = base->GetVar(varName))
                        return var;
                return nullptr;
            }
        };

        std::string path;

        std::map<std::string, Class> classes; // Alphabetical order
        List<std::string> materialExclusion;

        int minSize = -16384;
        int maxSize = 16384;
    };
}
