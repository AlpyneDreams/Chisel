#pragma once

#include "Window.h"
#include <imgui.h>
#include <iostream>

#include "console/Console.h"
#include "console/ConCommand.h"
#include "console/ConVar.h"

#include "input/Input.h"

#include "gui/IconsMaterialCommunity.h"

namespace chisel::GUI
{
    struct ConsoleWindow : public Window
    {
        ConsoleWindow() : Window(ICON_MC_CONSOLE, "Console", 512, 512, false) {}

        bool scrollToBottom = false;
        bool focus = false;

        std::string currentInput = "";
        int historyPos = -1; // -1 = currentInput

        void Update() final override
        {
            if (Keyboard.GetKeyUp(Key::Grave)) {
                ToggleOrFocus();
                if (open)
                    focus = true;
            }
            Window::Update();
        }

        // Based on imgui_demo.cpp ExampleAppConsole
        void Draw() final override
        {
            ImGui::PushFont(GUI::FontMonospace);

            // Reserve enough left-over height for 1 separator + 1 input text
            const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

            // Print each log line
            for (auto& [level, str] : Console.log) {
                using Level = Console::Level;
                switch (level)
                {
                    case Level::Input:
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.6f, 1.0f), "%s", str.c_str());
                        break;
                    case Level::Warning:
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "%s", str.c_str());
                        break;
                    case Level::Error:
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", str.c_str());
                        break;
                    default:
                    case Level::Info:
                        ImGui::TextUnformatted(str.c_str());
                        break;
                }
            }

            // Auto-scroll to bottom
            if (scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            scrollToBottom = false;

            ImGui::EndChild();
            ImGui::Separator();

            constexpr auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;
            constexpr auto callback = [](ImGuiInputTextCallbackData* data) -> int {
                return ((ConsoleWindow*)data->UserData)->OnInput(data);
            };

            char command[1024] = "";
            if (ImGui::InputText("Command", command, 1024, flags, callback, (void*)this))
            {
                ImGui::SetKeyboardFocusHere(-1);
                scrollToBottom = true;
                currentInput = "";
                historyPos = -1;

                Console.Execute(command);
            }
            else if (focus)
            {
                ImGui::SetKeyboardFocusHere(-1);
                focus = false;
            }

            ImGui::PopFont();
        }

        int OnInput(ImGuiInputTextCallbackData* data)
        {
            switch (data->EventFlag)
            {
                case ImGuiInputTextFlags_CallbackHistory:
                {
                    int pos = historyPos;

                    if (data->EventKey == ImGuiKey_UpArrow)
                        pos = std::min(pos + 1, int(Console.history.size() - 1));

                    else if (data->EventKey == ImGuiKey_DownArrow)
                        pos = std::max(pos - 1, -1);

                    if (pos != historyPos)
                    {
                        // If moving away from current input, cache it
                        if (historyPos < 0) {
                            currentInput = data->Buf;
                        }

                        std::string& str = (pos >= 0) ? Console.history[Console.history.size() - pos - 1] : currentInput;

                        data->DeleteChars(0, data->BufTextLen);
                        data->InsertChars(0, str.c_str());
                        historyPos = pos;
                    }

                    break;
                }
                default:
                    break;
            }
            return 0;
        }
    };
}