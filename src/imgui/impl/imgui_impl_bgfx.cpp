// Derived from this Gist by Richard Gale:
//     https://gist.github.com/RichardGale/6e2b74bc42b3005e08397236e4be0fd0
// Updated to support multiple viewports.

// ImGui BFFX binding
// In this binding, ImTextureID is used to store an OpenGL 'GLuint' texture
// identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See
// main.cpp for an example of using this. If you use this binding you'll need to
// call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(),
// ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see
// examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include "platform/Platform.h"

#include "imgui_impl_bgfx.h"
#include "imgui.h"

// BGFX/BX
#include "bgfx/bgfx.h"
#include "bgfx/embedded_shader.h"
#include "bx/math.h"
#include "bx/timer.h"

// Data
static uint8_t g_View = 255;
static uint8_t g_LastView = 255;
static bgfx::TextureHandle g_FontTexture = BGFX_INVALID_HANDLE;
static bgfx::ProgramHandle g_ShaderHandle = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle g_AttribLocationTex = BGFX_INVALID_HANDLE;
static bgfx::VertexLayout g_VertexLayout;

struct ImGui_Implbgfx_ViewportData
{
    bgfx::FrameBufferHandle fb;
    uint8_t view = 255;
};

static void ImGui_Implbgfx_SetupRenderState(ImDrawData* draw_data, uint8_t view, int fb_width, int fb_height)
{
    const bgfx::Caps* caps = bgfx::getCaps();

    // Setup viewport, orthographic projection matrix
    float ortho[16];
    bx::mtxOrtho(ortho,
        draw_data->DisplayPos.x,
        draw_data->DisplayPos.x + draw_data->DisplaySize.x,
        draw_data->DisplayPos.y + draw_data->DisplaySize.y,
        draw_data->DisplayPos.y, -1.0f, +1.0f,
        0.0f, caps->homogeneousDepth);
    bgfx::setViewTransform(view, NULL, ortho);
    bgfx::setViewRect(view, 0, 0, (uint16_t)fb_width, (uint16_t)fb_height);
}

// This is the main rendering function that you have to implement and call after
// ImGui::Render(). Pass ImGui::GetDrawData() to this function.
// Note: If text or lines are blurry when integrating ImGui into your engine,
// in your Render function, try translating your projection matrix by
// (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_Implbgfx_RenderDrawLists(ImDrawData* draw_data)
{
    ImGui_Implbgfx_ViewportData* vd = (ImGui_Implbgfx_ViewportData*)draw_data->OwnerViewport->RendererUserData;
    uint8_t view = vd ? vd->view : g_View;

    // Avoid rendering when minimized, scale coordinates for retina displays
    // (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0) {
        return;
    }

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup render state: alpha-blending enabled, no face culling,
    // no depth testing, scissor enabled
    uint64_t state =
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA |
        BGFX_STATE_BLEND_FUNC(
            BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);

    ImGui_Implbgfx_SetupRenderState(draw_data, view, fb_width, fb_height);

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        uint32_t idx_buffer_offset = 0;

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;

        uint32_t numVertices = (uint32_t)cmd_list->VtxBuffer.size();
        uint32_t numIndices = (uint32_t)cmd_list->IdxBuffer.size();

        if ((numVertices != bgfx::getAvailTransientVertexBuffer(
                                numVertices, g_VertexLayout)) ||
            (numIndices != bgfx::getAvailTransientIndexBuffer(numIndices))) {
            // not enough space in transient buffer, quit drawing the rest...
            break;
        }

        bgfx::allocTransientVertexBuffer(&tvb, numVertices, g_VertexLayout);
        bgfx::allocTransientIndexBuffer(&tib, numIndices);

        ImDrawVert* verts = (ImDrawVert*)tvb.data;
        memcpy(
            verts, cmd_list->VtxBuffer.begin(),
            numVertices * sizeof(ImDrawVert));

        ImDrawIdx* indices = (ImDrawIdx*)tib.data;
        memcpy(
            indices, cmd_list->IdxBuffer.begin(),
            numIndices * sizeof(ImDrawIdx));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (pcmd->UserCallback) {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_Implbgfx_SetupRenderState(draw_data, view, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            } else {
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;
                
                bgfx::setScissor(
                    (int)clip_min.x, (int)(clip_min.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)
                );

                bgfx::setState(state);
                bgfx::TextureHandle texture = {
                    (uint16_t)((intptr_t)pcmd->TextureId & 0xffff)};
                bgfx::setTexture(0, g_AttribLocationTex, texture);
                bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
                bgfx::setIndexBuffer(&tib, idx_buffer_offset, pcmd->ElemCount);
                bgfx::submit(view, g_ShaderHandle);
            }

            idx_buffer_offset += pcmd->ElemCount;
        }
    }
}

