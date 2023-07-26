#pragma once

#include "Entity.h"
#include "Map.h"
#include "Convex.h"

namespace chisel
{
    /////////////////////
    // Base Entity
    /////////////////////

    Entity::Entity(BrushEntity* parent)
        : Atom(parent)
    {
    }

    void Entity::Delete()
    {
        assert(m_parent->IsMap());
        static_cast<Map*>(m_parent)->RemoveEntity(*this);
    }

    /////////////////////
    // Point Entity
    /////////////////////

    PointEntity::PointEntity(BrushEntity* parent)
        : Entity(parent)
    {
    }

    // TODO: Bounds from model or FGD
    std::optional<AABB> PointEntity::GetBounds() const
    {
        return AABB{origin - vec3(32), origin + vec3(32)};
    }
    void PointEntity::Transform(const mat4x4& matrix)
    {
        origin = matrix * vec4(origin, 1.0f);
    }
    void PointEntity::AlignToGrid(vec3 gridSize)
    {
        origin = math::Snap(origin, gridSize);
    }
    Selectable* PointEntity::Duplicate()
    {
        assert(m_parent->IsMap());
        PointEntity *newEntity = new PointEntity(m_parent);
        newEntity->classname = this->classname;
        newEntity->targetname = this->targetname;
        newEntity->origin = this->origin;
        newEntity->kv = this->kv;
        static_cast<Map*>(m_parent)->AddEntity(newEntity);
        return newEntity;
    }

    /////////////////////
    // Brush Entity
    /////////////////////

    BrushEntity::BrushEntity(BrushEntity* parent)
        : Entity(parent)
    {
    }

    std::optional<AABB> BrushEntity::GetBounds() const
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

    void BrushEntity::Transform(const mat4x4& matrix)
    {
        for (auto& b : m_solids)
            b.Transform(matrix);
    }

    void BrushEntity::AlignToGrid(vec3 gridSize)
    {
        for (auto& b : m_solids)
            b.AlignToGrid(gridSize);
    }

    Selectable* BrushEntity::Duplicate()
    {
        assert(m_parent->IsMap());
        BrushEntity *newEntity = new BrushEntity(m_parent);
        newEntity->classname = this->classname;
        newEntity->targetname = this->targetname;
        newEntity->origin = this->origin;
        newEntity->kv = this->kv;
        for (Solid& brush : Brushes())
            newEntity->AddBrush(brush.GetSides());
        static_cast<Map*>(m_parent)->AddEntity(newEntity);
        return newEntity;
    }

    Solid& BrushEntity::AddBrush(std::vector<Side> sides)
    {
        return m_solids.emplace_back(this, sides);
    }

    void BrushEntity::RemoveBrush(const Solid& brush)
    {
        m_solids.remove(brush);
    }

    std::optional<RayHit> BrushEntity::QueryRay(const Ray& ray) const
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
}
