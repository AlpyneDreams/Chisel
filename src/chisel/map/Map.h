#pragma once

#include <list>

#include "core/Mesh.h"
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

        std::optional<AABB> GetBounds() const final override { return AABB(origin, origin); }
        void Transform(const mat4x4& matrix) final override { origin = matrix * vec4(origin, 0); }
        void AlignToGrid(vec3 gridSize) final override { origin = Snap(origin, gridSize); }
        void SetVolume(Volume volume) final override { /* Do Nothing */ }
    };

    struct BrushEntity : Entity
    {
    protected:
        std::list<Solid>    brushes;
        CSG::CSGTree        tree;

    public:
        ~BrushEntity()
        {
            brushes.clear();
        }

        Solid& AddBrush(Volume volume = Volumes::Solid)
        {
            return brushes.emplace_back(tree.CreateBrush(), volume);
        }

        Solid& AddCube(mat4x4 transform = glm::identity<mat4x4>(), Volume volume = Volumes::Solid)
        {
            brushes.push_back(CubeBrush(tree.CreateBrush(), volume, transform));
            return brushes.back();
        }

        void RemoveBrush(const Solid& brush)
        {
            brushes.remove(brush);
        }

        void Rebuild()
        {
            auto rebuilt = tree.Rebuild();
            for (CSG::Brush* brush : rebuilt)
                brush->GetUserdata<Solid*>()->UpdateMesh();
        }

        auto begin() { return brushes.begin(); }
        auto end() { return brushes.end(); }

    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override
        {
            std::optional<AABB> bounds;
            for (const Solid& selectable : brushes)
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

        void Transform(const mat4x4& matrix) final override { for (auto& b : brushes) b.Transform(matrix); }
        void AlignToGrid(vec3 gridSize) final override { for (auto& b : brushes) b.AlignToGrid(gridSize); }
        void SetVolume(Volume volume) final override { for (auto& b : brushes) b.SetVolume(volume); }

    };

    /**
     * Represents the entire map document, world brushes, and entities.
     */
    struct Map final : BrushEntity
    {
        vec3 gridSize = vec3(64.0f);

        // TODO: Polymorphic linked list
        std::vector<Entity*> entities;

        Map()
        {
            tree.SetVoidVolume(Volumes::Air);

            //brushes.push_back(CubeBrush(tree.CreateBrush(), Volumes::Solid));
            //brushes.push_back(CubeBrush(tree.CreateBrush(), Volumes::Air, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(0.25, 2.0, 0.25))));
            //brushes.push_back(CubeBrush(tree.CreateBrush(), Volumes::Air, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(2.0, 0.25f, 0.25))));
            //brushes.push_back(CubeBrush(tree.CreateBrush(), Volumes::Solid, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(2.0, 0.10f, 0.10))));

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