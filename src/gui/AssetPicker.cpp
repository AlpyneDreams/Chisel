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
    static const std::pair<uint32_t, uint32_t> AssetThumbnailSize{ 256, 256 };
    static const std::pair<uint32_t, uint32_t> AssetPadding{ 16, 16 };

    AssetPicker::AssetPicker() : GUI::Window(ICON_MC_INFORMATION, "Asset Picker", 1024, 512, true, 0)
    {
        m_LastWindowSize = ImVec2(1024, 512);
        m_AssetsPerRow = floor(m_LastWindowSize.x / (AssetThumbnailSize.first + AssetPadding.first));
        Refresh();
    }

    void AssetPicker::Draw()
    {
        //ImGui::PushFont(GUI::FontMonospace);
        m_LastWindowSize = ImGui::GetWindowSize();
        m_AssetsPerRow = floor(m_LastWindowSize.x / (AssetThumbnailSize.first + AssetPadding.first));

        float scroll = ImGui::GetScrollY();
        
        uint32_t numVisibleRows = (uint32_t(ImGui::GetWindowSize().y) / (AssetThumbnailSize.second + AssetPadding.second)) + 2;

        uint32_t firstAssetRow = scroll / (AssetThumbnailSize.second + AssetPadding.second);
        uint32_t firstAsset = firstAssetRow * m_AssetsPerRow;

        uint32_t currentAsset = firstAsset;
        for (uint32_t row = 0; row < numVisibleRows; row++)
        {
            for (uint32_t column = 0; column < m_AssetsPerRow; column++)
            {
                const float initialXPadding = 8.0f;
                const float initialYPadding = 32.0f;
                ImVec2 basePos = ImVec2(column * (AssetThumbnailSize.first + AssetPadding.first) + initialXPadding, (firstAssetRow + row) * (AssetThumbnailSize.second + AssetPadding.second) + initialYPadding);

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

                    ImVec2 endPos = ImVec2(screenPos.x + AssetThumbnailSize.first, screenPos.y + AssetThumbnailSize.second);

                    ImGui::GetWindowDrawList()->AddImage(
                        material.thing->baseTexture->srvLinear.ptr(),
                        screenPos, endPos,
                        ImVec2(0, 0), ImVec2(1, 1)
                    );
                }


                ImVec2 textPos = ImVec2(basePos.x, basePos.y + AssetThumbnailSize.second);
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

    bool AssetPicker::OverrideContentSize(ImVec2& size)
    {
        uint32_t count = uint32_t(m_materials.size());
        uint32_t numRows = count / m_AssetsPerRow;
        // Never have an X scrollbar.
        size = ImVec2(m_LastWindowSize.x, numRows * (AssetThumbnailSize.second + AssetPadding.second));
        return true;
    }

    void AssetPicker::Refresh()
    {
        m_materials.clear();

        Assets.EnumerateAssets<Material>(
        [&](const fs::Path& path)
        {
            m_materials.emplace_back(std::string(path), nullptr);
        });
        std::sort(m_materials.begin(), m_materials.end(), [](AssetPickerAsset<Material>& a, AssetPickerAsset<Material>& b) { return a.path < b.path; });
    }
}
