#pragma once

#include "core/Mesh.h"
#include "glm/ext/matrix_transform.hpp"
#include "math/Math.h"
#include "math/Color.h"
#include "render/Render.h"
#include "entity/components/Transform.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <vector>

namespace engine::editor
{
    inline struct Handles
    {
    private:
        static inline Mesh grid;
        static inline std::vector<vec4> gridVertices;

    public:
        enum class Tool {
            Translate, Rotate, Scale, Universal
        };
        
    // ImGuizmo //
        void Begin(const Rect& viewport, bool allowAxisFlip)
        {
            ImGuizmo::BeginFrame();
            ImGuizmo::AllowAxisFlip(allowAxisFlip);
            ImGuizmo::SetRect(viewport.x, viewport.y, viewport.w, viewport.h);
            ImGuizmo::SetDrawlist();
        }

        void Maniuplate(Transform& transform, const mat4x4& view, const mat4x4& proj, Tool activeTool, Space space, bool gridSnap, const vec3& gridSize)
        {
            float mtx[16];
            vec3 angles = transform.GetEulerAngles();
            ImGuizmo::RecomposeMatrixFromComponents(&transform.position[0], &angles[0], &transform.scale[0], mtx);

            ImGuizmo::MODE mode = space == Space::World ? ImGuizmo::MODE::WORLD : ImGuizmo::MODE::LOCAL;
            ImGuizmo::Manipulate(&view[0][0], &proj[0][0], GetOperation(activeTool), mode, &mtx[0], NULL, gridSnap ? &gridSize.x : NULL);
            
            ImGuizmo::DecomposeMatrixToComponents(mtx, &transform.position[0], &angles[0], &transform.scale[0]);
            transform.SetEulerAngles(angles);
        }
        
        bool IsMouseOver() { return ImGuizmo::IsOver(); }
        
        void ViewManiuplate(const Rect& viewport, mat4x4& view, float length = 35.f, float size = 128.f, Color color = Colors.Transparent)
        {
            ImGuizmo::ViewManipulate(&view[0][0], length, ImVec2(viewport.x, viewport.y), ImVec2(size, size), color.Pack<std::endian::little>());
        }
        
        void DrawGrid2(const mat4x4& view, const mat4x4& proj, float gridSize = 100)
        {
            static mat4x4 grid = glm::identity<mat4x4>();
            ImGuizmo::DrawGrid(&view[0][0], &proj[0][0], &grid[0][0], gridSize);
        }
        
        void DrawTestCube(const mat4x4& view, const mat4x4& proj)
        {
            static mat4x4 cube = glm::identity<mat4x4>();
            ImGuizmo::DrawCubes(&view[0][0], &proj[0][0], &cube[0][0], 1);
        }
    
    // Grid //

        // TODO: Scale grid based on snap increment. View based fade. Infinite grid
        void DrawGrid(render::Render& r, render::Shader* shader)
        {
            mat4x4 matrix = glm::translate(glm::identity<mat4x4>(), vec3(0, 0.015, 0));
            r.SetTransform(matrix);
            r.SetBlendFunc(render::BlendFuncs::Alpha);
            r.SetDepthTest(render::CompareFunc::LessEqual);
            r.SetPrimitiveType(render::PrimitiveType::Lines);
            r.SetShader(shader);
            r.DrawMesh(&grid);
            r.SetPrimitiveType(render::PrimitiveType::Triangles);
            r.SetDepthTest(render::CompareFunc::Less);
            r.SetBlendFunc(render::BlendFuncs::Normal);
        }

        Handles()
        {
            int gridSize = 100;
            int radius = gridSize / 2;
            int gridMajor = 10;
            gridVertices.resize((gridSize + 1) * 4);

            uint i = 0;
            for (int x = -radius; x <= radius; x++, i += 2)
            {
                gridVertices[i] = vec4(x, 0, -radius, x % gridMajor == 0);
                gridVertices[i+1] = vec4(x, 0, +radius, x % gridMajor == 0);
            }

            for (int z = -radius; z <= radius; z++, i += 2)
            {
                gridVertices[i] = vec4(-radius, 0, z, z % gridMajor == 0);
                gridVertices[i+1] = vec4(+radius, 0, z, z % gridMajor == 0);
            }

            auto& g = grid.AddGroup();
            g.vertices.layout.Add<float>(3, VertexAttribute::Position);
            g.vertices.layout.Add<float>(1, VertexAttribute::TexCoord);
            g.vertices.pointer = &gridVertices[0].x; g.vertices.count = gridVertices.size();
        }
        
    private:
    
        static ImGuizmo::OPERATION GetOperation(Tool tool)
        {
            using enum ImGuizmo::OPERATION;
            switch (tool) {
                default:
                case Tool::Translate: return TRANSLATE;
                case Tool::Rotate:    return ROTATE;
                case Tool::Scale:     return SCALE;
                case Tool::Universal: return UNIVERSAL;
            }
        }
    
    } Handles;
}