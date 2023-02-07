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

namespace chisel
{
    struct SideData
    {
        std::array<vec4, 2> textureAxes{};
        std::array<float, 2> scale{ 1.0f, 1.0f };
        float rotate = 0;
        float lightmapScale = 16;
        uint32_t smoothing = 0;
    };

    struct Solid : Atom, ISelectable
    {
    protected:
        CSG::Brush*             brush;
        Volume                  volume;

        MeshBuffer<VertexCSG>   mesh = MeshBuffer<VertexCSG>();     
        Color                   tempcolor;

        static inline Texture* gridTexture = nullptr;

    public:
        Solid(CSG::Brush& brush, Volume volume)
            : brush(&brush), volume(volume)
        {
            brush.SetVolumeOperation(CSG::CreateFillOperation(volume));
            brush.userdata = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));

            srand(brush.GetObjectID());
            tempcolor = Color::HSV((float)(rand() % 360), 0.7f, 1.0f);

            if (!gridTexture)
                gridTexture = Assets.Load<Texture, ".PNG">("materials/dev_grey.png");
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
            mesh.vertices.clear();
            mesh.indices.clear();

            for (auto& face : brush->GetFaces())
            {
                for (auto& fragment : face.fragments)
                {
                    if (fragment.back.volume == fragment.front.volume)
                        continue;

                    const bool flipFace = fragment.back.volume == Volumes::Air;
                    const vec3 normal = flipFace
                        ? -face.side->plane.normal
                        :  face.side->plane.normal;

                    const size_t startIndex = mesh.vertices.size();
                    const SideData& data = m_sides[face.side->userdata];
                    for (const auto& vert : fragment.vertices)
                    {
                        const float mappingWidth = 64.0f;
                        const float mappingHeight = 64.0f;

                        float u = glm::dot(glm::vec3(data.textureAxes[0].xyz), glm::vec3(vert.position)) / data.scale[0] + data.textureAxes[0].w;
                        float v = glm::dot(glm::vec3(data.textureAxes[1].xyz), glm::vec3(vert.position)) / data.scale[1] + data.textureAxes[1].w;

                        u = mappingWidth  ? u / float(mappingWidth)  : 0.0f;
                        v = mappingHeight ? v / float(mappingHeight) : 0.0f;

                        Console.Log("B: uv: {} {}", u, v);

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
            mesh.Update();
        }

        Texture *GetTexture()
        {
            return gridTexture;
        }

        std::vector<SideData> m_sides;

        CSG::ObjectID GetObjectID() const { return brush->GetObjectID(); }
        CSG::Brush& GetBrush() { return *brush; }

        Mesh* GetMesh() { return &mesh; }

        // TODO: Remove me, debugging.
        glm::vec4 GetTempColor() const { return tempcolor; }

        void Delete()
        {
            Console.Error("Need a reference to the map in here to kill myself.");
        }

        // Selectable Interface

        std::optional<AABB> SelectionBounds() const { return brush->GetBounds(); }
        void SelectionTransform(const mat4x4& matrix) { brush->Transform(matrix); }
        void SelectionDelete() { Delete(); }
        void SelectionAlignToGrid(vec3 gridSize) { brush->AlignToGrid(gridSize); }
        void SelectionSetVolume(Volume volume) { brush->SetVolumeOperation(CSG::CreateFillOperation(volume)); }

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