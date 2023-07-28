#include "console/Console.h"
#include "gui/Viewport.h"
#include "chisel/Gizmos.h"
#include "chisel/Handles.h"
#include "chisel/MapRender.h"
#include "chisel/tools/Tool.h"

#include "math/Plane.h"
#include "math/Ray.h"

#include "gui/IconsMaterialCommunity.h"

namespace chisel
{
    Viewport::Viewport() : View3D(ICON_MC_IMAGE_SIZE_SELECT_ACTUAL, "Viewport", 512, 512, true) {}

    void Viewport::Start()
    {
        View3D::Start();
        OnResize(width, height); // Ballpark initial size
    }

    void Viewport::OnResize(uint width, uint height)
    {
        rt_SceneView = Engine.rctx.CreateRenderTarget(width, height);
        ds_SceneView = Engine.rctx.CreateDepthStencil(width, height);
        rt_ObjectID  = Engine.rctx.CreateRenderTarget(width, height, DXGI_FORMAT_R32_UINT);
        camera.renderTarget = rt_SceneView;
    }

    void Viewport::Render()
    {
        Chisel.Renderer->DrawViewport(*this);
    }

    void* Viewport::GetMainTexture()
    {
        return GetTexture(drawMode)->srvLinear.ptr();
    }

    void Viewport::OnClick(uint2 mouse)
    {
        Chisel.tool->OnClick(*this, mouse);
    }

    void Viewport::DrawHandles(mat4x4& view, mat4x4& proj)
    {
        // Draw general handles
        Chisel.Renderer->DrawHandles(view, proj);

        // Draw transform handles
        Chisel.tool->DrawHandles(*this);
        
        // Draw view cube
        {
            const float size = 128.0f;
            Rect gizmoRect = viewport;
            gizmoRect.x = viewport.x + viewport.w - size;
            gizmoRect.w = size;
            gizmoRect.h = size;
            Handles.ViewManiuplate(gizmoRect, view, 35.f, size, Colors.Transparent);
            
            // Since we don't actually update the view matrix, just apply sped-up
            // mouselook if the user is dragging the cube with left click.
            if (IsMouseOver(gizmoRect) && Mouse.GetButtonDown(Mouse.Left))
                isDraggingCube = true;
            else if (!Mouse.GetButton(Mouse.Left))
                isDraggingCube = false;

            if (isDraggingCube && Mouse.GetButton(Mouse.Left))
                MouseLook(Mouse.GetMotion() * 5);
        }
    }

    void Viewport::OnPostDraw()
    {
        if (!open) {
            Engine.systems.RemoveSystem(this);
            return;
        }

        Chisel.tool->DrawPropertiesWindow(viewport, instance);

        if (IsMouseOver(viewport))
        {
            if (!Selection.Empty() && (/*Mouse.GetButtonUp(Mouse.Right) ||*/ Mouse.GetButtonDown(Mouse.Middle)))
            {
                ImGui::OpenPopup("Selection");
            }

            if (ImGui::BeginPopup("Selection"))
            {
                ImGui::Text("Brush Selection");
                ImGui::Separator();
                ImGui::Selectable( ICON_MC_SCISSORS_CUTTING " Cut (TODO)");
                ImGui::Selectable( ICON_MC_FILE_DOCUMENT " Copy (TODO)");
                ImGui::Selectable( ICON_MC_CLIPBOARD " Paste (TODO)");
                if (ImGui::Selectable( ICON_MC_TRASH_CAN " Delete"))
                {
                    Selection.Delete();
                }
                ImGui::Separator();

                if (ImGui::Selectable( ICON_MC_GRID " Align to Grid"))
                    Selection.AlignToGrid(view_grid_size);

                if (ImGui::Selectable( ICON_MC_CONTENT_DUPLICATE " Duplicate"))
                    Selection.Duplicate();

                ImGui::EndPopup();
            }
        }
    }

// Draw Modes //

    void Viewport::OnDrawMenu()
    {
        ImGui::TextUnformatted("View Mode");
        ImGui::Separator();
        for (int i = 0; i < sizeof(drawModes) / sizeof(const char*); i++)
        {
            if (ImGui::MenuItem(drawModes[i], "", drawMode == DrawMode(i)))
                drawMode = DrawMode(i);
        }
        ImGui::TextUnformatted("");
        ImGui::Spacing();
    }

    Texture* Viewport::GetTexture(Viewport::DrawMode mode)
    {
        switch (mode) {
            default:
            case DrawMode::Shaded:
            case DrawMode::Wireframe:
                return rt_SceneView.ptr();
#if 0
            case DrawMode::Depth:
                return rt_SceneView->GetDepthTexture();
#endif
        }
    }
}
