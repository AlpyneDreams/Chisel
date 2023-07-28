#include "PlacementTool.h"
#include "chisel/Chisel.h"
#include "chisel/MapRender.h"
#include "gui/Viewport.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Inspector.h"

namespace chisel
{
    struct EntityTool final : public PlacementTool
    {
        EntityTool() : PlacementTool("Entity", ICON_MC_LIGHTBULB, 100) {}
        virtual bool HasPropertiesGUI() override { return true; }
        virtual void DrawPropertiesGUI() override;
        virtual void OnMouseOver(Viewport& viewport, vec3 point, vec3 normal) override;
        virtual void OnClick(Viewport& viewport, vec3 point, vec3 normal) override;

        std::string className = "info_player_start";
        bool        randomYaw = false;
    };

    static EntityTool Instance;

    void EntityTool::DrawPropertiesGUI()
    {
        // TODO: Prefabs & instances mode
        Inspector::ClassnamePicker(&className, false, "Entity Type");
        ImGui::Checkbox("Random Yaw", &randomYaw);
    }

    void EntityTool::OnMouseOver(Viewport& viewport, vec3 point, vec3 normal)
    {
        // Draw hypothetical entity
        Chisel.Renderer->DrawPointEntity(className, true, point);
    }

    void EntityTool::OnClick(Viewport& viewport, vec3 point, vec3 normal)
    {
        // Place entity on click
        PointEntity* pt = Chisel.map.AddPointEntity(className.c_str());
        pt->origin = point;
        Selection.Clear();
        Selection.Select(pt);
    }
}
