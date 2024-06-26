#include "chisel/Selection.h"

namespace chisel
{
    SelectionID Selectable::s_nextID = 1;

//-------------------------------------------------------------------------------------------------

    inline SelectionID Selectable::NextID()
    {
        if (s_freeIDs.empty())
            return s_nextID++;
        
        SelectionID id = s_freeIDs.top();
        s_freeIDs.pop();
        return id;
    }

    Selectable::Selectable()
        : m_id(NextID())
    {
        s_map.emplace(m_id, this);
    }

    Selectable::~Selectable()
    {
        s_freeIDs.push(m_id);
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

        Selectable* resolved;
        while ((resolved = ent->ResolveSelectable()) != ent)
            ent = resolved;

        ent->SetSelected(true);
        m_selection.emplace_back(ent);
    }

    void Selection::Unselect(Selectable* ent)
    {
        if (!ent->IsSelected())
            return;

        Selectable* resolved;
        while ((resolved = ent->ResolveSelectable()) != ent)
            ent = resolved;

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

    bool Selection::Duplicate()
    {
        bool containsUnduplicatables = false;

        for (Selectable*& s : m_selection)
        {
            Selectable *duplicated = s->Duplicate();
            if (duplicated)
            {
                s->SetSelected(false);
                duplicated->SetSelected(true);
                s = duplicated;
            }
            else
                containsUnduplicatables = true;
        }

        return containsUnduplicatables;
    }

    class Selection Selection;

}
