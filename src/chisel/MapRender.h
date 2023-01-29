#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "chisel/Chisel.h"
#include "common/System.h"
#include "chisel/Tools.h"
#include "chisel/Selection.h"
#include "chisel/Handles.h"

#include "core/Primitives.h"
#include "core/VertexLayout.h"
#include "common/Time.h"
#include "math/Math.h"
#include "math/Color.h"
#include "VMF/VMF.h"
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

    typedef struct {
        double r;
        double g;
        double b;
    } rgb;

    typedef struct {
        double h;
        double s;
        double v;
    } hsv;

    rgb hsv2rgb(hsv HSV)
    {
        rgb RGB;
        double H = HSV.h, S = HSV.s, V = HSV.v,
                P, Q, T,
                fract;
        (H == 360.)?(H = 0.):(H /= 60.);
        fract = H - floor(H);
        P = V*(1. - S);
        Q = V*(1. - S*fract);
        T = V*(1. - S*(1. - fract));
        if      (0. <= H && H < 1.)
            RGB = rgb{V, T, P};
        else if (1. <= H && H < 2.)
            RGB = rgb{Q, V, P};
        else if (2. <= H && H < 3.)
            RGB = rgb{P, V, T};
        else if (3. <= H && H < 4.)
            RGB = rgb{P, Q, V};
        else if (4. <= H && H < 5.)
            RGB = rgb{T, P, V};
        else if (5. <= H && H < 6.)
            RGB = rgb{V, P, Q};
        else
            RGB = rgb{0., 0., 0.};
        return RGB;
    }

    class Primitive
    {
    public:
        Primitive(CSG::CSGTree* tree, ChiselVolume volume)
            : m_brush(tree->CreateBrush())
        {
            m_brush.SetVolumeOperation(CSG::CreateFillOperation(volume));
            m_brush.Userdata = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));

            srand(m_brush.GetObjectID());
            rgb tempcolor = hsv2rgb(hsv{ (float)(rand() % 360), 0.7f, 1.0f } );
            m_tempcolor = vec4(tempcolor.r, tempcolor.g, tempcolor.b, 1.0f);
        }

        ~Primitive()
        {
            m_brush.GetTree()->DestroyBrush(m_brush);
        }

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

        CSG::Brush& GetBrush()
        {
            return m_brush;
        }

        Mesh* GetMesh()
        {
            return &m_mesh;
        }

        // TODO: Remove me, debugging.
        glm::vec4 GetTempColor() const
        {
            return m_tempcolor;
        }
    protected:
        CSG::Brush& m_brush;
        glm::vec4 m_tempcolor;

        Mesh m_mesh;
        std::vector<VertexCSG> m_verts;
        std::vector<uint32_t> m_indices;
    };

    class CubePrimitive final : public Primitive
    {
    public:
        CubePrimitive(CSG::CSGTree* tree, ChiselVolume volume, const CSG::Matrix4& transform = CSG::Matrix4{1.0f})
            : Primitive(tree, volume)
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
                planes[i] = kUnitCubePlanes[i].Transformed(transform);

            m_brush.SetPlanes(&planes.front(), &planes.back() + 1);
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

            std::string buffer{};
            glz::write<glz::opts{.format = glz::json, .prettify = false}>(world, buffer);
            fprintf(stderr, "%s\n", buffer.c_str());
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

            auto& brushes = world.GetBrushes();

            // TODO: Cull!
            for (const CSG::Brush& brush : brushes)
            {
                Primitive* primitive = brush.GetUserdata<Primitive*>();
                r.SetUniform("u_color", primitive->GetTempColor());
                
                if (brush.GetObjectID() == Selection.Active())
                {
                    // Draw a wire box around the brush
                    r.SetTransform(glm::identity<mat4x4>());
                    Tools.DrawSelectionOutline(&Primitives.Cube);
                    
                    // Draw wireframe of the brush's mesh
                    r.SetTransform(glm::identity<mat4x4>());
                    Tools.DrawSelectionOutline(primitive->GetMesh());
                    
                    // Draw the actual mesh faces in red
                    r.SetUniform("u_color", Color(1, 0, 0));
                }

                r.DrawMesh(primitive->GetMesh());
            }
        }

        void DrawSelectionPass()
        {
            // TODO: Cull!
            for (const CSG::Brush& brush : world.GetBrushes())
            {
                Primitive* primitive = brush.GetUserdata<Primitive*>();
                Tools.PreDrawSelection(r, brush.GetObjectID());
                r.DrawMesh(primitive->GetMesh());
            }
        }
        
        void DrawHandles(mat4x4& view, mat4x4& proj, auto... args)
        {
            if (!Selection.Any())
                return;
            
            SelectionID id = Selection.Active();
            
            if (CSG::Brush* brush = world.GetBrush(CSG::ObjectID(id)))
            {
                auto bounds = brush->GetBounds();
                if (!bounds)
                    return;

                // Get AABB center for translate for gizmo, then un-apply that
                // translation when we get it out the gizmo.
                auto mtx     = glm::translate(CSG::Matrix4(1.0), bounds->Center());
                auto inv_mtx = glm::translate(CSG::Matrix4(1.0), -bounds->Center());
                if (Handles.Manipulate(mtx, view, proj, args...))
                {
                    brush->Transform(mtx * inv_mtx);
                }
            }
        }
    };

}