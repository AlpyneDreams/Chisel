#pragma once

#include "gui/Common.h"
#include "gui/Window.h"

namespace chisel
{
    struct Entity;

    template <typename T>
    struct AssetPickerAsset
    {
        std::string path;
        T* thing = nullptr;
        bool triedToLoad = false;
    };

    struct AssetPicker : public GUI::Window
    {
        AssetPicker();

        void Draw() override;

        bool OverrideContentSize(ImVec2& size) override;

        void Refresh();

    private:
        std::vector<AssetPickerAsset<Material>> m_materials;

        ImVec2 m_LastWindowSize;
        float m_AssetsPerRow;
    };
}
