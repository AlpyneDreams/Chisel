#pragma once

#include <list>

#include "core/Mesh.h"
#include "../CSG/Brush.h"
#include "../CSG/CSGTree.h"

#include "Common.h"
#include "Brush.h"

namespace chisel
{
    struct Entity : Atom {};

    struct PointEntity : Entity {};

    struct BrushEntity : Entity
    {
    protected:
        std::list<Brush>  brushes;
        CSG::CSGTree        tree;

    public:
        // TODO: This could be faster. Maybe use an indexed free list.
        Brush* GetBrush(auto id)
        {
            for (Brush& brush : brushes)
                if (brush.GetObjectID() == id)
                    return &brush;
            return nullptr;
        }

        Brush& AddBrush(Volume volume = Volumes::Solid)
        {
            return brushes.emplace_back(tree.CreateBrush(), volume);
        }

        Brush& AddCube(Volume volume = Volumes::Solid, mat4x4 transform = glm::identity<mat4x4>())
        {
            brushes.push_back(CubeBrush(tree.CreateBrush(), volume, transform));
            return brushes.back();
        }

        void RemoveBrush(const Brush& brush)
        {
            brushes.remove(brush);
        }

        void Rebuild()
        {
            auto rebuilt = tree.Rebuild();
            for (CSG::Brush* brush : rebuilt)
                brush->GetUserdata<Brush*>()->UpdateMesh();
        }

        auto begin() { return brushes.begin(); }
        auto end() { return brushes.end(); }
    };

    /**
     * Represents the entire map document, world brushes, and entities.
     */
    struct Map : BrushEntity
    {
        vec3                gridSize = vec3(64.0f);
        std::list<Atom>   atoms;

        Map()
        {
            tree.SetVoidVolume(Volumes::Air);

            //brushes.push_back(CubeBrush(tree.CreateBrush(), Volumes::Solid));
            //brushes.push_back(CubeBrush(tree.CreateBrush(), Volumes::Air, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(0.25, 2.0, 0.25))));
            //brushes.push_back(CubeBrush(tree.CreateBrush(), Volumes::Air, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(2.0, 0.25f, 0.25))));
            //brushes.push_back(CubeBrush(tree.CreateBrush(), Volumes::Solid, glm::scale(CSG::Matrix4(1.0), CSG::Vector3(2.0, 0.10f, 0.10))));

            // Per-frame test
            /*if (tunnel && tunnel2)
            {
                static const CSG::Matrix4 tunnelOrigTransform = tunnel->GetTransform();
                static const CSG::Matrix4 tunnel2OrigTransform = tunnel2->GetTransform();
                float time = 0.0f;//Time::GetTime();
                tunnel->SetTransform(
                    glm::rotate(CSG::Matrix4(1), math::radians(time), CSG::Vector3(0,1,0)) *
                    tunnelOrigTransform
                );
                tunnel2->SetTransform(
                    glm::rotate(CSG::Matrix4(1), math::radians(time), CSG::Vector3(0,1,0)) *
                    tunnel2OrigTransform
                );
            }*/

        }
    };
}