#pragma once

#include "gui/Common.h"
#include "gui/Window.h"
#include "assets/Assets.h"

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

        void Draw() override;

        bool OverrideContentSize(uint2& size) override;

        void Refresh();

    private:
        std::vector<AssetPickerAsset<Material>> m_materials;

        uint2 m_LastWindowSize;
        int ThumbnailScale = 7; // size = 16 * scale
        uint2 AssetThumbnailSize = { 128, 128 };
        uint m_AssetsPerRow;
    };
}
