#pragma once

#include "core/Mesh.h"
#include "../CSG/Brush.h"
#include "../CSG/CSGTree.h"
#include "chisel/Selection.h"
#include "assets/Assets.h"
#include "render/Texture.h"

#include "math/Color.h"

#include "Common.h"

#include <memory>
#include <unordered_map>

namespace chisel
{
    struct SideData
    {
        Texture *texture{};
        std::array<vec4, 2> textureAxes{};
        std::array<float, 2> scale{ 1.0f, 1.0f };
        float rotate = 0;
        float lightmapScale = 16;
        uint32_t smoothing = 0;
    };

    struct BrushMesh
    {
        MeshBuffer<VertexCSG> mesh = MeshBuffer<VertexCSG>();
        Texture *texture = nullptr;
    };

    struct Solid : Atom
    {
    protected:
        CSG::Brush*             brush;
        Volume                  volume;

        std::vector<BrushMesh>  meshes;
        Color                   tempcolor;

        Texture* gridTexture = nullptr;

    public:
        Solid(CSG::Brush& brush, Volume volume)
            : brush(&brush), volume(volume)
        {
            brush.SetVolumeOperation(CSG::CreateFillOperation(volume));
            brush.userdata = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));

            srand(brush.GetObjectID());
            tempcolor = Color::HSV((float)(rand() % 360), 0.7f, 1.0f);
        }

        Solid(Solid&& that)
            : Solid(*that.brush, that.volume)
        {
            that.brush = nullptr;
        }
        
        ~Solid()
        {
            if (brush)
                brush->GetTree()->DestroyBrush(*brush);
        }

        bool operator ==(const Solid& that) const
        {
            return brush == that.brush
                && brush != nullptr
                && that.brush != nullptr;
        }

        void SetSides(CSG::Side *begin_side, CSG::Side *end_side, SideData* begin_data, SideData* end_data)
        {
            GetBrush().SetSides(begin_side, end_side);
            m_sides = std::vector<SideData>(begin_data, end_data);
        }

        void UpdateMesh()
        {
            meshes.clear();

            // Create one mesh for each unique material
            size_t textureCount = 0;
            std::unordered_map<Texture*, size_t> uniqueTextures;
            for (auto& face : brush->GetFaces())
            {
                const SideData& data = m_sides[face.side->userdata];
                if (!uniqueTextures.contains(data.texture))
                    uniqueTextures.emplace(data.texture, textureCount++);
            }

            meshes.resize(textureCount);

            for (auto& face : brush->GetFaces())
            {
                const SideData& data = m_sides[face.side->userdata];

                BrushMesh& brushmesh = meshes[uniqueTextures[data.texture]];
                brushmesh.texture = data.texture;
                auto& mesh = brushmesh.mesh;

                for (auto& fragment : face.fragments)
                {
                    if (fragment.back.volume == fragment.front.volume)
                        continue;

                    const bool flipFace = fragment.back.volume == Volumes::Air;
                    const vec3 normal = flipFace
                        ? -face.side->plane.normal
                        :  face.side->plane.normal;

                    const size_t startIndex = mesh.vertices.size();

                    for (const auto& vert : fragment.vertices)
                    {
                        const float mappingWidth = 64.0f;
                        const float mappingHeight = 64.0f;

                        float u = glm::dot(glm::vec3(data.textureAxes[0].xyz), glm::vec3(vert.position)) / data.scale[0] + data.textureAxes[0].w;
                        float v = glm::dot(glm::vec3(data.textureAxes[1].xyz), glm::vec3(vert.position)) / data.scale[1] + data.textureAxes[1].w;

                        u = mappingWidth  ? u / float(mappingWidth)  : 0.0f;
                        v = mappingHeight ? v / float(mappingHeight) : 0.0f;

                        mesh.vertices.emplace_back(vert.position, normal, glm::vec2(u, v));
                    }

                    std::vector<CSG::TriangleIndices> tris = fragment.Triangulate();
                    for (const auto& tri : tris)
                    {
                        if (flipFace)
                        {
                            mesh.indices.push_back(startIndex + tri[0]);
                            mesh.indices.push_back(startIndex + tri[2]);
                            mesh.indices.push_back(startIndex + tri[1]);
                        }
                        else
                        {
                            mesh.indices.push_back(startIndex + tri[0]);
                            mesh.indices.push_back(startIndex + tri[1]);
                            mesh.indices.push_back(startIndex + tri[2]);
                        }
                        /*fprintf(stderr, "Adding triangle: (%g %g %g), (%g %g %g), (%g %g %g)\n",
                            mesh.vertices[startIndex + tri[0]].position.x, mesh.vertices[startIndex + tri[0]].position.y, mesh.vertices[startIndex + tri[0]].position.z,
                            mesh.vertices[startIndex + tri[1]].position.x, mesh.vertices[startIndex + tri[1]].position.y, mesh.vertices[startIndex + tri[1]].position.z,
                            mesh.vertices[startIndex + tri[2]].position.x, mesh.vertices[startIndex + tri[2]].position.y, mesh.vertices[startIndex + tri[2]].position.z);*/
                    }
                }
            }

            // Upload all meshes after they're complete
            for (auto& submesh : meshes)
                submesh.mesh.Update();
        }

        std::vector<SideData> m_sides;

        CSG::ObjectID GetObjectID() const { return brush->GetObjectID(); }
        CSG::Brush& GetBrush() { return *brush; }

        std::vector<BrushMesh>& GetMeshes() { return meshes; }

        // TODO: Remove me, debugging.
        glm::vec4 GetTempColor() const { return tempcolor; }

    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override { return brush->GetBounds(); }
        void Transform(const mat4x4& matrix) final override { brush->Transform(matrix); }
        void AlignToGrid(vec3 gridSize) final override { brush->AlignToGrid(gridSize); }
        void SetVolume(Volume volume) final override { brush->SetVolumeOperation(CSG::CreateFillOperation(volume)); }
    };

    inline Solid CubeBrush(CSG::Brush& brush, Volume volume, const mat4x4& transform = glm::identity<mat4x4>())
    {
        Solid cube = Solid(brush, volume);
        
        static const std::array<CSG::Plane, 6> kUnitCubePlanes =
        {
            CSG::Plane(vec3(+1,0,0), vec3(+1,0,0)),
            CSG::Plane(vec3(-1,0,0), vec3(-1,0,0)),
            CSG::Plane(vec3(0,+1,0), vec3(0,+1,0)),
            CSG::Plane(vec3(0,-1,0), vec3(0,-1,0)),
            CSG::Plane(vec3(0,0,+1), vec3(0,0,+1)),
            CSG::Plane(vec3(0,0,-1), vec3(0,0,-1))
        };
        
        std::array<CSG::Side, 6> sides;
        for (size_t i = 0; i < 6; i++)
            sides[i].plane = kUnitCubePlanes[i].Transformed(glm::scale(transform, vec3(64.f)));
        
        cube.GetBrush().SetSides(&sides.front(), &sides.back() + 1);
        
        return cube;
    }
}