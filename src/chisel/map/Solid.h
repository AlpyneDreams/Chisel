#pragma once

#include "console/ConVar.h"
#include "chisel/Selection.h"
#include "assets/Assets.h"
#include "render/Render.h"
#include "Orientation.h"

#include "math/Color.h"

#include "Common.h"

#include <memory>
#include <unordered_map>
#include <array>

namespace chisel
{
    struct Side
    {
        Plane plane{};

        Material *material{};
        std::array<vec4, 2> textureAxes{};
        std::array<float, 2> scale{ 1.0f, 1.0f };
        float rotate = 0;
        float lightmapScale = 16;
        uint32_t smoothing = 0;
    };

    struct Solid;

    struct BrushMesh
    {
        std::vector<VertexSolid> vertices;
        std::vector<uint32_t>    indices;

        std::optional<BrushGPUAllocator::Allocation> alloc;
        Material *material = nullptr;
        Solid *brush = nullptr;
    };

    // TODO: Move to some transform state?
    extern ConVar<bool>  trans_texture_lock;
    extern ConVar<bool>  trans_texture_scale_lock;
    extern ConVar<bool>  trans_texture_face_alignment;

    struct Face
    {
        Side* side;
        std::vector<vec3> points;
    };

    struct Solid : Atom
    {
    public:
        Solid();
        Solid(std::vector<Side> sides, bool initMesh = true);
        Solid(Solid&& other);
        ~Solid();

        // What the fuck, why do I need this?
        bool operator == (const Solid& other) const
        {
            return this == &other;
        }

        std::vector<BrushMesh>& GetMeshes() { return m_meshes; }
        const std::vector<Face>& GetFaces() const { return m_faces; }

        void UpdateMesh();

    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override { return m_bounds; }

        void Transform(const mat4x4& _matrix) final override;
        void AlignToGrid(vec3 gridSize) final override;

    private:
        std::vector<BrushMesh> m_meshes;
        std::vector<Side> m_sides;
        std::optional<AABB> m_bounds;

        std::vector<Face> m_faces;
    };

    std::vector<Side> CreateCubeBrush(vec3 size = vec3(64.f), const mat4x4& transform = glm::identity<mat4x4>());
}