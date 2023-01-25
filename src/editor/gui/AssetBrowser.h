#pragma once

#include "imgui/Common.h"
#include "imgui/Window.h"
#include "imgui/IconsMaterialCommunity.h"

#include "input/Keyboard.h"

#include "assets/Assets.h"

#include "common/Filesystem.h"
#include <filesystem>

namespace engine::editor
{
    struct AssetBrowser : public GUI::Window
    {
        AssetBrowser() : GUI::Window(ICON_MC_FOLDER, "Files", 512, 512, false, ImGuiWindowFlags_MenuBar) {}
        static inline bool loadedIcons = false;
        static inline Texture* IconFolder;
        static inline Texture* IconFile;


        const fs::Path rootDir = ".";
        fs::Path currentDir = "core";
        std::set<fs::Path> selected;

        void ChangeDir(fs::Path path)
        {
            currentDir = path;
            selected.clear();
        }

        void Update() final override
        {
            if (!loadedIcons) {
                IconFolder = Assets.Load<Texture, ".PNG">("textures/ui/folder.png");
                IconFile   = Assets.Load<Texture, ".PNG">("textures/ui/file.png");
                loadedIcons = true;
            }

            if (ImGui::GetIO().KeyCtrl && Keyboard.GetKeyUp(Key::Space)) {
                ToggleOrFocus();
            }
            Window::Update();
        }

        void Draw() override
        {
            if (ImGui::BeginMenuBar())
            {
                ImGui::TextUnformatted("Files");
                fs::Path path = rootDir;
                for (auto dir : std::filesystem::relative(currentDir, rootDir))
                {
                    path = path / dir;
                    ImGui::TextUnformatted(ICON_MC_CHEVRON_RIGHT);
                    if (GUI::MenuBarButton(dir.string())) {
                        ChangeDir(path);
                    }
                }
                ImGui::EndMenuBar();
            }
            DrawFiles();
        }

        void DrawFiles()
        {
            // TODO: Cache directory file list
            for (auto file : std::filesystem::directory_iterator(currentDir))
            {
                auto path = file.path();
                bool dir = file.is_directory();
                if (GUI::Thumbnail(path.filename().string().c_str(), dir ? IconFolder : IconFile, selected.contains(path)))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (dir) {
                            ChangeDir(path);
                        }
                    }
                    else
                    {
                        if (ImGui::GetIO().KeyCtrl) // Multi-select / deselect
                        {
                            if (selected.contains(path))
                                selected.erase(path);
                            else
                                selected.insert(path);
                        }
                        else // Single selection
                        {
                            selected.clear();
                            selected.insert(path);
                        }
                    }
                }
                ImGui::SameLine();
            }
        }
    };
}
