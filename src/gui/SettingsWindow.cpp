#include "SettingsWindow.h"
#include "chisel/Settings.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Common.h"
#include "gui/Modal.h"
#include "platform/Platform.h"
#include "console/Console.h"

#include <misc/cpp/imgui_stdlib.h>

namespace chisel
{
    SettingsWindow::SettingsWindow() : GUI::Window(ICON_MC_COG, "Settings", 512, 512, false)
    {
    }

    void SettingsWindow::Draw()
    {
        using namespace ImGui;

        static bool popup = false;

        if (Button("Reset All Settings", ImVec2(-FLT_MIN, 0)) && !popup)
        {
            Console.Log("Opening popup...");
            popup = true;
            OpenPopup("Reset Settings");
        }

        if (popup)
        {
            enum { Waiting = -1, NoPopup, Yes = 1, No };
            int choice = GUI::ModalChoices(
                "Reset Settings",
                "Are you sure you want to reset ALL of Chisel's settings to their defaults?",
                "Reset", "Cancel");

            if (choice == Yes)
            {
                Settings::Reset();
                Settings::Save();
                popup = false;
            }
        }

        TextUnformatted("Search Paths");

        static int selected = -1;
        PushFont(GUI::FontMonospace);
        if (BeginListBox("##Search Paths", ImVec2(-FLT_MIN, 0)))
        {
            int i = 0;
            bool clickedItem = false;
            for (auto& path : Settings::SearchPaths)
            {
                bool isSelected = selected == i;
                if (Selectable(path.c_str(), isSelected))
                {
                    clickedItem = true;
                    selected = i;
                }

                if (isSelected)
                    SetItemDefaultFocus();
                
                i++;
            }

            EndListBox();

            // Deselect if clicked in the empty space on the list box
            if (!clickedItem && Mouse.GetButtonUp(Mouse.Left))
            {
                vec2 pos = vec2(GetMousePos().x, GetMousePos().y);
                vec2 mins = vec2(GetItemRectMin().x, GetItemRectMin().y);
                vec2 maxs = vec2(GetItemRectMax().x, GetItemRectMax().y);
                if (glm::all(glm::lessThanEqual(pos, maxs)) && glm::all(glm::greaterThanEqual(pos, mins)))
                    selected = -1;
            }
        }
        PopFont();

        if (Button("Add"))
        {
            if (std::string path = Platform.FolderPicker(); !path.empty())
            {
                Settings::SearchPaths.push_back(path);
                Settings::Save();
            }
        }

        bool disabled = false;
        if (selected < 0)
            disabled = (BeginDisabled(), true);

        SameLine();
        if (Button("Remove"))
        {
            Settings::SearchPaths.erase(Settings::SearchPaths.begin() + selected);
            Settings::Save();
            if (selected > 0)
                selected--;
            else if (selected < Settings::SearchPaths.size() - 1)
                selected++;
            else
                selected = -1;
        }

        SameLine();
        if (Button(ICON_MC_TRIANGLE) && selected > 0)
        {
            std::swap(Settings::SearchPaths[selected], Settings::SearchPaths[selected - 1]);
            Settings::Save();
            selected--;
        }

        SameLine();
        if (Button(ICON_MC_TRIANGLE_DOWN) && selected < Settings::SearchPaths.size() - 1)
        {
            std::swap(Settings::SearchPaths[selected], Settings::SearchPaths[selected + 1]);
            Settings::Save();
            selected++;
        }

        if (disabled)
            EndDisabled();
    }
}