bool ImGui_Implbgfx_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Upload texture to graphics system
    g_FontTexture = bgfx::createTexture2D(
        (uint16_t)width, (uint16_t)height, false, 1, bgfx::TextureFormat::BGRA8,
        0, bgfx::copy(pixels, width * height * 4));

    // Store our identifier
    io.Fonts->TexID = (void*)(intptr_t)g_FontTexture.idx;

    return true;
}

static void ImGui_Implbgfx_CreateWindow(ImGuiViewport* viewport)
{
    ImGui_Implbgfx_ViewportData* vd = IM_NEW(ImGui_Implbgfx_ViewportData)();
    viewport->RendererUserData = vd;

    vd->fb = bgfx::createFrameBuffer(viewport->PlatformHandleRaw, uint16_t(viewport->Size.x), uint16_t(viewport->Size.y));
    vd->view = --g_LastView;
}

static void ImGui_Implbgfx_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGui_Implbgfx_ViewportData* vd = (ImGui_Implbgfx_ViewportData*)viewport->RendererUserData;
    bgfx::destroy(vd->fb);

    vd->fb = bgfx::createFrameBuffer(viewport->PlatformHandleRaw, uint16_t(size.x), uint16_t(size.y));
}

static void ImGui_Implbgfx_DestroyWindow(ImGuiViewport* viewport)
{
    if (ImGui_Implbgfx_ViewportData* vd = (ImGui_Implbgfx_ViewportData*)viewport->RendererUserData)
    {
        if (vd->view == g_LastView)
            g_LastView++;
        
        bgfx::destroy(vd->fb);
        delete vd;
    }
    viewport->RendererUserData = NULL;
}


#include "fs_ocornut_imgui.bin.h"
#include "vs_ocornut_imgui.bin.h"

static const bgfx::EmbeddedShader s_embeddedShaders[] = {
    BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
    BGFX_EMBEDDED_SHADER(fs_ocornut_imgui), BGFX_EMBEDDED_SHADER_END()};

bool ImGui_Implbgfx_CreateDeviceObjects()
{
    bgfx::RendererType::Enum type = bgfx::getRendererType();
    g_ShaderHandle = bgfx::createProgram(
        bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_ocornut_imgui"),
        bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_ocornut_imgui"),
        true);

    g_VertexLayout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    g_AttribLocationTex =
        bgfx::createUniform("g_AttribLocationTex", bgfx::UniformType::Sampler);

    ImGui_Implbgfx_CreateFontsTexture();

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    for (int i = 1; i < platform_io.Viewports.Size; i++)
        if (!platform_io.Viewports[i]->RendererUserData)
            ImGui_Implbgfx_CreateWindow(platform_io.Viewports[i]);


    return true;
}

void ImGui_Implbgfx_InvalidateDeviceObjects()
{
    bgfx::destroy(g_AttribLocationTex);
    bgfx::destroy(g_ShaderHandle);

    if (isValid(g_FontTexture)) {
        bgfx::destroy(g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture.idx = bgfx::kInvalidHandle;
    }

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    for (int i = 1; i < platform_io.Viewports.Size; i++)
        if (platform_io.Viewports[i]->RendererUserData)
            ImGui_Implbgfx_DestroyWindow(platform_io.Viewports[i]);

}

static void ImGui_Implbgfx_RenderWindow(ImGuiViewport* viewport, void*)
{
    ImGui_Implbgfx_ViewportData* vd = (ImGui_Implbgfx_ViewportData*)viewport->RendererUserData;
    bgfx::setViewFrameBuffer(vd->view, vd->fb);

    if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
    {
        bgfx::setViewClear(vd->view, BGFX_CLEAR_COLOR, 0x000000ff);
    }
    ImGui_Implbgfx_RenderDrawLists(viewport->DrawData);
}

static void ImGui_Implbgfx_SwapBuffers(ImGuiViewport* viewport, void*)
{
    // Don't need to do this or else we get clear color flashing
    //bgfx::frame();
}

static void ImGui_Implbgfx_InitPlatformInterface()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_RenderWindow = ImGui_Implbgfx_RenderWindow;
    platform_io.Renderer_CreateWindow = ImGui_Implbgfx_CreateWindow;
    platform_io.Renderer_DestroyWindow = ImGui_Implbgfx_DestroyWindow;
    platform_io.Renderer_SetWindowSize = ImGui_Implbgfx_SetWindowSize;
    platform_io.Renderer_SwapBuffers = ImGui_Implbgfx_SwapBuffers;
}

void ImGui_Implbgfx_Init(int view)
{
    g_View = (uint8_t)(view & 0xff);
    g_LastView = g_View;

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui_Implbgfx_InitPlatformInterface();
}

void ImGui_Implbgfx_Shutdown()
{
    ImGui::DestroyPlatformWindows();
    ImGui_Implbgfx_InvalidateDeviceObjects();
}

void ImGui_Implbgfx_NewFrame()
{
    if (!isValid(g_FontTexture)) {
        ImGui_Implbgfx_CreateDeviceObjects();
    }
}