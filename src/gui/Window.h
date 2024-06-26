#pragma once

#include "common/Common.h"
#include "common/System.h"
#include "math/Math.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include <unordered_map>

namespace chisel::GUI
{
    struct Window : public System
    {
        const char* icon = "";
        const char* title = "";
        const char* id = "";
        uint instance = s_nextInstance.try_emplace(id, 0).first->second++;
        std::string name = "##"; // (unnamed)
        uint width = 512;
        uint height = 512;
        bool open = true;
        bool visible = false;
        ImGuiWindowFlags flags;

    public:

        Window() {}

        // Icon, Title, and ID
        Window(const char* icon, const char* title, const char* id, uint width, uint height, bool open = true, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
          : icon(icon), title(title), id(id)
          , name(std::string(icon) + " " + title + "###" + id + (!instance ? "" : std::to_string(instance)))
          , width(width), height(height), open(open), flags(flags) {}

        // Icon, Title == ID
        Window(const char* icon, const char* name, uint width, uint height, bool open = true, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
          : Window(icon, name, name, width, height, open, flags) {}

        // No Icon
        Window(const char* name, uint width, uint height, bool open = true, ImGuiWindowFlags flags = ImGuiWindowFlags_None)
          : title(name), id(name), name(std::string(name) + "###" + name + (!instance ? "" : std::to_string(instance)))
          , width(width), height(height), open(open), flags(flags) {}


        void Update() override
        {
            if (!open) {
                visible = false;
                return;
            }

            // Setup window variables
            uint2 contentSize = uint2(0, 0);
            if (OverrideContentSize(contentSize))
                ImGui::SetNextWindowContentSize(ImVec2(float(contentSize.x), float(contentSize.y)));
            ImGui::SetNextWindowSize(ImVec2(float(width), float(height)), ImGuiCond_FirstUseEver);
            PreDraw();

            if (visible = ImGui::Begin(name.c_str(), &open, flags))
            {
                m_window = ImGui::GetCurrentWindow();
                Draw();
            }

            PostDraw();
            ImGui::End();
        }

        bool IsMouseOver(const Rect& rect) const
        {
            return ImGui::IsMouseHoveringRect(
                ImVec2(rect.x, rect.y),
                ImVec2(rect.x + rect.w, rect.y + rect.h)
            );
        }

        // Focuses the window if it's hidden,
        // otherwise toggles it open/closed.
        void ToggleOrFocus()
        {
            if (visible) {
                open = false;
            } else {
                open = true;
                if (m_window)
                    ImGui::FocusWindow(m_window);
            }
        }

        // Subclasses should override this.
        virtual void Draw() {}

        // Subclasses can override these.
        // Unlike Draw, PreDraw and PostDraw run regardless of whether
        // the window is visible, so you should check this->visible in them.
        virtual void PreDraw() {}
        virtual void PostDraw() {}

        virtual bool OverrideContentSize(uint2& size) { return false; }

    private:
        ImGuiWindow* m_window = nullptr;
        static inline std::unordered_map<std::string, uint> s_nextInstance;
    };
}