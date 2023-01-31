#pragma once

#include "common/Common.h"
#include "console/Console.h"

#include "math/AABB.h"
#include "map/Common.h"
#include <optional>
#include <unordered_map>

namespace chisel
{
    class Selection;

    using SelectionID = uint32_t;


    class ISelectable
    {
    public:
        ISelectable();
        virtual ~ISelectable();

        SelectionID GetSelectionID() const { return m_id; }
        bool IsSelected() const { return m_selected; }

        virtual std::optional<AABB> SelectionBounds() const = 0;
        virtual void SelectionTransform(const mat4x4& matrix) = 0;
        virtual void SelectionDelete() = 0;
        virtual void SelectionAlignToGrid(vec3 gridSize) = 0;
        virtual void SelectionSetVolume(Volume volume) = 0;
    protected:
        friend class Selection;

        void SetSelected(bool selected) { m_selected = selected; }
        static ISelectable* Find(SelectionID id);
    private:
        static SelectionID s_lastId;
        static std::unordered_map<SelectionID, ISelectable*> s_map;

        SelectionID m_id = 0;
        bool m_selected = false;
    };

    extern class Selection
    {
    public:
        Selection();

        bool Empty();
        void Select(ISelectable* ent);
        void Unselect(ISelectable* ent);
        void Toggle(ISelectable* ent);
        void Clear();
        ISelectable* Find(SelectionID id);

        ISelectable** begin() { return &m_selection.front(); }
        ISelectable** end()   { return &m_selection.back() + 1; }
    private:
        std::vector<ISelectable*> m_selection;
    } Selection;
}