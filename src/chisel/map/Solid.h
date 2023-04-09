#pragma once

#include "../CSG/Brush.h"
#include "../CSG/CSGTree.h"
#include "console/ConVar.h"
#include "chisel/Selection.h"
#include "assets/Assets.h"
#include "render/Render.h"

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
        std::vector<VertexCSG> vertices;
        std::vector<uint32_t>  indices;

        std::optional<BrushGPUAllocator::Allocation> alloc;
        Texture *texture = nullptr;
    };

    // TODO: Move to some transform state?
    extern ConVar<bool>  trans_texture_lock;
    extern ConVar<bool>  trans_texture_scale_lock;

    struct Solid : Atom
    {
    protected:
        CSG::Brush*             brush;
        Volume                  volume;

        std::vector<BrushMesh>  meshes;
        Color                   tempcolor;

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

        void UpdateMesh(BrushGPUAllocator& a)
        {
            // TODO: Avoid clearing meshes out every time.
            for (auto& mesh : meshes)
            {
                if (mesh.alloc)
                {
                    a.free(*mesh.alloc);
                    mesh.alloc = std::nullopt;
                }
            }
            meshes.clear();

            static const SideData defaultSide;

            // Create one mesh for each unique material
            size_t textureCount = 0;
            std::unordered_map<Texture*, size_t> uniqueTextures;
            for (auto& face : brush->GetFaces())
            {
                const SideData& data = m_sides.empty() ? defaultSide : m_sides[face.side->userdata];
                if (!uniqueTextures.contains(data.texture))
                    uniqueTextures.emplace(data.texture, textureCount++);
            }

            meshes.resize(textureCount);

            render::RenderContext& r = a.rctx();

            for (auto& face : brush->GetFaces())
            {
                const SideData& data = m_sides.empty() ? defaultSide : m_sides[face.side->userdata];

                BrushMesh& mesh = meshes[uniqueTextures[data.texture]];
                mesh.texture = data.texture;

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
                        float mappingWidth = 32.0f;
                        float mappingHeight = 32.0f;
                        if (data.texture != nullptr)
                        {
                            D3D11_TEXTURE2D_DESC desc;
                            data.texture->texture->GetDesc(&desc);

                            mappingWidth = float(desc.Width);
                            mappingHeight = float(desc.Height);
                        }

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
                    }
                }
            }

            // Upload all meshes after they're complete
            for (auto& mesh : meshes)
            {
                uint32_t verticesSize = sizeof(VertexCSG) * mesh.vertices.size();
                uint32_t indicesSize = sizeof(uint32_t) * mesh.indices.size();
                mesh.alloc = a.alloc(verticesSize + indicesSize);
                // Store vertices then indices.
                memcpy(&a.data()[mesh.alloc->offset + 0],            mesh.vertices.data(), verticesSize);
                memcpy(&a.data()[mesh.alloc->offset + verticesSize], mesh.indices.data(),  indicesSize);
            }
        }

        std::vector<SideData> m_sides;

        CSG::ObjectID GetObjectID() const { return brush->GetObjectID(); }
        CSG::Brush& GetBrush() { return *brush; }

        std::vector<BrushMesh>& GetMeshes() { return meshes; }

        // TODO: Remove me, debugging.
        glm::vec4 GetTempColor() const { return tempcolor; }

    // Selectable Interface //

        std::optional<AABB> GetBounds() const final override { return brush->GetBounds(); }
        void Transform(const mat4x4& _matrix) final override
        {
            brush->Transform(_matrix);
            for (auto& side : m_sides)
            {
                mat4x4 trans = _matrix;

                bool locking = trans_texture_lock;
                bool scaleLocking = trans_texture_scale_lock;

                vec3 delta = trans[3].xyz;
                trans[3].xyz = glm::vec3(0.0f, 0.0f, 0.0f);
                
                bool moving = glm::length2(delta) > 0.00001f;

                if (trans == glm::identity<glm::mat4x4>())
                {
                    if (moving && locking)
                    {
                        side.textureAxes[0][3] -= glm::dot(delta, vec3(side.textureAxes[0].xyz)) / side.scale[0];
                        side.textureAxes[1][3] -= glm::dot(delta, vec3(side.textureAxes[1].xyz)) / side.scale[1];
                    }

                    continue;
                }

                vec3 u = side.textureAxes[0];
                vec3 v = side.textureAxes[1];

                float scaleU = glm::length(u);
                float scaleV = glm::length(v);
                if (scaleU <= 0.0f) scaleU = 1.0f;
                if (scaleV <= 0.0f) scaleV = 1.0f;

                u = glm::mat3(trans) * u;
                v = glm::mat3(trans) * v;

                scaleU = glm::length(u) / scaleU;
                scaleV = glm::length(v) / scaleV;
                if (scaleU <= 0.0f) scaleU = 1.0f;
                if (scaleV <= 0.0f) scaleV = 1.0f;

                bool uvAxisSameScale = math::CloseEnough(scaleU, 1.0f, 0.0001f) && math::CloseEnough(scaleV, 1.0f, 0.0001f);
                bool uvAxisPerpendicular = math::CloseEnough(glm::dot(u, v), 0.0f, 0.0025f);

                if (locking && uvAxisPerpendicular)
                {
                    side.textureAxes[0].xyz = u / scaleU;
                    side.textureAxes[1].xyz = v / scaleV;
                }

                if (uvAxisSameScale)
                {
                    if (!locking)
                    {
                        // TODO: re-init
                    }
                }
                else
                {
                    if (scaleLocking)
                    {
                        side.scale[0] *= scaleU;
                        side.scale[1] *= scaleV;
                    }
                }

                if (moving && locking)
                {
                    side.textureAxes[0][3] -= glm::dot(delta, vec3(side.textureAxes[0].xyz)) / side.scale[0];
                    side.textureAxes[1][3] -= glm::dot(delta, vec3(side.textureAxes[1].xyz)) / side.scale[1];
                }
            }
        }
        void AlignToGrid(vec3 gridSize) final override { brush->AlignToGrid(gridSize); }
        void SetVolume(Volume volume) final override { brush->SetVolumeOperation(CSG::CreateFillOperation(volume)); }
    };

    inline Solid CubeBrush(CSG::Brush& brush, Volume volume, vec3 size = vec3(64.f), const mat4x4& transform = glm::identity<mat4x4>())
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
            sides[i].plane = kUnitCubePlanes[i].Transformed(glm::scale(transform, size));
        
        cube.GetBrush().SetSides(&sides.front(), &sides.back() + 1);
        
        return cube;
    }
}