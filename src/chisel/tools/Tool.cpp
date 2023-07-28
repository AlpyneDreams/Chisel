#include "Tool.h"
#include "console/ConVar.h"
#include "gui/Common.h"

namespace chisel
{
    static ConVar<float> gui_tool_properties_opacity("gui_tool_properties_opacity", 0.7f, "Opacity of the tool properties window.");

    Tool::Tool(const char* name, const char* icon, uint order)
        : name(name), icon(icon), order(order)
    {
        Tools.insert(this);
    }

    void Tool::DrawPropertiesWindow(Rect viewport, uint instance)
    {
        if (!this->HasPropertiesGUI())
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
            DrawPropertiesGUI();
        }
        ImGui::End();

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        // Keep this window always on top of viewport
        if (ImGui::IsWindowFocused())
            ImGui::BringWindowToDisplayFront(window);
    }
}
