#pragma once

#include "Atom.h"
#include "math/Math.h"
#include "RayHit.h"
#include "Solid.h"
#include "formats/KeyValues.h"
#include <optional>
#include <list>

namespace chisel
{
    class Entity : public Atom
    {
    public:
        Entity(BrushEntity* parent);

        void Delete() final override;

        virtual bool IsBrushEntity() const = 0;

    // Public members

        std::string classname;
        std::string targetname;

        glm::vec3 origin;

        kv::KeyValues kv;
    };

    class PointEntity final : public Entity
    {
    public:
        PointEntity(BrushEntity* parent);

    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override;
        void Transform(const mat4x4& matrix) final override;
        void AlignToGrid(vec3 gridSize) final override;

    // Entity Interface //

        virtual bool IsBrushEntity() const override { return false; }
    };

    class BrushEntity : public Entity
    {
    public:
        BrushEntity(BrushEntity* parent);

    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override;
        void Transform(const mat4x4& matrix) final override;
        void AlignToGrid(vec3 gridSize) final override;

    // Entity Interface //

        bool IsBrushEntity() const override { return true; }

    //

        virtual bool IsMap() { return false; }

        auto Brushes() { return IteratorPassthru(m_solids); }

        Solid& AddBrush(std::vector<Side> sides);

        void RemoveBrush(const Solid& brush);

        std::optional<RayHit> QueryRay(const Ray& ray) const;

    protected:

        std::list<Solid> m_solids;
    };
}