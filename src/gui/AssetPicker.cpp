#include "AssetPicker.h"

#include "imgui.h"
#include "chisel/Chisel.h"
#include "chisel/FGD/FGD.h"
#include "chisel/map/Map.h"
#include "chisel/Selection.h"
#include "gui/IconsMaterialCommunity.h"

#include <misc/cpp/imgui_stdlib.h>
#include <string>

namespace chisel
{
    static const uint2 AssetPadding { 16, 16 };

    static std::unordered_set<AssetPicker*> s_AssetPickers;

    AssetPicker::AssetPicker() : GUI::Window(ICON_MC_FOLDER, "Assets", 1024, 512, false, ImGuiWindowFlags_MenuBar)
    {
        if (static bool s_Registered = false; !s_Registered)
        {
            Assets.OnRefresh += [] {
                for (auto picker : s_AssetPickers)
                    picker->Refresh();
            };
            s_Registered = true;
        }
        
        s_AssetPickers.insert(this);
        m_LastWindowSize = uint2(1024, 512);
        m_AssetsPerRow = uint(floor(m_LastWindowSize.x / (AssetThumbnailSize.x + AssetPadding.x)));
        Refresh();
    }

    AssetPicker::~AssetPicker()
    {
        if (s_AssetPickers.contains(this))
            s_AssetPickers.erase(this);
    }

    bool AssetPicker::IsAssetVisible(uint index) const
    {
        return index > m_FirstVisibleAsset
            && index < m_FirstVisibleAsset + (m_AssetsPerRow * m_NumVisibleRows);
    }
    
    bool AssetPicker::IsAssetAlmostVisible(uint index) const
    {
        return index > m_FirstVisibleAsset - (m_AssetsPerRow * m_NumVisibleRows * 2)
            && index < m_FirstVisibleAsset + (m_AssetsPerRow * m_NumVisibleRows * 2);
    }

    void AssetPicker::Draw()
    {
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("Loaded %u/%llu", m_LoadedAssetCount, m_materials.size());
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
        ImGui::PushFont(GUI::FontDense);

        ImVec2 windowSize = ImGui::GetWindowSize();
        m_LastWindowSize = uint2(windowSize.x, windowSize.y);
        m_AssetsPerRow = uint(floor(m_LastWindowSize.x / (AssetThumbnailSize.x + AssetPadding.x)));

        float scroll = ImGui::GetScrollY();
        
        m_NumVisibleRows = uint(uint(ImGui::GetWindowSize().y) / (AssetThumbnailSize.y + AssetPadding.y)) + 2;

        uint xAssetRow = uint(scroll / (AssetThumbnailSize.y + AssetPadding.y));
        m_FirstVisibleAsset = xAssetRow * m_AssetsPerRow;

        // This is in a lambda so we can PopFont when we're done
        auto render = [&]
        {
            if (m_materials.size() == 0)
                return;

            uint currentAsset = m_FirstVisibleAsset;
            for (uint row = 0; row < m_NumVisibleRows; row++)
            {
                for (uint column = 0; column < m_AssetsPerRow; column++)
                {
                    const float initialXPadding = 8.0f;
                    const float initialYPadding = 32.0f;
                    ImVec2 basePos = ImVec2(column * (AssetThumbnailSize.x + AssetPadding.x) + initialXPadding, (xAssetRow + row) * (AssetThumbnailSize.y + AssetPadding.y) + initialYPadding);

                    auto& material = m_materials[currentAsset];

                    if (material.thing != nullptr && material.thing->baseTexture != nullptr && material.thing->baseTexture->srvLinear != nullptr)
                    {
                        ImGui::SetCursorPos(basePos);

                        bool selected = Chisel.activeMaterial == material.thing;
                        if (selected)
                            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TabActive));

                        if (ImGui::ImageButton(material.path.c_str(), (ImTextureID)material.thing->baseTexture->srvLinear.ptr(),
                            ImVec2(float(AssetThumbnailSize.x), float(AssetThumbnailSize.y)), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
                        {
                            Chisel.activeMaterial = material.thing;
                        }

                        if (selected)
                            ImGui::PopStyleColor();
                    }
                    else
                    {
                        ImGui::SetCursorPos(basePos);

                        if (ImGui::ImageButton(material.path.c_str(), (ImTextureID)nullptr,
                            ImVec2(float(AssetThumbnailSize.x), float(AssetThumbnailSize.y)), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
                        {
                            Chisel.activeMaterial = material.thing;
                        }

                        if (!material.triedToLoad && !m_thumbnailQueueSet.contains(currentAsset))
                        {
                            m_thumbnailQueue.push_front(currentAsset);
                            m_thumbnailQueueSet.insert(currentAsset);
                        }
                    }

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", material.path.c_str());
                        ImGui::EndTooltip();
                    }

                    ImVec2 textPos = ImVec2(basePos.x, basePos.y + AssetThumbnailSize.y);
                    ImGui::SetCursorPos(textPos);

                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImGui::PushClipRect(pos, ImVec2(pos.x + AssetThumbnailSize.x, pos.y + AssetPadding.y), true);

                    std::string_view name = material.name;
                    ImGui::Text("%.*s", (int)name.size(), name.data());

                    ImGui::PopClipRect();

                    if (++currentAsset >= m_materials.size())
                        return;
                }
            }
        };
        render();

        ImGui::PopFont();
    }

    void AssetPicker::Tick()
    {
        if (!open)
            return;

        int numLoaded = 0;
        if (m_thumbnailQueue.size() > 0)
        {
            // Load 8 textures per tick as we scroll in new textures
			while ( m_thumbnailQueue.size() > 0 && numLoaded++ < 8 )
			{
                uint index = m_thumbnailQueue.front();
                m_materials[index].Load();
                m_thumbnailQueue.pop_front();
                m_LoadedAssetCount++;
                m_thumbnailQueueSet.erase(index);
			}
        }
        else
        {
            // Otherwise, load 4 models per tick from off-screen
			for ( uint i = 0; i < m_materials.size() && numLoaded <= 4; i++ )
			{
				if ( !m_materials[i].triedToLoad && IsAssetAlmostVisible(i) )
				{
					m_materials[i].Load();
                    m_LoadedAssetCount++;
					numLoaded++;
				}
			}
        }
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
        m_thumbnailQueue.clear();

        Assets.ForEachFile<Material>(
        [&](const fs::Path& p)
        {
            AssetPickerAsset<Material>& asset = m_materials.emplace_back();
            asset.path = std::string(p);

            fs::Path subpath;
            std::filesystem::path path = p;
            bool foundMaterials = false;
            for (auto& part : path)
            {
                if (foundMaterials)
                    subpath /= part;
                if (part == "materials")
                    foundMaterials = true;
            }
            if (!foundMaterials)
                subpath = p.dirname().filename() / p.filename();

            // Remove materials/ and .vmt for display name
            std::string_view name = subpath;
            if (name.starts_with("materials") && (name[9] == '/' || name[9] == '\\'))
                name.remove_prefix(10);
            name.remove_suffix(std::string_view(p.ext()).size());
            asset.name = name;
        });
        std::sort(m_materials.begin(), m_materials.end(), [](AssetPickerAsset<Material>& a, AssetPickerAsset<Material>& b) { return a.path < b.path; });
        if (Chisel.activeMaterial == nullptr && !m_materials.empty())
        {
            m_materials[0].Load();
            Chisel.activeMaterial = m_materials[0].thing;
        }
    }
}
