#include "SelectTool.h"
#include "input/Keyboard.h"
#include "gui/IconsMaterialCommunity.h"
#include "gui/Viewport.h"

namespace chisel
{
    static SelectTool Instance;

    SelectTool::SelectTool()
        : Tool("Select", ICON_MC_CURSOR_DEFAULT, 0)
    {
    }

    void SelectTool::OnClick(Viewport& viewport, uint2 mouse)
    {
        Engine.PickObject(mouse, viewport.rt_ObjectID, [](void* data) {
            uint id = ((uint*)data)[0];
            
            if (id == 0) {
                Selection.Clear();
            } else {
                Selectable* selection = Selection.Find(id);

                if (selection)
                {
                    if (Keyboard.ctrl)
                    {
                        Selection.Toggle(selection);
                    }
                    else
                    {
                        Selection.Clear();
                        Selection.Select(selection);
                    }
                }
            }
        });
    }
}
