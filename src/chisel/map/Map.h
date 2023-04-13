#pragma once

#include <list>

#include "core/Mesh.h"
#include "Common.h"
#include "Solid.h"

namespace chisel
{
    struct Entity : Atom
    {
        std::string classname;
        std::string targetname;

        glm::vec3 origin;

        // TODO: Variants...
        std::unordered_map<std::string, std::string> kv;
        std::unordered_map<std::string, std::string> connections;

        std::optional<AABB> GetBounds() const override { return std::nullopt; }
        void Transform(const mat4x4& matrix) override { /* Do Nothing */ }
        void AlignToGrid(vec3 gridSize) override { /* Do Nothing */ }

        virtual bool IsBrushEntity() const = 0;
    };

    struct PointEntity final : Entity
    {
        // TODO: Bounds from model or FGD
        std::optional<AABB> GetBounds() const final override { return AABB(origin - vec3(32), origin + vec3(32)); }
        void Transform(const mat4x4& matrix) final override { origin = matrix * vec4(origin, 1); }
        void AlignToGrid(vec3 gridSize) final override { origin = math::Snap(origin, gridSize); }

    // Entity Interface //

        virtual bool IsBrushEntity() const override { return false; }
    };

    struct BrushEntity : Entity
    {
    public:
        BrushEntity()
        {
        }

        ~BrushEntity()
        {
        }

        Solid& AddBrush(std::vector<Side> sides)
        {
            return m_solids.emplace_back(sides);
        }

        Solid& AddCube(mat4x4 transform = glm::identity<mat4x4>(), vec3 size = vec3(64.f))
        {
            return AddBrush(CreateCubeBrush(size, transform));
        }

        void RemoveBrush(const Solid& brush)
        {
            m_solids.remove(brush);
        }

        auto begin() { return m_solids.begin(); }
        auto end()   { return m_solids.end(); }

    // Entity Interface //

        bool IsBrushEntity() const override { return true; }

    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override
        {
            std::optional<AABB> bounds;
            for (const Solid& selectable : m_solids)
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

        void Transform(const mat4x4& matrix) final override { for (auto& b : m_solids) b.Transform(matrix); }
        void AlignToGrid(vec3 gridSize) final override { for (auto& b : m_solids) b.AlignToGrid(gridSize); }
    protected:
        std::list<Solid> m_solids;
    };

    /**
     * Represents the entire map document, world brushes, and entities.
     */
    struct Map final : BrushEntity
    {
        vec3 gridSize = vec3(64.0f);

        // TODO: Polymorphic linked list
        std::vector<Entity*> entities;

        bool Empty() const
        {
            return m_solids.empty() && entities.empty();
        }

        void Clear()
        {
            m_solids.clear();
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

        Map()
            : BrushEntity()
        {
        }
    };
}