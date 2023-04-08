#pragma once

#include <list>

#include "core/Mesh.h"
#include "Common.h"
#include "../CSG/Brush.h"
#include "../CSG/CSGTree.h"

#include "Common.h"
#include "Solid.h"

namespace chisel
{
    struct Entity : Atom
    {
        std::string classname;
        std::string targetname;

        // TODO: Variants...
        std::unordered_map<std::string, std::string> kv;
        std::unordered_map<std::string, std::string> connections;

        std::optional<AABB> GetBounds() const override { return std::nullopt; }
        void Transform(const mat4x4& matrix) override { /* Do Nothing */ }
        void AlignToGrid(vec3 gridSize) override { /* Do Nothing */ }
        void SetVolume(Volume volume) override { /* Do Nothing */ }
    };

    struct PointEntity final : Entity
    {
        glm::vec3 origin;

        // TODO: Bounds from model or FGD
        std::optional<AABB> GetBounds() const final override { return AABB(origin - vec3(32), origin + vec3(32)); }
        void Transform(const mat4x4& matrix) final override { origin = matrix * vec4(origin, 1); }
        void AlignToGrid(vec3 gridSize) final override { origin = math::Snap(origin, gridSize); }
        void SetVolume(Volume volume) final override { /* Do Nothing */ }
    };

    struct BrushEntity : Entity
    {
    protected:
        std::list<Solid>    solids;
        CSG::CSGTree        tree;

    public:
        BrushEntity()
        {
            tree.SetVoidVolume(Volumes::Air);
        }

        ~BrushEntity()
        {
            solids.clear();
        }

        Solid& AddBrush(Volume volume = Volumes::Solid)
        {
            return solids.emplace_back(tree.CreateBrush(), volume);
        }

        Solid& AddCube(mat4x4 transform = glm::identity<mat4x4>(), vec3 size = vec3(64.f), Volume volume = Volumes::Solid)
        {
            solids.push_back(CubeBrush(tree.CreateBrush(), volume, size, transform));
            return solids.back();
        }

        void RemoveBrush(const Solid& brush)
        {
            solids.remove(brush);
        }

        virtual void Rebuild(BrushGPUAllocator& a)
        {
            auto rebuilt = tree.Rebuild();
            for (CSG::Brush* brush : rebuilt)
                brush->GetUserdata<Solid*>()->UpdateMesh(a);
        }

        auto begin() { return solids.begin(); }
        auto end() { return solids.end(); }

    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override
        {
            std::optional<AABB> bounds;
            for (const Solid& selectable : solids)
            {
                auto selectedBounds = selectable.GetBounds();
                if (!selectedBounds)
                    continue;

                bounds = bounds
                    ? AABB::Extend(*bounds, *selectedBounds)
                    : *selectedBounds;
            }

            return bounds;
        }

        void Transform(const mat4x4& matrix) final override { for (auto& b : solids) b.Transform(matrix); }
        void AlignToGrid(vec3 gridSize) final override { for (auto& b : solids) b.AlignToGrid(gridSize); }
        void SetVolume(Volume volume) final override { for (auto& b : solids) b.SetVolume(volume); }

    };

    /**
     * Represents the entire map document, world brushes, and entities.
     */
    struct Map final : BrushEntity
    {
        vec3 gridSize = vec3(64.0f);

        // TODO: Polymorphic linked list
        std::vector<Entity*> entities;


        void Rebuild(BrushGPUAllocator& a) final override
        {
            BrushEntity::Rebuild(a);

            for (Entity* ent : entities)
                if (BrushEntity* brush = dynamic_cast<BrushEntity*>(ent))
                    brush->Rebuild(a);
        }

        bool Empty() const {
            return solids.empty() && entities.empty();
        }

        void Clear()
        {
            solids.clear();
            for (Entity* ent : entities)
                delete ent;
            entities.clear();
        }

        ~Map() { Clear(); }

        PointEntity* AddPointEntity(const char* classname)
        {
            PointEntity* ent = new PointEntity();
            ent->classname = classname;
            entities.push_back(ent);
            return ent;
        }

        Map() : BrushEntity()
        {
            //solids.push_back(CubeBrush(tree.CreateBrush(), Volumes::Solid));
            //solids.push_back(CubeBrush(tree.CreateBrush(), Volumes::Air, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(0.25, 2.0, 0.25))));
            //solids.push_back(CubeBrush(tree.CreateBrush(), Volumes::Air, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(2.0, 0.25f, 0.25))));
            //solids.push_back(CubeBrush(tree.CreateBrush(), Volumes::Solid, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(2.0, 0.10f, 0.10))));

            // Per-frame test
            /*if (tunnel && tunnel2)
            {
                static const CSG::Matrix4 tunnelOrigTransform = tunnel->GetTransform();
                static const CSG::Matrix4 tunnel2OrigTransform = tunnel2->GetTransform();
                float time = 0.0f;//Time::GetTime();
                tunnel->SetTransform(
                    glm::rotate(CSG::Matrix4(1), math::radians(time), CSG::Vector3(0,1,0)) *
                    tunnelOrigTransform
                );
                tunnel2->SetTransform(
                    glm::rotate(CSG::Matrix4(1), math::radians(time), CSG::Vector3(0,1,0)) *
                    tunnel2OrigTransform
                );
            }*/

        }
    };
}