#pragma once

#include <list>

#include "core/Mesh.h"
#include "Common.h"
#include "Solid.h"
#include "formats/KeyValues.h"

namespace chisel
{
    struct RayHit
    {
        const Solid* brush;
        const Face* face;
        float t;
    };

    inline bool PointInsideConvex(const vec3& point, std::span<const vec3> vertices)
    {
        if (vertices.size() < 3)
            return false;

        glm::vec3 v0 = vertices[0];
        glm::vec3 v1 = vertices[1];
        glm::vec3 v2 = vertices[2];
        glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);

        for (size_t i = 0; i < vertices.size(); i++)
        {
            size_t j = (i + 1) % vertices.size();

            glm::vec3 vi = vertices[i];
            glm::vec3 vj = vertices[j];

            static constexpr float epsilon = 0.001f;
            if (glm::dot(normal, glm::cross(vj - vi, point - vi)) < -epsilon)
                return false;
        }

        return true;
    }

    struct Entity : Atom
    {
        Entity(BrushEntity* parent)
            : Atom(parent)
        {
        }

        std::string classname;
        std::string targetname;

        glm::vec3 origin;

        kv::KeyValues kv;

        std::optional<AABB> GetBounds() const override { return std::nullopt; }
        void Transform(const mat4x4& matrix) override { /* Do Nothing */ }
        void AlignToGrid(vec3 gridSize) override { /* Do Nothing */ }

        virtual bool IsBrushEntity() const = 0;

        void Delete() final override;
    };

    struct PointEntity final : Entity
    {
        PointEntity(BrushEntity* parent)
            : Entity(parent)
        {
        }

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
        BrushEntity(BrushEntity* parent)
            : Entity(parent)
        {
        }

        ~BrushEntity()
        {
        }

        Solid& AddBrush(std::vector<Side> sides)
        {
            return m_solids.emplace_back(this, sides);
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
        virtual bool IsMap() { return false; }

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

        std::optional<RayHit> QueryRay(const Ray& ray) const
        {
            std::optional<RayHit> hit;

            for (const auto& brush : m_solids)
            {
                auto bounds = brush.GetBounds();
                if (!bounds)
                    continue;

                if (!ray.Intersects(*bounds))
                    continue;

                for (const auto& face : brush.GetFaces())
                {
                    float t;
                    if (!ray.Intersects(face.side->plane, t))
                        continue;

                    vec3 intersection = ray.GetPoint(t);
                    if (!PointInsideConvex(intersection, face.points))
                        continue;

                    RayHit thisHit =
                    {
                        .brush    = &brush,
                        .face     = &face,
                        .t        = t,
                    };

                    if (!hit || t < hit->t)
                        hit = thisHit;
                }
            }

            return hit;
        }
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

        bool IsMap() final override { return true; }

        ~Map() { Clear(); }

        PointEntity* AddPointEntity(const char* classname)
        {
            PointEntity* ent = new PointEntity(this);
            ent->classname = classname;
            entities.push_back(ent);
            return ent;
        }

        void RemoveEntity(Entity& entity)
        {
            // SUCKS
            entities.erase(std::remove_if(entities.begin(),
                entities.end(),
                [&](Entity* a)-> bool
                { return a == &entity; }),
                entities.end());

            delete &entity;
        }

        Map()
            : BrushEntity(nullptr)
        {
        }
    };

    inline void Entity::Delete()
    {
        assert(m_parent->IsMap());
        static_cast<Map*>(m_parent)->RemoveEntity(*this);
    }
}