#include "console/ConVar.h"
#include "chisel/Chisel.h"
#include "gui/ToolProperties.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Inspector.h"

#include <imgui.h>

namespace chisel
{
    extern ConVar<ClipType> tool_clip_type;
}

namespace chisel::GUI
{
    static ConVar<float> gui_tool_properties_opacity("gui_tool_properties_opacity", 0.7f, "Opacity of the tool properties window.");

    void ToolPropertiesWindow(Tool tool, Rect viewport, uint instance)
    {
        if (tool != Tool::Entity && tool != Tool::Block && tool != Tool::Clip)
            return;

        ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoMove
              | ImGuiWindowFlags_NoResize
              | ImGuiWindowFlags_NoScrollbar
              | ImGuiWindowFlags_NoDocking
              | ImGuiWindowFlags_AlwaysAutoResize
              | ImGuiWindowFlags_NoFocusOnAppearing
              | ImGuiWindowFlags_NoSavedSettings;

        constexpr float padding = 10.0f;
        ImVec2 pos;
        pos.x = viewport.x + padding;
        pos.y = viewport.y + viewport.h - padding;
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 1.0f));

        uint32 color = Color255(15, 15, 15, gui_tool_properties_opacity * 255).PackABGR();
        ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
        ImGui::PushStyleColor(ImGuiCol_TitleBg, color);
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, color);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGuiWindow* window;

        if (ImGui::Begin((std::string("Tool Properties##") + std::to_string(instance)).c_str(), nullptr, flags))
        {
            window = ImGui::GetCurrentWindow();
            ToolProperties(tool);
        }
        ImGui::End();

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        // Keep this window always on top of viewport
        if (ImGui::IsWindowFocused())
            ImGui::BringWindowToDisplayFront(window);
    }

    void ToolProperties(Tool tool)
    {
        using enum Tool;
        switch (tool)
        {
            default: break;

            // TODO: Prefabs & instances mode
            case Entity:
            {
                Inspector::ClassnamePicker(&Chisel.entTool.className, false, "Entity Type");
                ImGui::Checkbox("Random Yaw", &Chisel.entTool.randomYaw);
                break;
            }
            case Clip:
            {
                static const char *buttonNames[] = {
                    "Front",
                    "Back",
                    "Keep Both"
                };
                ImGui::Text("Clipping Mode");
                for (int i = 0; i < 3; i++)
                    ImGui::RadioButton(buttonNames[i], (int*)&tool_clip_type.value, i);
                break;
            }
            case Block:
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

                if (ImGui::BeginCombo("Primitive Type", primitiveTypeNames[int(Chisel.blockTool.type)]))
                {
                    for (int i = 0; i < numPrimitiveTypes; i++)
                    {
                        PrimitiveType type = PrimitiveType(i);
                        bool selected = Chisel.blockTool.type == type;
                        if (ImGui::Selectable(primitiveTypeNames[int(type)], selected))
                            Chisel.blockTool.type = type;
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                break;
            }
        }
    }
}