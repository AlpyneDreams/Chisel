#include "Map.h"

namespace chisel
{
    Map::Map()
        : BrushEntity(nullptr)
    {
    }

    Map::~Map()
    {
        Clear();
    }

    bool Map::Empty() const
    {
        return m_solids.empty() && m_entities.empty();
    }

    void Map::Clear()
    {
        m_solids.clear();
        for (Entity* ent : m_entities)
            delete ent;
        m_entities.clear();
    }

    bool Map::IsMap()
    {
        return true;
    }

    PointEntity* Map::AddPointEntity(const char* classname)
    {
        PointEntity* ent = new PointEntity(this);
        ent->classname = classname;
        m_entities.push_back(ent);
        return ent;
    }

    void Map::AddEntity(Entity* entity)
    {
        // CHANGE ME
        m_entities.push_back(entity);
    }

    void Map::RemoveEntity(Entity& entity)
    {
        // SUCKS
        m_entities.erase(std::remove_if(m_entities.begin(),
            m_entities.end(),
            [&](Entity* a)-> bool
            { return a == &entity; }),
            m_entities.end());

        delete &entity;
    }
}
