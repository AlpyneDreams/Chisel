#pragma once

#include "core/Mesh.h"
#include "../CSG/Brush.h"
#include "../CSG/CSGTree.h"

#include "math/Color.h"

#include "Common.h"

#include <memory>

namespace chisel
{
    struct Brush : Atom
    {
    protected:
        CSG::Brush*             brush;
        Volume                  volume;

        MeshBuffer<VertexCSG>   mesh = MeshBuffer<VertexCSG>();     
        Color                   tempcolor;

    public:
        Brush(CSG::Brush& brush, Volume volume)
            : brush(&brush), volume(volume)
        {
            brush.SetVolumeOperation(CSG::CreateFillOperation(volume));
            brush.Userdata = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));

            srand(brush.GetObjectID());
            tempcolor = Color::HSV((float)(rand() % 360), 0.7f, 1.0f);
        }

        Brush(Brush&& that)
            : Brush(*that.brush, that.volume)
        {
            that.brush = nullptr;
        }
        
        ~Brush()
        {
            if (brush)
                brush->GetTree()->DestroyBrush(*brush);
        }

        bool operator ==(const Brush& that) const
        {
            return brush == that.brush
                && brush != nullptr
                && that.brush != nullptr;
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
                        ? -face.plane->normal
                        :  face.plane->normal;

                    const size_t startIndex = mesh.vertices.size();
                    for (const auto& vert : fragment.vertices)
                        mesh.vertices.emplace_back(vert.position, normal);

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

        CSG::ObjectID GetObjectID() const { return brush->GetObjectID(); }
        CSG::Brush& GetBrush() { return *brush; }

        Mesh* GetMesh() { return &mesh; }

        // TODO: Remove me, debugging.
        glm::vec4 GetTempColor() const { return tempcolor; }
    };

    Brush CubeBrush(CSG::Brush& brush, Volume volume, const mat4x4& transform = glm::identity<mat4x4>())
    {
        Brush cube = Brush(brush, volume);
        
        static const std::array<CSG::Plane, 6> kUnitCubePlanes =
        {
            CSG::Plane(vec3(+1,0,0), vec3(+1,0,0)),
            CSG::Plane(vec3(-1,0,0), vec3(-1,0,0)),
            CSG::Plane(vec3(0,+1,0), vec3(0,+1,0)),
            CSG::Plane(vec3(0,-1,0), vec3(0,-1,0)),
            CSG::Plane(vec3(0,0,+1), vec3(0,0,+1)),
            CSG::Plane(vec3(0,0,-1), vec3(0,0,-1))
        };
        
        std::array<CSG::Plane, 6> planes;
        for (size_t i = 0; i < 6; i++)
            planes[i] = kUnitCubePlanes[i].Transformed(glm::scale(transform, vec3(64.f)));
        
        cube.GetBrush().SetPlanes(&planes.front(), &planes.back() + 1);
        
        return cube;
    }
}