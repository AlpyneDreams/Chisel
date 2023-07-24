#pragma once

#include "common/Common.h"
#include "console/Console.h"

#include "math/AABB.h"
#include "math/Math.h"
#include <optional>
#include <unordered_map>
#include <stack>

namespace chisel
{
    using SelectionID = uint32_t;

    class Selectable
    {
    public:
        Selectable();
        virtual ~Selectable();

        SelectionID GetSelectionID() const { return m_id; }
        virtual bool IsSelected() const { return m_selected; }

        virtual std::optional<AABB> GetBounds() const = 0;
        virtual void Transform(const mat4x4& matrix) = 0;
        virtual void Delete() = 0;
        virtual void AlignToGrid(vec3 gridSize) = 0;
        virtual Selectable* ResolveSelectable() { return this; }
    protected:
        friend class Selection;

        void SetSelected(bool selected) { m_selected = selected; }
        static Selectable* Find(SelectionID id);
    private:
        static SelectionID s_nextID;
        static inline std::unordered_map<SelectionID, Selectable*> s_map;
        static inline std::stack<SelectionID> s_freeIDs;

        static inline SelectionID NextID();

        SelectionID m_id = 0;
        bool m_selected = false;
    };

    extern class Selection : public Selectable
    {
    public:
        Selection();

        bool Empty() const;
        uint Count() const { return m_selection.size(); }
        void Select(Selectable* ent);
        void Unselect(Selectable* ent);
        void Toggle(Selectable* ent);
        void Clear();
        Selectable* Find(SelectionID id);

        Selectable** begin() { return m_selection.size() > 0 ? &m_selection.front() : nullptr; }
        Selectable** end()   { return m_selection.size() > 0 ? &m_selection.back() + 1 : nullptr; }
        Selectable* operator [](size_t index) { return m_selection[index]; }

    public:
    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override;
        void Transform(const mat4x4& matrix) final override { for (auto* s : m_selection) s->Transform(matrix); }
        void Delete() final override;
        void AlignToGrid(vec3 gridSize) final override { for (auto* s : m_selection) s->AlignToGrid(gridSize); }

    private:
        std::vector<Selectable*> m_selection;
    } Selection;
}