#pragma once

#include "common/Common.h"
#include "console/Console.h"

#include "math/AABB.h"
#include "map/Common.h"
#include <optional>
#include <unordered_map>

namespace chisel
{
    using SelectionID = uint32_t;

    class Selectable
    {
    public:
        Selectable();
        virtual ~Selectable();

        SelectionID GetSelectionID() const { return m_id; }
        bool IsSelected() const { return m_selected; }

        virtual std::optional<AABB> GetBounds() const = 0;
        virtual void Transform(const mat4x4& matrix) = 0;
        virtual void Delete() = 0;
        virtual void AlignToGrid(vec3 gridSize) = 0;
        virtual void SetVolume(Volume volume) = 0;
    protected:
        friend class Selection;

        void SetSelected(bool selected) { m_selected = selected; }
        static Selectable* Find(SelectionID id);
    private:
        static SelectionID s_lastId;
        static std::unordered_map<SelectionID, Selectable*> s_map;

        SelectionID m_id = 0;
        bool m_selected = false;
    };

    extern class Selection : public Selectable
    {
    public:
        Selection();

        bool Empty() const;
        void Select(Selectable* ent);
        void Unselect(Selectable* ent);
        void Toggle(Selectable* ent);
        void Clear();
        Selectable* Find(SelectionID id);

        Selectable** begin() { return m_selection.size() > 0 ? &m_selection.front() : nullptr; }
        Selectable** end()   { return m_selection.size() > 0 ? &m_selection.back() + 1 : nullptr; }

    public:
    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override;
        void Transform(const mat4x4& matrix) final override { for (auto* s : m_selection) { s->Transform(matrix); } }
        void Delete() final override;
        void AlignToGrid(vec3 gridSize) final override { for (auto* s : m_selection) { s->AlignToGrid(gridSize); } }
        void SetVolume(Volume volume) final override { for (auto* s : m_selection) { s->SetVolume(volume); } }

    private:
        std::vector<Selectable*> m_selection;
    } Selection;
}