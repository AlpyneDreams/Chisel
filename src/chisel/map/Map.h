#pragma once

#include <list>

#include "Entity.h"
#include "Action.h"

namespace chisel
{
    /**
     * Represents the entire map document, world brushes, and entities.
     */
    class Map final : public BrushEntity
    {
    public:
        Map();
        ~Map();

        bool Empty() const;

        void Clear();

        bool IsMap() final override;

        PointEntity* AddPointEntity(const char* classname);

        void AddEntity(Entity *entity);
        void RemoveEntity(Entity& entity);

        auto Entities() { return IteratorPassthru(m_entities); }
        ActionList& Actions() { return m_actions; }

    private:
        // TODO: Polymorphic linked list
        std::vector<Entity*> m_entities;

        ActionList m_actions;
    };
}