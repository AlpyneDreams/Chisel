#include "AssetPicker.h"

#include "imgui.h"
#include "chisel/Chisel.h"
#include "chisel/FGD/FGD.h"
#include "chisel/map/Map.h"
#include "chisel/Selection.h"
#include "gui/IconsMaterialCommunity.h"

#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <unordered_set>

namespace chisel
{
    static const uint2 AssetPadding { 16, 16 };

    AssetPicker::AssetPicker() : GUI::Window(ICON_MC_FOLDER, "Assets", 1024, 512, false, ImGuiWindowFlags_MenuBar)
    {
        m_LastWindowSize = uint2(1024, 512);
        m_AssetsPerRow = uint(floor(m_LastWindowSize.x / (AssetThumbnailSize.x + AssetPadding.x)));
        Refresh();
    }

    void AssetPicker::Draw()
    {
        if (ImGui::BeginMenuBar())
        {
            // Right side
            ImGui::Spacing();
            ImGui::SameLine(ImGui::GetWindowWidth() - 200);
            ImGui::SetNextItemWidth(190);
            if (ImGui::SliderInt("##Zoom", &ThumbnailScale, 8, 32, std::to_string(AssetThumbnailSize.x).c_str()))
            {
                AssetThumbnailSize = uint2(16 * ThumbnailScale);
            }
            ImGui::EndMenuBar();
        }
        //ImGui::PushFont(GUI::FontMonospace);
        ImVec2 windowSize = ImGui::GetWindowSize();
        m_LastWindowSize = uint2(windowSize.x, windowSize.y);
        m_AssetsPerRow = uint(floor(m_LastWindowSize.x / (AssetThumbnailSize.x + AssetPadding.x)));

        float scroll = ImGui::GetScrollY();
        
        uint numVisibleRows = uint(uint(ImGui::GetWindowSize().y) / (AssetThumbnailSize.y + AssetPadding.y)) + 2;

        uint xAssetRow = uint(scroll / (AssetThumbnailSize.y + AssetPadding.y));
        uint xAsset = xAssetRow * m_AssetsPerRow;

        if (m_materials.size() == 0)
            return;

        uint currentAsset = xAsset;
        for (uint row = 0; row < numVisibleRows; row++)
        {
            for (uint column = 0; column < m_AssetsPerRow; column++)
            {
                const float initialXPadding = 8.0f;
                const float initialYPadding = 32.0f;
                ImVec2 basePos = ImVec2(column * (AssetThumbnailSize.x + AssetPadding.x) + initialXPadding, (xAssetRow + row) * (AssetThumbnailSize.y + AssetPadding.y) + initialYPadding);

                auto& material = m_materials[currentAsset];
                if (!material.thing && !material.triedToLoad)
                {
                    material.thing = Assets.Load<Material>(material.path);
                    material.triedToLoad = true;
                }
                if (material.thing && material.thing->baseTexture && material.thing->baseTexture->srvLinear != nullptr)
                {
                    ImGui::SetCursorPos(basePos);
                    ImVec2 screenPos = ImGui::GetCursorScreenPos();

                    ImVec2 endPos = ImVec2(screenPos.x + AssetThumbnailSize.x, screenPos.y + AssetThumbnailSize.y);

                    ImGui::GetWindowDrawList()->AddImage(
                        material.thing->baseTexture->srvLinear.ptr(),
                        screenPos, endPos,
                        ImVec2(0, 0), ImVec2(1, 1)
                    );
                }


                ImVec2 textPos = ImVec2(basePos.x, basePos.y + AssetThumbnailSize.y);
                ImGui::SetCursorPos(textPos);
                // this sucks
                std::string path = material.path;
                if (path.starts_with("materials"))
                    path = path.substr(10);
                ImGui::Text("%s", path.c_str());
                currentAsset++;

                if (currentAsset >= m_materials.size())
                    return;
            }
        }

        //ImGui::PopFont();
    }

    bool AssetPicker::OverrideContentSize(uint2& size)
    {
        uint count = uint(m_materials.size());
        uint numRows = count / m_AssetsPerRow;
        // Never have an X scrollbar.
        size = uint2(m_LastWindowSize.x, numRows * (AssetThumbnailSize.y + AssetPadding.y));
        return true;
    }

    void AssetPicker::Refresh()
    {
        m_materials.clear();

        Assets.ForEachFile<Material>(
        [&](const fs::Path& path)
        {
            m_materials.emplace_back(std::string(path), nullptr);
        });
        std::sort(m_materials.begin(), m_materials.end(), [](AssetPickerAsset<Material>& a, AssetPickerAsset<Material>& b) { return a.path < b.path; });
    }
}
