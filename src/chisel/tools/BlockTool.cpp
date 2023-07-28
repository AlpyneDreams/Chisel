#include "PlacementTool.h"
#include "TransformTool.h"
#include "chisel/Chisel.h"
#include "chisel/Handles.h"
#include "chisel/Gizmos.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Viewport.h"

namespace chisel
{
    using BoundsTool = TransformTool<TransformType::Bounds>;

    struct BlockTool final : public DragTool, public BoundsTool
    {
        BlockTool() : DragTool("Block", ICON_MC_CUBE_OUTLINE, 101) {}

        virtual void DrawHandles(Viewport& viewport) override;
        virtual bool HasPropertiesGUI() override { return true; }
        virtual void DrawPropertiesGUI() override;
        virtual void OnMouseOver(Viewport& viewport, vec3 point, vec3 normal) override;
        virtual void OnFinishDrag(Viewport& viewport, vec3 point, vec3 normal) override;

        PrimitiveType type = PrimitiveType::Block;
    };

    static BlockTool Instance;

    void BlockTool::DrawHandles(Viewport& viewport)
    {
        mat4x4 view = viewport.camera.ViewMatrix();
        mat4x4 proj = viewport.camera.ProjMatrix();

        // User can edit block bounds while adding blocks
        BoundsTool::DrawHandles(viewport);
        if (Handles.IsMouseOver())
            return;
        
        PlacementTool::DrawHandles(viewport);
    }

    void BlockTool::DrawPropertiesGUI()
    {
        static const int numPrimitiveTypes = 9;
        static const char* primitiveTypeNames[] = {
            ICON_MC_CUBE_OUTLINE   " Block",
            ICON_MC_SQUARE_OUTLINE " Quad",
            ICON_MC_STAIRS         " Stairs",
            ICON_MC_VECTOR_RADIUS  " Arch",     // TODO: Better icon
            ICON_MC_CYLINDER       " Cylinder",
            ICON_MC_SPHERE         " Sphere",
            ICON_MC_CONE           " Cone",     // AKA Spike
            ICON_MC_CIRCLE_DOUBLE  " Torus",
            ICON_MC_SLOPE_UPHILL   " Wedge",    // TODO: Better icon
        };

        if (ImGui::BeginCombo("Primitive Type", primitiveTypeNames[int(type)]))
        {
            for (int i = 0; i < numPrimitiveTypes; i++)
            {
                PrimitiveType shape = PrimitiveType(i);
                bool selected = type == shape;
                if (ImGui::Selectable(primitiveTypeNames[int(shape)], selected))
                    type = shape;
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    void BlockTool::OnMouseOver(Viewport& viewport, vec3 point, vec3 normal)
    {
        DragTool::OnMouseOver(viewport, point, normal);
        
        if (viewport.draggingBlock)
        {
            vec3 direction = point - viewport.dragStartPos;
            vec3 corner1 = vec3(point.x, viewport.dragStartPos.y, point.z);
            vec3 corner2 = vec3(viewport.dragStartPos.x, point.yz);

            Gizmos.DrawPoint(viewport.dragStartPos);
            Gizmos.DrawPoint(corner1);
            Gizmos.DrawPoint(corner2);
            Gizmos.DrawLine(viewport.dragStartPos, corner1);
            Gizmos.DrawLine(viewport.dragStartPos, corner2);
            Gizmos.DrawLine(corner1, point);
            Gizmos.DrawLine(corner2, point);
        }
    }

    void BlockTool::OnFinishDrag(Viewport& viewport, vec3 point, vec3 normal)
    {
        vec3 vec = glm::abs(point - viewport.dragStartPos);
        vec3 center = (viewport.dragStartPos + point) / 2.f;
        vec3 extrude = vec3(0);
        // If we have a degenerate axis, extrude by normal by 1 grid size.
        if (math::CloseEnough(vec.x, 0) || math::CloseEnough(vec.y, 0) || math::CloseEnough(vec.z, 0))
            extrude = normal * view_grid_size.value;
        mat4x4 mtx = glm::translate(mat4x4(1), vec3(center.xyz) + (extrude * 0.5f));
        vec3 size = (vec3(vec.xyz) + extrude) * 0.5f;
        // make sure we are not degenerate size.
        bool degenerate = math::CloseEnough(size.x, 0.0f) || math::CloseEnough(size.y, 0.0f) || math::CloseEnough(size.z, 0.0f);
        if (!degenerate)
        {
            Chisel.map.Actions().PerformAction("Add Cube",
                // Do
                [this, solids = CreateCubeBrush(Chisel.activeMaterial.ptr(), size, mtx)](
                    std::any& userdata)
                {
                    auto& cube = Chisel.map.AddBrush(solids);
                    Selection.Clear();
                    Selection.Select(&cube);
                    userdata = &cube;
                },
                // Undo
                [ this ] (std::any& userdata)
                {
                    Solid *cube = std::any_cast<Solid*>(userdata);
                    cube->Delete();
                });
        }
    }
}
