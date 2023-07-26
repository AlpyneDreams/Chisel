#pragma once

#include "console/ConVar.h"
#include "chisel/Selection.h"
#include "assets/Assets.h"
#include "render/Render.h"
#include "Atom.h"

#include "math/Color.h"

#include "Common.h"
#include "Face.h"

#include <memory>
#include <unordered_map>

namespace chisel
{
    class Solid;
    class BrushEntity;

    struct BrushMesh
    {
        std::vector<VertexSolid> vertices;
        std::vector<uint32_t>    indices;

        std::optional<BrushGPUAllocator::Allocation> alloc;
        Material *material = nullptr;
        Solid *brush = nullptr;
    };

    class Solid : public Atom
    {
    public:
        Solid(BrushEntity* parent);
        Solid(BrushEntity* parent, std::vector<Side> sides, bool initMesh = true);
        Solid(Solid&& other);
        ~Solid();

        // What the fuck, why do I need this?
        bool operator == (const Solid& other) const
        {
            return this == &other;
        }

        bool HasDisplacement() const { return m_displacement; }
        std::vector<BrushMesh>& GetMeshes() { return m_meshes; }
        const std::vector<Side>& GetSides() const { return m_sides; }
        const std::vector<Face>& GetFaces() const { return m_faces; }

        void Clip(Side side); // Remember to UpdateMesh after this!

        void UpdateMesh();


    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override { return m_bounds; }

        void Transform(const mat4x4& _matrix) final override;
        void AlignToGrid(vec3 gridSize) final override;

        Selectable* ResolveSelectable() final override;
        bool IsSelected() const final override;

        void Delete() final override;

        Selectable* Duplicate() override;

    private:
        friend struct Face;

        bool m_displacement = false;

        std::vector<BrushMesh> m_meshes;
        std::vector<Side> m_sides;
        std::optional<AABB> m_bounds;

        std::vector<Face> m_faces;
    };

    std::vector<Side> CreateCubeBrush(Material* material, vec3 size = vec3(64.f), const mat4x4& transform = glm::identity<mat4x4>());
}