#include "View3D.h"

#include "entity/Common.h"
#include "editor/Gizmos.h"
#include "entity/components/MeshRenderer.h"
#include "entity/Scene.h"
#include "entity/Entity.h"

namespace engine::editor
{
    struct SceneView : public View3D
    {
        SceneView() : View3D(ICON_MC_IMAGE_SIZE_SELECT_ACTUAL, "Scene", 512, 512, true) {}
        
        void DrawHandles(mat4x4& view, mat4x4& proj) override
        {                
            Entity active = Selection.Active();
            if (active && active.HasComponent<Transform>())
            {
                Transform& transform = active.GetComponent<Transform>();
                Handles.Maniuplate(transform, view, proj, activeTool, space, gridSnap, gridSize);
            }
            
            //Handles.DrawGrid2(view, proj, 100);
            //Handles.DrawTestCube(view, proj);
            //Handles.ViewManiuplate(viewport, view, 35.f, 128.f, Colors.Transparent);
        }
        
        void OnPostDraw() override
        {
            // Draw wireframe of current selection
            Entity ent = Selection.Active();
            if (ent && ent.HasComponent<MeshRenderer>()) {
                Mesh* mesh = ent.GetComponent<MeshRenderer>().mesh;
                if (ent.HasComponent<Transform>()) {
                    Tools.DrawSelectionOutline(mesh, ent.GetComponent<Transform>());
                } else {
                    Tools.DrawSelectionOutline(mesh);                    
                }
            }

            Gizmos.DrawIcon(vec3(0), Gizmos.icnLight);
        }
    };
}
