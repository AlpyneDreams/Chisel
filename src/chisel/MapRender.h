#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "chisel/Chisel.h"
#include "common/System.h"
#include "chisel/Tools.h"
#include "chisel/VMF.h"
#include "chisel/Selection.h"

#include "core/Primitives.h"
#include "core/VertexLayout.h"
#include "common/Time.h"
#include "math/Math.h"
#include "math/Color.h"
#include <glm/gtx/normal.hpp>

#include "CSG/CSGTree.h"

namespace chisel
{
    namespace ChiselVolumes
    {
        enum ChiselVolume
        {
            Air,
            Solid
        };
    }
    using ChiselVolume = ChiselVolumes::ChiselVolume;

    struct VertexCSG
    {
        vec3 position;
        vec3 normal;
    };

    static VertexLayout LayoutCSG = VertexLayout {
        VertexAttribute::For<float>(3, VertexAttribute::Position),
        VertexAttribute::For<float>(3, VertexAttribute::Normal, true),
    };

    class Primitive
    {
    public:
        Primitive(CSG::CSGTree* tree, ChiselVolume volume, const CSG::Matrix4& transform = CSG::Matrix4{1.0f})
            : m_brush(tree->CreateBrush())
            , m_transform(transform)
        {
            m_brush.SetVolumeOperation(CSG::CreateFillOperation(volume));
            m_brush.Userdata = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));
        }

        ~Primitive()
        {
            m_brush.GetTree()->DestroyBrush(m_brush);
        }

        void SetTransform(const CSG::Matrix4& mat)
        {
            m_transform = mat;
            this->UpdatePlanes();
        }

        const CSG::Matrix4& GetTransform() const
        {
            return m_transform;
        }

        virtual void UpdatePlanes() = 0;

        void UpdateMesh()
        {
            m_verts.clear();
            m_indices.clear();

            for (auto& face : m_brush.GetFaces())
            {
                for (auto& fragment : face.fragments)
                {
                    if (fragment.back.volume == fragment.front.volume)
                        continue;

                    const bool flipFace = fragment.back.volume == ChiselVolumes::Air;
                    const CSG::Vector3 normal = flipFace
                        ? -face.plane->normal
                        :  face.plane->normal;

                    const size_t startIndex = m_verts.size();
                    for (const auto& vert : fragment.vertices)
                        m_verts.emplace_back(vert.position, normal);

                    std::vector<CSG::TriangleIndices> tris = fragment.Triangulate();
                    for (const auto& tri : tris)
                    {
                        if (flipFace)
                        {
                            m_indices.push_back(startIndex + tri[0]);
                            m_indices.push_back(startIndex + tri[2]);
                            m_indices.push_back(startIndex + tri[1]);
                        }
                        else
                        {
                            m_indices.push_back(startIndex + tri[0]);
                            m_indices.push_back(startIndex + tri[1]);
                            m_indices.push_back(startIndex + tri[2]);
                        }
                        /*fprintf(stderr, "Adding triangle: (%g %g %g), (%g %g %g), (%g %g %g)\n",
                            m_verts[startIndex + tri[0]].position.x, m_verts[startIndex + tri[0]].position.y, m_verts[startIndex + tri[0]].position.z,
                            m_verts[startIndex + tri[1]].position.x, m_verts[startIndex + tri[1]].position.y, m_verts[startIndex + tri[1]].position.z,
                            m_verts[startIndex + tri[2]].position.x, m_verts[startIndex + tri[2]].position.y, m_verts[startIndex + tri[2]].position.z);*/
                    }
                }
            }
            printf("UPDATING MESH: %u\n", uint32_t(m_indices.size()));
            m_mesh = Mesh(LayoutCSG, m_verts, m_indices);
        }

        Mesh* GetMesh()
        {
            return &m_mesh;
        }
    protected:
        CSG::Brush& m_brush;
        CSG::Matrix4 m_transform;

        Mesh m_mesh;
        std::vector<VertexCSG> m_verts;
        std::vector<uint32_t> m_indices;
    };

    class CubePrimitive final : public Primitive
    {
    public:
        CubePrimitive(CSG::CSGTree* tree, ChiselVolume volume, const CSG::Matrix4& transform = CSG::Matrix4{1.0f})
            : Primitive(tree, volume, transform)
        {
            this->UpdatePlanes();
        }

        void UpdatePlanes() override
        {
            static const std::array<CSG::Plane, 6> kUnitCubePlanes =
            {
                CSG::Plane(CSG::Vector3(+1,0,0), CSG::Vector3(+1,0,0)),
                CSG::Plane(CSG::Vector3(-1,0,0), CSG::Vector3(-1,0,0)),
                CSG::Plane(CSG::Vector3(0,+1,0), CSG::Vector3(0,+1,0)),
                CSG::Plane(CSG::Vector3(0,-1,0), CSG::Vector3(0,-1,0)),
                CSG::Plane(CSG::Vector3(0,0,+1), CSG::Vector3(0,0,+1)),
                CSG::Plane(CSG::Vector3(0,0,-1), CSG::Vector3(0,0,-1))
            };

            std::array<CSG::Plane, 6> planes;
            for (size_t i = 0; i < 6; i++)
                planes[i] = kUnitCubePlanes[i].Transformed(m_transform);

            m_brush.SetPlanes(planes.begin(), planes.end());
        }
    };

    // TODO: Make this a RenderPipeline
    struct MapRender : public System
    {
    private:
        static inline VertexLayout xyz = {
            VertexAttribute::For<float>(3, VertexAttribute::Position),
            VertexAttribute::For<float>(3, VertexAttribute::Normal, true),
        };

        CSG::CSGTree world;
    public:

        render::Render& r = Tools.Render;
        render::Shader* shader;

        CubePrimitive* tunnel = nullptr;
        CubePrimitive* tunnel2 = nullptr;

        void Start() final override
        {
            shader = Tools.Render.LoadShader("flat");

            world.SetVoidVolume(ChiselVolumes::Air);

            new CubePrimitive(&world, ChiselVolumes::Solid);
            new CubePrimitive(&world, ChiselVolumes::Air, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(0.25, 2.0, 0.25)));
            tunnel = new CubePrimitive(&world, ChiselVolumes::Air, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(2.0, 0.25f, 0.25)));
            tunnel2 = new CubePrimitive(&world, ChiselVolumes::Solid, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(2.0, 0.10f, 0.10)));
        }

        void Update() final override
        {
            r.SetClearColor(true, Color(0.2, 0.2, 0.2));
            r.SetClearDepth(true, 1.0f);
            r.SetRenderTarget(Tools.rt_SceneView);
            r.SetShader(shader);
            r.SetTransform(glm::identity<mat4x4>());

            /*if (tunnel && tunnel2)
            {
                static const CSG::Matrix4 tunnelOrigTransform = tunnel->GetTransform();
                static const CSG::Matrix4 tunnel2OrigTransform = tunnel2->GetTransform();
                float time = 0.0f;//Time::GetTime();
                tunnel->SetTransform(
                    glm::rotate(CSG::Matrix4(1), glm::radians(time), CSG::Vector3(0,1,0)) *
                    tunnelOrigTransform
                );
                tunnel2->SetTransform(
                    glm::rotate(CSG::Matrix4(1), glm::radians(time), CSG::Vector3(0,1,0)) *
                    tunnel2OrigTransform
                );
            }*/

            auto rebuilt = world.Rebuild();
            for (CSG::Brush* brush : rebuilt)
                brush->GetUserdata<Primitive*>()->UpdateMesh();

            // TODO: Cull!
            for (const CSG::Brush& brush : world.GetBrushes())
                r.DrawMesh(brush.GetUserdata<Primitive*>()->GetMesh());
        }
    };

}