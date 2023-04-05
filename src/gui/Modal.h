#pragma once

#include <imgui.h>
#include <algorithm>

namespace chisel::GUI
{
    inline int ModalChoices(const char* title, const char* message, auto... choices)
    {
        using namespace ImGui;
        int choice = 0;
        int count = sizeof...(choices);

        ImVec2 center = GetMainViewport()->GetCenter();
        SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        SetNextWindowContentSize(ImVec2(60.f + (80.f * count), 0.f));

        if (BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            choice = -1;
            TextWrapped("%s\n\n", message);
            Separator();
            Spacing();

            SetItemDefaultFocus();
            PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.f, GetStyle().ItemSpacing.y));
            Indent(60.f);
            Columns(count, NULL, false);
            for (int i = 0; auto& button : { choices... })
            {
                if (i++, Button(button, ImVec2(-FLT_MIN, 0.f)))
                    choice = i;
                NextColumn();
            }
            PopStyleVar();

            if (choice > 0)
                CloseCurrentPopup();

            EndPopup();
        }

        return choice;
    }
}
