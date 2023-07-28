#include "PlacementTool.h"
#include "chisel/Gizmos.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Viewport.h"
#include "math/Plane.h"

namespace chisel
{
    ConVar<ClipType> tool_clip_type("tool_clip_type", ClipType::Front, "Which clip mode we are in.");

    struct ClipTool final : public DragTool
    {
        ClipTool() : DragTool("Clip", ICON_MC_SQUARE_OFF_OUTLINE, 102) {}

        virtual bool HasPropertiesGUI() override { return true; }
        virtual void DrawPropertiesGUI() override;
        virtual void OnMouseOver(Viewport& viewport, vec3 point, vec3 normal) override;
        virtual void OnFinishDrag(Viewport& viewport, vec3 point, vec3 normal) override;

        Plane plane;
    };

    static ClipTool Instance;

    void ClipTool::DrawPropertiesGUI()
    {
        static const char *buttonNames[] = {
            "Front",
            "Back",
            "Keep Both"
        };
        ImGui::Text("Clipping Mode");
        for (int i = 0; i < 3; i++)
            ImGui::RadioButton(buttonNames[i], (int*)&tool_clip_type.value, i);
    }

    void ClipTool::OnMouseOver(Viewport& viewport, vec3 point, vec3 normal)
    {
        // Find the perpendicular plane
        if (viewport.draggingBlock)
        {
            vec3 direction = glm::normalize(point - viewport.dragStartPos);
            plane = Plane(point, glm::cross(direction, normal));
        }

        DragTool::OnMouseOver(viewport, point, normal);

        // Draw clip plane
        if (viewport.draggingBlock)
        {
            Gizmos.DrawPoint(viewport.dragStartPos);
            Gizmos.DrawPlane(plane, Color(0.0f, 1.0f, 1.0f, 0.1f), true);
            Gizmos.DrawPlane(plane, Color(1.0f, 0.0f, 0.0f, 0.1f), false);
        }

        // Reset plane
        plane = Plane();
    }

    void ClipTool::OnFinishDrag(Viewport& viewport, vec3 point, vec3 normal)
    {
        Side sides[2] = {{ plane, Chisel.activeMaterial, 0.25f }, { plane.Inverse(), Chisel.activeMaterial, 0.25f }};

        for (Selectable* selectable : Selection)
        {
            if (Solid* solid = dynamic_cast<Solid*>(selectable))
            {
                if (tool_clip_type == ClipType::KeepBoth)
                {
                    BrushEntity *parent = solid->GetParent();
                    assert(parent != nullptr);

                    std::vector<Side> newSides = solid->GetSides();
                    newSides.emplace_back(sides[1]);
                    Solid& newSolid = parent->AddBrush(std::move(newSides));
                    Selection.Select(&newSolid);
                }

                solid->Clip(tool_clip_type == ClipType::Back ? sides[1] : sides[0]);
                solid->UpdateMesh();
            }
        }
    }
}
