#pragma once

#include "common/Common.h"
#include "math/Math.h"

#include <set>

namespace chisel
{
    struct Viewport;

    struct Tool
    {
        Tool(const char* name, const char* icon, uint order);

        // Called on viewport click. Used for selection.
        virtual void OnClick(Viewport& viewport, uint2 mouse) {}

        // Called every frame. Handle rendering and input logic go here.
        virtual void DrawHandles(Viewport& viewport) {}
        
        // Tool Properties GUI
        virtual bool HasPropertiesGUI() { return false; }
        virtual void DrawPropertiesGUI() {}

    public:
        uint GetToolCategory() const { return order / 100; }
        void DrawPropertiesWindow(Rect viewport, uint instance = 0);

        uint order = 0;
        const char* name = "Tool";
        const char* icon = "";

    public:
        static Tool* Default;

        struct ToolOrder {
            bool operator()(Tool* a, Tool* b) const {
                return a->order < b->order;
            }
        };

        static inline std::set<Tool*, ToolOrder> Tools;
    };
}
