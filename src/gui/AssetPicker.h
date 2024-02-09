#pragma once

#include "gui/Common.h"
#include "gui/Window.h"
#include "assets/Assets.h"

#include <deque>
#include <unordered_set>
namespace chisel
{
    template <typename T>
    struct AssetPickerAsset
    {
        std::string path;
        std::string name;
        Rc<T> thing;
        bool triedToLoad = false;

        void Load()
        {
            if (thing == nullptr && !triedToLoad)
            {
                thing = Assets.Load<T>(path);
                triedToLoad = true;
            }
        }
    };

    struct AssetPicker : public GUI::Window
    {
        AssetPicker();
        ~AssetPicker();

        void Draw() override;
        void Tick() override;

        bool OverrideContentSize(uint2& size) override;

        void Refresh();

    private:
        bool IsAssetVisible(uint index) const;
        bool IsAssetAlmostVisible(uint index) const;

        std::vector<AssetPickerAsset<Material>> m_materials;
        std::deque<uint> m_thumbnailQueue;
        std::unordered_set<uint> m_thumbnailQueueSet;

        uint2 m_LastWindowSize;
        int ThumbnailScale = 7; // size = 16 * scale
        uint2 AssetThumbnailSize = { 128, 128 };
        uint m_AssetsPerRow;
        uint m_FirstVisibleAsset = 0;
        uint m_NumVisibleRows = 0;
        uint m_LoadedAssetCount = 0;
    };
}
