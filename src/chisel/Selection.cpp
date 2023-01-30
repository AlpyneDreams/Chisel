#include "chisel/Selection.h"

namespace chisel
{
    SelectionID ISelectable::s_lastId = 0;
    std::unordered_map<SelectionID, ISelectable*> ISelectable::s_map;

//-------------------------------------------------------------------------------------------------

    ISelectable::ISelectable()
        : m_id(++s_lastId)
    {
        s_map.emplace(m_id, this);
    }

    ISelectable::~ISelectable()
    {
        Selection.Unselect(this);
        s_map.erase(m_id);
    }

    /*static*/ ISelectable* ISelectable::Find(SelectionID id)
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

    bool Selection::Empty()
    {
        return m_selection.empty();
    }

    void Selection::Select(ISelectable* ent)
    {
        if (ent->IsSelected())
            return;

        ent->SetSelected(true);
        m_selection.emplace_back(ent);
    }

    void Selection::Unselect(ISelectable* ent)
    {
        if (!ent->IsSelected())
            return;

        ent->SetSelected(false);
        std::erase(m_selection, ent);
    }

    void Selection::Toggle(ISelectable* ent)
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

    ISelectable* Selection::Find(SelectionID id)
    {
        return ISelectable::Find(id);
    }

    class Selection Selection;

}
