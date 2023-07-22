#pragma once

#include <list>

#include "core/Mesh.h"
#include "Common.h"
#include "Solid.h"
#include "math/Ray.h"
#include "formats/KeyValues.h"
#include <any>

namespace chisel
{
    class Action
    {
    public:
        Action(const char *name, bool dummy)
            : m_dummy(dummy) { }

        virtual ~Action() { }

        virtual void Do() = 0;
        virtual void Undo() = 0;

        bool IsDummy() const { return m_dummy; }

        void SetGraphInfo(Action *prev, Action *next0, Action *next1)
        {
            m_prev = prev;
            m_next0 = next0;
            m_next1 = next1;
        }

        Action* GetPrev() const  { return m_prev; }
        Action* GetNext0() const { return m_next0; }
        Action* GetNext1() const  { return m_next1; }
    private:
        Action* m_prev = nullptr;
        Action* m_next0 = nullptr; // Our most recent paths.
        Action* m_next1 = nullptr; // The past do's... Only used on dummy nodes

        // Whether this is a dummy node in our history graph
        // to represent a divergence in history
        bool m_dummy = false;
    };

    // Dummy nodes to simplify the graph
    class DummyAction final : public Action
    {
    public:
        DummyAction(const char *name)
            : Action(name, true) { }

        DummyAction             (DummyAction&&) = delete;
        DummyAction& operator = (DummyAction&&) = delete;

        // Should never be executed as it's a dummy.
        void Do()   { assert(false && "Attempted to do dummy action"); }
        void Undo() { assert(false && "Attempted to undo dummy action"); }
    };

    template <typename DoCmd, typename UndoCmd>
    class FunctionAction final : public Action
    {
    public:
        FunctionAction(const char *name, DoCmd&& doCmd, UndoCmd&& undoCmd)
            : Action(name, false)
            , m_do(std::move(doCmd))
            , m_undo(std::move(undoCmd)) { }

        FunctionAction             (FunctionAction&&) = delete;
        FunctionAction& operator = (FunctionAction&&) = delete;

        void Do() { m_do(m_userdata); }
        void Undo() { m_undo(m_userdata); m_userdata.reset(); }
    private:
        std::any m_userdata;

        DoCmd   m_do;
        UndoCmd m_undo;
    };

    class ActionList
    {
    public:
        ActionList()
        {
            m_head = new DummyAction("Root");
            m_activeTail = m_head;
        }

        template <typename DoCmd, typename UndoCmd>
        void PerformAction(const char *name, DoCmd&& doFunc, UndoCmd&& undoFunc)
        {
            Action* prevTail = m_activeTail;
            Action* prevNext = prevTail->GetNext0();

            m_activeTail = new FunctionAction<DoCmd, UndoCmd>(name, std::move(doFunc), std::move(undoFunc));
            // Insert a dummy node if we have divergence.
            if (prevNext)
            {
                DummyAction *dummy = new DummyAction("Divergence");
                dummy->SetGraphInfo(prevTail, m_activeTail, prevNext);
                m_activeTail->SetGraphInfo(dummy, nullptr, nullptr);
                prevNext->SetGraphInfo(dummy, prevNext->GetNext0(), prevNext->GetNext1());
                prevTail->SetGraphInfo(prevTail->GetPrev(), m_activeTail, prevTail->GetNext1());

                //                  /-- m_activeTail (new)
                // prevTail -- dummy
                //                  \-- prevNext (prevTail->GetNext0())
            }
            else
            {
                m_activeTail->SetGraphInfo(prevTail, nullptr, nullptr);
                prevTail->SetGraphInfo(prevTail->GetPrev(), m_activeTail, prevTail->GetNext1());

                // prevTail -- m_activeTail
            }

            m_activeTail->Do();

            GraphSanityAssert();
        }

        void Undo()
        {
            if (!m_activeTail->IsDummy())
            {
                m_activeTail->Undo();

                do
                {
                    if (Action *newTail = m_activeTail->GetPrev())
                        m_activeTail = newTail;
                } while (m_activeTail->IsDummy() && m_activeTail != m_head);
            }

            GraphSanityAssert();
        }

        void Redo()
        {
            Action *redoNode = nullptr;
            do
            {
                if ( Action *nextNode = m_activeTail->GetNext0() )
                    redoNode = nextNode;
            } while ( redoNode && redoNode->IsDummy() );

            if ( redoNode && !redoNode->IsDummy() )
            {
                redoNode->Do();
                m_activeTail = redoNode;
            }

            GraphSanityAssert();
        }

        void GraphSanityAssert()
        {
            assert(m_activeTail && m_head);
            assert(!m_activeTail->IsDummy() || m_activeTail == m_head);
            assert(m_head->IsDummy());
        }
    private:
        Action *m_head = nullptr;
        Action *m_activeTail = nullptr;
    };

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
        std::optional<AABB> GetBounds() const final override { return AABB{origin - vec3(32), origin + vec3(32)}; }
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

        Solid& AddCube(Material* material, mat4x4 transform = glm::identity<mat4x4>(), vec3 size = vec3(64.f))
        {
            return AddBrush(CreateCubeBrush(material, size, transform));
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

        ActionList actions;
    };

    inline void Entity::Delete()
    {
        assert(m_parent->IsMap());
        static_cast<Map*>(m_parent)->RemoveEntity(*this);
    }
}