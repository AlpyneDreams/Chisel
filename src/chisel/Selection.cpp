#include "chisel/Selection.h"

namespace chisel
{
    SelectionID Selectable::s_lastId = 0;

//-------------------------------------------------------------------------------------------------

    Selectable::Selectable()
        : m_id(++s_lastId)
    {
        s_map.emplace(m_id, this);
    }

    Selectable::~Selectable()
    {
        Selection.Unselect(this);
        s_map.erase(m_id);
    }

    /*static*/ Selectable* Selectable::Find(SelectionID id)
    {
        auto iter = s_map.find(id);
        if (iter == s_map.end())
            return nullptr;

        return iter->second;
    }

//-------------------------------------------------------------------------------------------------

    Selection::Selection()
    {
    }

    bool Selection::Empty() const
    {
        return m_selection.empty();
    }

    void Selection::Select(Selectable* ent)
    {
        if (ent->IsSelected())
            return;

        ent->SetSelected(true);
        m_selection.emplace_back(ent);
    }

    void Selection::Unselect(Selectable* ent)
    {
        if (!ent->IsSelected())
            return;

        ent->SetSelected(false);
        if (m_selection.size() > 0)
            std::erase(m_selection, ent);
    }

    void Selection::Toggle(Selectable* ent)
    {
        if (ent->IsSelected())
            Unselect(ent);
        else
            Select(ent);
    }

    void Selection::Clear()
    {
        for (const auto& selected : m_selection)
            selected->SetSelected(false);
        m_selection.clear();
    }

    Selectable* Selection::Find(SelectionID id)
    {
        return Selectable::Find(id);
    }

//-------------------------------------------------------------------------------------------------

    std::optional<AABB> Selection::GetBounds() const
    {
        std::optional<AABB> bounds;
        for (Selectable* selectable : m_selection)
        {
            auto selectedBounds = selectable->GetBounds();
            if (!selectedBounds)
                continue;

            bounds = bounds
                ? AABB::Extend(*bounds, *selectedBounds)
                : *selectedBounds;
        }
        
        return bounds;
    }

    void Selection::Delete()
    {
        for (Selectable* s : m_selection)
        {
            s->Delete();
        }
        Clear();
    }

    class Selection Selection;

}
