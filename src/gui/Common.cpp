#include "common/Common.h"
#include "Common.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "console/Console.h"
#include "console/ConVar.h"
#include "common/Filesystem.h"

#include "gui/IconsMaterialCommunity.h"

namespace chisel
{
    // Merge in icon font(s) - see gui/docs/FONTS.md
    // This process is expensive for MaterialCommunityIcons (~7000 icons)
    static void MergeIconFonts(float size)
    {
        ImFontConfig config;
        config.MergeMode = true;
        config.GlyphOffset = ImVec2(0, 1);
        config.GlyphMinAdvanceX = size;
        static const ImWchar icon_ranges[] = { ImWchar(ICON_MIN_MC), ImWchar(ICON_MAX_MC), ImWchar(0) };
        ImGui::GetIO().Fonts->AddFontFromFileTTF("core/fonts/" FONT_ICON_FILE_NAME_MC, size, &config, icon_ranges);
    }

    static ImFont* AddFontFile(const char* name, float size)
    {
        std::string path = std::string("core/fonts/") + name;
        if (!fs::exists(path)) {
            Console.Error("[GUI] Font file not found: " + path);
            return nullptr;
        }

        ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), size);
        return font;
    }

    inline ConCommand gui_reset("gui_reset", "Loads default UI layout", [](ConCmd& cmd) {
        //ImGui::LoadIniSettingsFromDisk("chisel_ui_default.ini");
        // HACK: Docking doesn't work properly unless settings are loaded between frames.
        fs::copyFile("chisel_ui_default.ini", "chisel_ui.ini");
        ImGuiContext* g = ImGui::GetCurrentContext();
        g->SettingsWindows.clear();
        g->SettingsLoaded = false;
    });

    inline ConCommand gui_save("gui_save", "Saves default UI layout", [](ConCmd& cmd) {
        ImGui::SaveIniSettingsToDisk("chisel_ui_default.ini");
    });

    void GUI::Setup()
    {
        ImGui::CreateContext();

        if (fs::exists("chisel_ui_default.ini")) {
            fs::copy("chisel_ui_default.ini", "chisel_ui.ini", fs::update_existing);
        }

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = "chisel_ui.ini";
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Click once to edit DragXXX sliders.
        io.ConfigDragClickToInputText = true;

        // Default font
        AddFontFile("Roboto-Regular.ttf", 15);

        // Add icons to default font
        MergeIconFonts(15);

        // Named fonts
        GUI::FontMonospace = AddFontFile("CascadiaMono-Regular.ttf", 15);
        GUI::FontDense     = AddFontFile("SourceSansPro-Regular.ttf", 16);

        //io.Fonts->AddFontDefault(); // ImGui Default: ProggyClean, 13px
        //AddFontFile("Cousine-Regular.ttf", 15);
        //AddFontFile("DroidSans.ttf", 15);
        //AddFontFile("Karla-Regular.ttf", 15);
        //AddFontFile("Roboto-Medium.ttf", 15);

        ImGuiStyle& style = ImGui::GetStyle();

        // Widget rounding
        style.FrameRounding = 4;
        style.GrabRounding = 4;
        style.TabRounding = 4;

        // Windows - keep it subtle because root viewport windows have no rounding
        style.WindowRounding = 6;
        style.ChildRounding = 6;
        style.PopupRounding = 6;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg]               = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    }

    void GUI::Present()
    {
        static ImGuiIO& io = ImGui::GetIO();

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    ImVec2 operator-(ImVec2 lhs, ImVec2 rhs) {
        return ImVec2{lhs.x - rhs.x, lhs.y - rhs.y};
    }

    // https://github.com/ocornut/gui/issues/3469#issuecomment-691845667
    void GUI::ItemLabel(std::string_view title, bool right)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImVec2 lineStart = ImGui::GetCursorScreenPos();
        const ImGuiStyle& style = ImGui::GetStyle();
        float fullWidth = ImGui::GetContentRegionAvail().x;
        float itemWidth = ImGui::CalcItemWidth() + style.ItemSpacing.x;
        ImVec2 textSize = ImGui::CalcTextSize(&title.front(), &title.back());
        ImRect textRect;
        textRect.Min = ImGui::GetCursorScreenPos();
        if (right)
            textRect.Min.x = textRect.Min.x + itemWidth;
        textRect.Max = textRect.Min;
        textRect.Max.x += fullWidth - itemWidth;
        textRect.Max.y += textSize.y;

        ImGui::SetCursorScreenPos(textRect.Min);

        ImGui::AlignTextToFramePadding();
        // Adjust text rect manually because we render it directly into a drawlist instead of using public functions.
        textRect.Min.y += window->DC.CurrLineTextBaseOffset;
        textRect.Max.y += window->DC.CurrLineTextBaseOffset;

        ImGui::ItemSize(textRect);
        if (ImGui::ItemAdd(textRect, window->GetID(title.data(), title.data() + title.size())))
        {
            ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), textRect.Min, textRect.Max, textRect.Max.x,
                textRect.Max.x, title.data(), title.data() + title.size(), &textSize);

            if (textRect.GetWidth() < textSize.x && ImGui::IsItemHovered())
                ImGui::SetTooltip("%.*s", (int)title.size(), title.data());
        }
        if (!right)
        {
            ImGui::SetCursorScreenPos(textRect.Max - ImVec2{0, textSize.y + window->DC.CurrLineTextBaseOffset});
            ImGui::SameLine();
        }
        else if (right)
            ImGui::SetCursorScreenPos(lineStart);
    }

    bool GUI::MenuBarButton(const char* title, const char* tooltip)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
        bool clicked = ImGui::Button(title);
        ImGui::PopStyleColor();
        if (tooltip && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tooltip);
        return clicked;
    }

    bool GUI::MenuBarToggle(const char* title, bool* v, const char* tooltip)
    {
        ImVec2 size = ImGui::CalcTextSize(title);
        bool clicked = ImGui::Selectable(title, v, ImGuiSelectableFlags_None, size);
        if (tooltip && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tooltip);
        return clicked;
    }

    void GUI::WindowToggleButton(GUI::Window* window, float width, const char* tooltip)
    {
        if (ImGui::Selectable(window->name.c_str(), window->visible, ImGuiSelectableFlags_None, ImVec2(width, 20.0f)))
        {
            window->ToggleOrFocus();
        }
        if (tooltip && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tooltip);
    }

    bool GUI::Thumbnail(const char* name, Texture* icon, bool selected)
    {
        bool clicked = false;
        ImVec2 itemSize = {80, 80};
        float iconSize = 48;
        float iconPadding = (itemSize.x - iconSize) * 0.5;
        float textSize = ImGui::CalcTextSize(name).x;
        float textPadding = (itemSize.x - textSize) * 0.5;

        ImGui::BeginGroup();
        if (ImGui::Selectable((std::string("##") + name).c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick, itemSize)) {
            clicked = true;
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - itemSize.y);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + iconPadding);
#if 0
        if (icon != nullptr)
            ImGui::Image(icon->handle->Value(), {iconSize, iconSize});
#endif
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textPadding);
        ImGui::TextUnformatted(name);
        ImGui::EndGroup();

        return clicked;
    }
}