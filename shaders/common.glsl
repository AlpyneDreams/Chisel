
// You should include this file in all shaders.

// bgfx_shader.sh: This can change with bgfx versions.
// - Includes many common functions: saturate, mul, rcp, etc.
// - Includes common uniforms: u_viewRect, u_viewTexel, u_alphaRef
// - Includes common matrices:
//      - u_view, u_invView
//      - u_proj, u_invProj
//      - u_viewProj, u_invViewProj
//      - u_model[N], u_modelView, u_modelViewProj
#include "../submodules/bgfx/src/bgfx_shader.sh"

