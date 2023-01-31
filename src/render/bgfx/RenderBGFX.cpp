#include "common/Common.h"
#include "common/Filesystem.h"
#include "platform/Platform.h"
#include "console/ConVar.h"
#include "common/Time.h"

#include <algorithm>
#include <bgfx/bgfx.h>
#include <bx/string.h>
#include <bgfx/platform.h>

#include <imgui.h>
#include "gui/impl/imgui_impl_bgfx.h"
#include "gui/Common.h"

#include <stdexcept>
#include <unordered_map>
#include <map>
#include <typeindex>
#include <stack>

#include "platform/Window.h"
#include "render/Render.h"
#include "core/Mesh.h"
#include "common/Time.h"
#include "console/Console.h"
#include "console/ConCommand.h"
#include "render/Texture.h"

namespace chisel::render
{
    // Maps an integral type to bgfx::AttribType
    static std::unordered_map<std::type_index, bgfx::AttribType::Enum> bgfxAttribTypes {
        {std::type_index(typeid(uint8)), bgfx::AttribType::Uint8},
        //{std::type_index(typeid(uint10)), bgfx::AttribType::Uint10},
        {std::type_index(typeid(int16)), bgfx::AttribType::Int16},
        //{std::type_index(typeid(half)), bgfx::AttribType::Half},
        {std::type_index(typeid(float)), bgfx::AttribType::Float},
    };


    static bgfx::TextureFormat::Enum ConvertTextureFormat(TextureFormat format)
    {
        static std::unordered_map<TextureFormat, bgfx::TextureFormat::Enum> bgfxTextureFormats {
            {TextureFormat::R8,         bgfx::TextureFormat::R8},
            {TextureFormat::RG8,        bgfx::TextureFormat::RG8},
            {TextureFormat::RGB8,       bgfx::TextureFormat::RGB8},
            {TextureFormat::RGBA8,      bgfx::TextureFormat::RGBA8},
            {TextureFormat::R32F,       bgfx::TextureFormat::R32F},
            {TextureFormat::RGBA32F,    bgfx::TextureFormat::RGBA32F},
            {TextureFormat::D32F,       bgfx::TextureFormat::D32F},
        };

        return bgfxTextureFormats.contains(format) ? bgfxTextureFormats[format] : bgfx::TextureFormat::Unknown;
    }

    ConVar<bool> r_vsync("r_vsync", false, "Enable/disable vsync");

    struct ShaderBGFX final : public Shader
    {
        bgfx::ProgramHandle program;

        ~ShaderBGFX() {
            bgfx::destroy(program);
        }
    };

    // TODO: Refactor for arbitrary MRT RenderTarget
    struct RenderTargetBGFX final : public RenderTarget
    {
        bool hasDepth = true; // TEMP
        bgfx::TextureHandle color, depth; // TEMP
        bgfx::TextureFormat::Enum format, depthFormat; // TEMP
        bgfx::FrameBufferHandle fb;
        int view = -1;
        uint width, height;

        bool                       readBack       = false;
        bgfx::TextureHandle        readBackBuffer = BGFX_INVALID_HANDLE;
        uint                       readBackFrame  = 0;
        ReadBackFunc               readBackFunc;
        float*                     readBackData   = nullptr;
        size_t                     readBackSize   = 0;
        MSAA                       msaa           = MSAA::None;

        static inline std::set<RenderTargetBGFX*> pendingReadBacks;

        RenderTargetBGFX(uint width, uint height, TextureFormat color, TextureFormat depth) : width(width), height(height)
        {
            format = ConvertTextureFormat(color);
            if (depth == TextureFormat::None)
                hasDepth = false;
            depthFormat = ConvertTextureFormat(depth);
            Create();
        }

        void Create()
        {
            uint64 flags = BGFX_TEXTURE_RT;
            
            if (msaa > MSAA::None && IsValid(msaa)) {
                // Remap: x2 -> 2, x4 -> 3, x8 -> 4, x16 -> 5
                uint64 log2 = std::bit_width(uint64(msaa)) - 1;
                flags |= ((log2+1) << BGFX_TEXTURE_RT_MSAA_SHIFT) & BGFX_TEXTURE_RT_MSAA_MASK;                
            }

            color = bgfx::createTexture2D(width, height, false, 1, format, flags);
            
            // If MSAA is enabled, then depth buffer must be write-only
            if (msaa > MSAA::None)
                flags |= BGFX_TEXTURE_RT_WRITE_ONLY;
            
            if (hasDepth)
                depth = bgfx::createTexture2D(width, height, false, 1, depthFormat, flags);
            
            bgfx::TextureHandle handles[] = {color, depth};
            fb = bgfx::createFrameBuffer(hasDepth ? 2 : 1, handles, true);
            
            if (!bgfx::isValid(fb))
                throw std::runtime_error("[BGFX] Failed to create framebuffer!");

            if (readBack) {
                readBackBuffer = bgfx::createTexture2D(width, height, false, 1, format,
                    BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK
                );
            } else {
                readBackBuffer = BGFX_INVALID_HANDLE;
            }
        }

        void Destroy()
        {
            if (bgfx::isValid(fb))
                bgfx::destroy(fb);
            if (bgfx::isValid(color))
                bgfx::destroy(color);
            if (hasDepth && bgfx::isValid(depth))
                bgfx::destroy(depth);
            if (readBack && bgfx::isValid(readBackBuffer))
                bgfx::destroy(readBackBuffer);
        }

        void Recreate() { Destroy(); Create(); }

        void* GetTexture() const {
            return (void*)uintptr_t(bgfx::getTexture(fb).idx);
        }

        void* GetDepthTexture() const {
            if (!hasDepth)
                return nullptr;
            return (void*)uintptr_t(bgfx::getTexture(fb, 1).idx);
        }

        uint2 GetSize() const {
            return uint2(width, height);
        }

        void Resize(uint w, uint h)
        {
            if (w == width && h == height)
                return;

            width = w; height = h;
            Recreate();
        }

        void SetMSAA(MSAA mode)
        {
            msaa = mode;
            Recreate();
        }

        void SetReadBack(bool enabled)
        {
            readBack = enabled;
            Recreate();
        }

        void ReadTexture(ReadBackFunc func)
        {
            // No readback, or already pending readback
            if (!readBack || readBackFrame != 0)
                return;
            readBackSize = width * height;
            readBackData = new float[readBackSize];
            bgfx::blit(view + 1, readBackBuffer, 0, 0, color);
            readBackFunc = func;
            readBackFrame = bgfx::readTexture(readBackBuffer, readBackData);
            pendingReadBacks.insert(this);
        }

        static void CheckPendingReadBacks(uint frame)
        {
            for (auto it = pendingReadBacks.begin(); it != pendingReadBacks.end(); )
            {
                auto* rt = *it;
                // No pending readback
                if (!rt->readBack || rt->readBackFrame == 0)
                {
                    pendingReadBacks.erase(it++);
                    continue;
                }

                // Readback is ready!
                if (rt->readBackFrame >= frame)
                {
                    rt->readBackFunc(rt->readBackData, rt->readBackSize, rt->width);
                    rt->readBackFrame = 0;
                    delete rt->readBackData;
                    rt->readBackData = nullptr;
                    pendingReadBacks.erase(it++);
                    continue;
                }
                else
                    ++it;
            }
        }

        ~RenderTargetBGFX() {
            bgfx::destroy(fb);
        }
    };

    struct HandleBGFX final : public Handle
    {
        union {
            bgfx::VertexBufferHandle vb;
            bgfx::IndexBufferHandle ib;
            bgfx::TextureHandle texture;
            void* value;
        };

        bgfx::UniformHandle sampler;

        void* Value() const override {
            return value;
        }
    };

    class RenderBGFX final : public Render
    {
        struct RenderState {
            uint width, height;
            bgfx::ViewId view = 0;
            bgfx::ViewId nextView = 1;

            static constexpr bgfx::ViewId defaultView = 0;
            static constexpr bgfx::ViewId imguiView = 255;

            mat4x4 mTransform, mView, mProj;

            struct ClearState {
                uint16 flags = BGFX_CLEAR_NONE;
                uint32 rgba  = 0x000000FF;
                float depth = 1.0f;
                uint8 stencil = 0;
            } clear;

            uint64 state = (0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CW // Clockwise winding order
                | BGFX_STATE_MSAA
                //| BGFX_STATE_LINEAA
            );

            bgfx::ProgramHandle currentProgram;

            std::map<std::string_view, bgfx::UniformHandle> uniforms;
        } state;

    protected:
        bool m_bWasVSync = false;
        Window *m_window = nullptr;

        void Init(Window* window)
        {
            m_window = window;

            // Signal to BGFX not to create a render thread
            bgfx::renderFrame();

            bgfx::Init init;

            init.type = bgfx::RendererType::Vulkan;

            init.platformData.ndt = window->GetNativeDisplay();
            init.platformData.nwh = window->GetNativeWindow();

            auto [width, height] = window->GetSize();
            init.resolution.width = uint32(width);
            init.resolution.height = uint32(height);

            if (r_vsync)
                init.resolution.reset = BGFX_RESET_VSYNC;
            //init.resolution.maxFrameLatency = 1; // This reduces mouse input lag for imgui
            m_bWasVSync = r_vsync;

            state.width = width, state.height = height;

            // Handle window resize
            window->SetResizeCallback([=, this](uint width, uint height) {
                bgfx::reset(uint32(width), uint32(height), BGFX_RESET_VSYNC);
                bgfx::setViewRect(state.defaultView, 0, 0, bgfx::BackbufferRatio::Equal);
            });

            if (!bgfx::init(init)) {
                throw std::runtime_error("[BGFX] Failed to initialize!");
            } else {
                Console.Log("[BGFX] Initialized BGFX with {}!", bgfx::getRendererName(bgfx::getRendererType()));
            }

            bgfx::setViewRect(state.defaultView, 0, 0, bgfx::BackbufferRatio::Equal);

            ImGui::CreateContext();

            // Setup fonts etc.
            GUI::Setup();

            ImGui_Implbgfx_Init(state.imguiView);
            ImGui_Implbgfx_NewFrame(); // Call at least once so ImGui_Implbgfx_Shutdown cannot fail
        }

        void BeginFrame()
        {
            // Always clear this view even if no draw calls are made
            if (m_window && m_bWasVSync != r_vsync)
            {
                auto [width, height] = m_window->GetSize();
                bgfx::reset(uint32(width), uint32(height), r_vsync ? BGFX_RESET_VSYNC : 0);
                m_bWasVSync = r_vsync;
            }
            bgfx::touch(state.defaultView);
        }

        void EndFrame()
        {
            bgfx::dbgTextClear();
            /*
            //bgfx::dbgTextImage(bx::max<uint16_t>(uint16_t(width / 2 / 8), 20) - 20, bx::max<uint16_t>(uint16_t(height / 2 / 16), 6) - 6, 40, 12, s_logo, 160);
            //bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats.");
            bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");
            bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
            bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");
            const bgfx::Stats* stats = bgfx::getStats();
            bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters.", stats->width, stats->height, stats->textWidth, stats->textHeight);
            */
            bgfx::setDebug(BGFX_DEBUG_TEXT);

            static double totalTime = 0;
            static double fps = (1 / Time.unscaled.deltaTime);
            totalTime += Time.unscaled.deltaTime;
            if (Time.frameCount % 29 == 0) {
                fps = (1 / (totalTime / 30));
                totalTime = Time.unscaled.deltaTime;
            }

            //bgfx::dbgTextPrintf(0, 3, 0x0f, "fps: %.0f, t: %.4f, dt: %.0f ms, frame: %d", fps, Time.unscaled.time, Time.unscaled.deltaTime * 1000, Time.frameCount);
            bgfx::dbgTextPrintf(0, 1, 0x0f, "%.0f", round(fps));

            ImGui::Render();
            ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

            uint frame = bgfx::frame();

            RenderTargetBGFX::CheckPendingReadBacks(frame);
        }

        void Shutdown()
        {
            for (auto& [name, handle] : state.uniforms) {
                bgfx::destroy(handle);
            }

            ImGui_Implbgfx_Shutdown();
            ImGui::DestroyContext();

            bgfx::shutdown();
        }

    public:
    // Resource Uploading //

        void UploadMesh(Mesh* mesh)
        {
            for (auto& group : mesh->groups)
            {
                if (!group.vertices)
                    continue;
                
                bgfx::VertexLayout layout;
                layout.begin();

                int colors = -1;
                int texcoords = -1;
                auto GetAttribute = [&](VertexAttribute::Mode mode) {
                    switch (mode) {
                        case VertexAttribute::Position: return bgfx::Attrib::Position;
                        case VertexAttribute::Normal:   return bgfx::Attrib::Normal;
                        case VertexAttribute::Tangent:  return bgfx::Attrib::Tangent;
                        case VertexAttribute::Bitangent: return bgfx::Attrib::Bitangent;
                        case VertexAttribute::Color:
                            colors = std::min(colors+1, 3);
                            return bgfx::Attrib::Enum(bgfx::Attrib::Color0 + colors);
                        case VertexAttribute::Indices:  return bgfx::Attrib::Indices;
                        case VertexAttribute::Weight:   return bgfx::Attrib::Weight;

                        default:
                        case VertexAttribute::TexCoord:
                            texcoords = std::min(texcoords+1, 7);
                            return bgfx::Attrib::Enum(bgfx::Attrib::TexCoord0 + texcoords);
                    }
                };
                for (auto attr : group.vertices.layout.Attributes()) {
                    layout.add(GetAttribute(attr.mode), attr.dimension, bgfxAttribTypes[std::type_index(attr.type)], attr.normalized);
                }
                layout.end();

                auto vb = bgfx::createVertexBuffer(
                    bgfx::makeRef(group.vertices.vertices, group.vertices.Size()),
                    layout
                );

                HandleBGFX* vh = new HandleBGFX();
                vh->vb = vb;
                group.vertices.handle = vh;

                if (group.indices)
                {
                    auto ib = bgfx::createIndexBuffer(
                        bgfx::makeRef(group.indices.indices, group.indices.Size()),
                        group.indices.type == IndexBuffer::UInt32 ? BGFX_BUFFER_INDEX32 : 0
                    );

                    HandleBGFX* ih = new HandleBGFX();
                    ih->ib = ib;
                    group.indices.handle = ih;
                }
            }

            mesh->uploaded = true;
        }

        void UploadTexture(Texture* texture, bool release)
        {
            if (texture->uploaded)
                return;

            if (!texture->hasData) {
                Console.Error("Tried to upload texture '{}' but it has no data!", texture->path);
                return;
            }

            auto releaseFn = [](void* data, void* userData) {
                ((Texture*)userData)->Free();
            };

            auto tex = bgfx::createTexture2D(
                uint16(texture->width),
                uint16(texture->height),
                false,
                1,
                ConvertTextureFormat(texture->format),
                BGFX_TEXTURE_NONE,
                release
                  ? bgfx::makeRef(texture->data, texture->Size(), releaseFn, texture)
                  : bgfx::makeRef(texture->data, texture->Size())
            );

            HandleBGFX* handle = new HandleBGFX();
            handle->texture = tex;
            handle->sampler = bgfx::createUniform(texture->path.string().c_str(), bgfx::UniformType::Sampler);
            texture->handle = handle;

            texture->uploaded = true;
        }

    // Resource Creation and Loading //

        RenderTarget* CreateRenderTarget(uint width, uint height, TextureFormat color, TextureFormat depth)
        {
            return new RenderTargetBGFX(width, height, color, depth);
        }

        Shader* CreateShader(const char* vsFile, const char* fsFile)
        {
            bgfx::ShaderHandle vert = LoadShaderModule(vsFile);
            bgfx::ShaderHandle frag = BGFX_INVALID_HANDLE;
            if (fsFile != NULL) {
                frag = LoadShaderModule(fsFile);
            }

            ShaderBGFX* shader = new ShaderBGFX();
            shader->program = bgfx::createProgram(vert, frag, true /* destroy shaders when program is destroyed */);
            return shader;
        }

    // Per-Camera State //

        inline void SetView(uint16 view)
        {
            state.view = view;

            // TODO: Preserve all per-view state
            SetViewTransform(state.mView, state.mProj);
            UpdateClearState(state);
            bgfx::setViewRect(state.view, 0, 0, bgfx::BackbufferRatio::Equal);
        }

        void SetRenderTarget(RenderTarget* target)
        {
            if (!target) {
                SetView(state.defaultView);
                bgfx::setViewFrameBuffer(state.view, BGFX_INVALID_HANDLE);
            } else {
                RenderTargetBGFX* rt = static_cast<RenderTargetBGFX*>(target);
                if (rt->view == -1) {
                    rt->view = state.nextView++;
                }
                SetView(rt->view);
                bgfx::setViewFrameBuffer(state.view, rt->fb);
            }
        }

        void SetViewTransform(const mat4x4& view, const mat4x4& proj)
        {
            state.mView = view; state.mProj = proj;
            bgfx::setViewTransform(state.view, &view[0][0], &proj[0][0]);
        }

        static inline void UpdateClearState(const RenderState& state)
        {
            bgfx::setViewClear(state.view, state.clear.flags, state.clear.rgba, state.clear.depth, state.clear.stencil);
        }

        void SetClearColor(bool clear, Color color)
        {
            if (clear)  state.clear.flags |= BGFX_CLEAR_COLOR;
            else        state.clear.flags &= ~BGFX_CLEAR_COLOR;
            state.clear.rgba = color.Pack();
            UpdateClearState(state);
        }

        void SetClearDepth(bool clear, float depth)
        {
            if (clear)  state.clear.flags |= BGFX_CLEAR_DEPTH;
            else        state.clear.flags &= ~BGFX_CLEAR_DEPTH;
            state.clear.depth = depth;
            UpdateClearState(state);
        }

    // Per-Object State //

        void SetDepthWrite(bool write)
        {
            if (write)
                state.state |= BGFX_STATE_WRITE_Z;
            else
                state.state &= ~BGFX_STATE_WRITE_Z;
        }

        void SetDepthTest(CompareFunc func)
        {
            state.state &= ~BGFX_STATE_DEPTH_TEST_MASK;
            auto GetDepthTest = [&](CompareFunc mode) -> uint64 {
                switch (mode) {
                    case CompareFunc::Disabled:     default: return 0;
                    case CompareFunc::Never:        return BGFX_STATE_DEPTH_TEST_NEVER;
                    case CompareFunc::Always:       return BGFX_STATE_DEPTH_TEST_ALWAYS;
                    case CompareFunc::Less:         return BGFX_STATE_DEPTH_TEST_LESS;
                    case CompareFunc::LessEqual:    return BGFX_STATE_DEPTH_TEST_LEQUAL;
                    case CompareFunc::Greater:      return BGFX_STATE_DEPTH_TEST_GREATER;
                    case CompareFunc::GreaterEqual: return BGFX_STATE_DEPTH_TEST_GEQUAL;
                    case CompareFunc::Equal:        return BGFX_STATE_DEPTH_TEST_EQUAL;
                    case CompareFunc::NotEqual:     return BGFX_STATE_DEPTH_TEST_NOTEQUAL;
                }
            };
            state.state |= GetDepthTest(func);
        }

        static inline uint64 GetBlendMode(BlendMode mode)
        {
            switch (mode) {
                default:
                case BlendMode::Default:          return 0;
                case BlendMode::Zero:             return BGFX_STATE_BLEND_ZERO;
                case BlendMode::One:              return BGFX_STATE_BLEND_ONE;
                case BlendMode::SrcColor:         return BGFX_STATE_BLEND_SRC_COLOR;
                case BlendMode::OneMinusSrcColor: return BGFX_STATE_BLEND_INV_SRC_COLOR;
                case BlendMode::DstColor:         return BGFX_STATE_BLEND_DST_COLOR;
                case BlendMode::OneMinusDstColor: return BGFX_STATE_BLEND_INV_DST_COLOR;
                case BlendMode::SrcAlpha:         return BGFX_STATE_BLEND_SRC_ALPHA;
                case BlendMode::OneMinusSrcAlpha: return BGFX_STATE_BLEND_INV_SRC_ALPHA;
                case BlendMode::DstAlpha:         return BGFX_STATE_BLEND_DST_ALPHA;
                case BlendMode::OneMinusDstAlpha: return BGFX_STATE_BLEND_INV_DST_ALPHA;
                case BlendMode::SrcAlphaSaturate: return BGFX_STATE_BLEND_SRC_ALPHA_SAT;
            }
        }

        static inline uint64 GetBlendOp(BlendOp mode)
        {
            switch (mode) {
                default:
                case BlendOp::Default:            return 0;
                case BlendOp::Add:                return BGFX_STATE_BLEND_EQUATION_ADD;
                case BlendOp::Subtract:           return BGFX_STATE_BLEND_EQUATION_SUB;
                case BlendOp::ReverseSubtract:    return BGFX_STATE_BLEND_EQUATION_REVSUB;
                case BlendOp::Min:                return BGFX_STATE_BLEND_EQUATION_MIN;
                case BlendOp::Max:                return BGFX_STATE_BLEND_EQUATION_MAX;
            }
        }

        void SetBlendFunc(BlendFunc func)
        {
            state.state &= ~BGFX_STATE_BLEND_MASK;
            state.state &= ~BGFX_STATE_BLEND_EQUATION_MASK;
            state.state |= BGFX_STATE_BLEND_FUNC_SEPARATE(
                GetBlendMode(func.src), GetBlendMode(func.dst),
                GetBlendMode(func.srcAlpha), GetBlendMode(func.dstAlpha)
            );
            state.state |= BGFX_STATE_BLEND_EQUATION_SEPARATE(
                GetBlendOp(func.rgbOp), GetBlendOp(func.alphaOp)
            );
        }

        void SetPrimitiveType(PrimitiveType type)
        {
            state.state &= ~BGFX_STATE_PT_MASK;
            switch (type) {
                case PrimitiveType::Points:     state.state |= BGFX_STATE_PT_POINTS; break;
                case PrimitiveType::Lines:      state.state |= BGFX_STATE_PT_LINES; break;
                case PrimitiveType::LineStrip:  state.state |= BGFX_STATE_PT_LINESTRIP; break;
                case PrimitiveType::TriStrip:   state.state |= BGFX_STATE_PT_TRISTRIP; break;
                case PrimitiveType::Triangles:  default: break;
            }
        }

        void SetPolygonMode(PolygonMode mode)
        {
            state.state &= ~BGFX_STATE_WIREFRAME;
            switch (mode) {
                case PolygonMode::Points:
                    SetPrimitiveType(PrimitiveType::Points);
                    return;
                case PolygonMode::Wireframe:
                    state.state |= BGFX_STATE_WIREFRAME;
                    return;
                default:
                    return;
            }
        }

        void SetTransform(const mat4x4& matrix)
        {
            state.mTransform = matrix;
            bgfx::setTransform(&matrix[0][0]);
        }

        void SetShader(Shader* shader)
        {
            state.currentProgram = static_cast<ShaderBGFX*>(shader)->program;
            // TODO: get uniforms...
        }

        void SetTexture(uint slot, Texture* texture)
        {
            if (!texture->uploaded) [[unlikely]] {
                UploadTexture(texture, true);
            }

            HandleBGFX* handle = static_cast<HandleBGFX*>(texture->handle);
            bgfx::setTexture(slot, handle->sampler, handle->texture);
        }

        void SetUniform(std::string_view name, void* value, uint stride, uint count)
        {
            bgfx::UniformHandle uniform;
            if (!state.uniforms.contains(name)) {
                state.uniforms[name] = bgfx::createUniform(name.data(), stride <= 4 ? bgfx::UniformType::Vec4 : bgfx::UniformType::Mat4, count);
            } else {
                uniform = state.uniforms[name];
            }
            bgfx::setUniform(state.uniforms[name], value, count);
        }

    // State Recording //
    
    protected:
        // TODO: Store partial state to make this faster.
        std::stack<RenderState> stateStack;
    
    public:
        void PushState()
        {
            stateStack.push(state);   
        }
        
        void PopState()
        {
            if (stateStack.empty())
                return;
            state = stateStack.top();
            stateStack.pop();
            
        // Per-view state
            SetView(state.view);
            // TODO: SetRenderTarget
            bgfx::setViewTransform(state.view, &state.mView[0][0], &state.mProj[0][0]);
            UpdateClearState(state);
        
        // Per-instance state
            // Most things are set in DrawMesh    
            bgfx::setTransform(&state.mTransform[0][0]);
            // TODO: SetTexture
            // TODO: SetUniform
        }

    // Draw Calls //

        void DrawMesh(Mesh* mesh)
        {
            if (!mesh) {
                return;
            }

            if (!mesh->uploaded) [[unlikely]] {
                UploadMesh(mesh);
            }

            bgfx::setState(state.state);

            for (auto& group : mesh->groups)
            {
                if (group.vertices.handle == nullptr)
                    continue;

                auto vb = static_cast<HandleBGFX*>(group.vertices.handle)->vb;
                bgfx::setVertexBuffer(0, vb);

                if (group.indices.handle != nullptr) {
                    auto ib = static_cast<HandleBGFX*>(group.indices.handle)->ib;
                    bgfx::setIndexBuffer(ib);
                }

                bgfx::submit(state.view, state.currentProgram);
            }
        }

    private:
        const bgfx::Memory* LoadMem(const char* filePath)
        {
            auto buffer = fs::readFileBinary(filePath);
            if (!buffer)
                return nullptr;
            return bgfx::copy(buffer->data(), buffer->size());
        }

        bgfx::ShaderHandle LoadShaderModule(const char* _name)
        {
            char filePath[512];

            const char* shaderPath = "???";

            switch (bgfx::getRendererType() )
            {
            case bgfx::RendererType::Noop:
            case bgfx::RendererType::Direct3D9:  shaderPath = "shaders/dx9/";   break;
            case bgfx::RendererType::Direct3D11:
            case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/";  break;
            case bgfx::RendererType::Agc:
            case bgfx::RendererType::Gnm:        shaderPath = "shaders/pssl/";  break;
            case bgfx::RendererType::Metal:      shaderPath = "shaders/metal/"; break;
            case bgfx::RendererType::Nvn:        shaderPath = "shaders/nvn/";   break;
            case bgfx::RendererType::OpenGL:     shaderPath = "shaders/glsl/";  break;
            case bgfx::RendererType::OpenGLES:   shaderPath = "shaders/essl/";  break;
            case bgfx::RendererType::Vulkan:     shaderPath = "shaders/spirv/"; break;
            case bgfx::RendererType::WebGPU:     shaderPath = "shaders/spirv/"; break;

            case bgfx::RendererType::Count:
                BX_ASSERT(false, "You should not be here!");
                break;
            }

            bx::strCopy(filePath, BX_COUNTOF(filePath), "core/");
            bx::strCat(filePath, BX_COUNTOF(filePath), shaderPath);
            bx::strCat(filePath, BX_COUNTOF(filePath), _name);
            bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");

            bgfx::ShaderHandle handle = bgfx::createShader(LoadMem(filePath));
            bgfx::setName(handle, _name);

            return handle;
        }

    };

    Render* Render::Create()
    {
        return new RenderBGFX();
    }

    static ConCommand bgfx_caps = ConCommand("bgfx_caps", "Prints bgfx GPU capabilities", []() {
        const bgfx::Caps* caps = bgfx::getCaps();
        Console.Log("Supported: {} (BGFX_CAPS_*)", caps->supported);
        Console.Log("Vendor ID: {}", caps->vendorId);
        Console.Log("Device ID: {}", caps->deviceId);
        Console.Log("Depth:     {}", caps->homogeneousDepth ? "[-1, 1]" : "[0, 1]");
        Console.Log("Origin Bottom Left: {}", caps->originBottomLeft);
        Console.Log("Num GPUs:  {}", caps->numGPUs);
        Console.Log("Limits:");
        Console.Log("  Draw Calls:          {}", caps->limits.maxDrawCalls);
        Console.Log("  Blit Calls:          {}", caps->limits.maxBlits);
        Console.Log("  Texture Size:        {}", caps->limits.maxTextureSize);
        Console.Log("  Texture Layers:      {}", caps->limits.maxTextureLayers);
        Console.Log("  Views:               {}", caps->limits.maxViews);
        Console.Log("  Frame Buffers:       {}", caps->limits.maxFrameBuffers);
        Console.Log("  FB Attachments:      {}", caps->limits.maxFBAttachments);
        Console.Log("  Program Handles:     {}", caps->limits.maxPrograms);
        Console.Log("  Shader Handles:      {}", caps->limits.maxShaders);
        Console.Log("  Textures:            {}", caps->limits.maxTextures);
        Console.Log("  Texture Samplers:    {}", caps->limits.maxTextureSamplers);
        Console.Log("  Compute Bindings:    {}", caps->limits.maxComputeBindings);
        Console.Log("  Vertex Layouts:      {}", caps->limits.maxVertexLayouts);
        Console.Log("  Vertex Streams:      {}", caps->limits.maxVertexStreams);
        Console.Log("  Index Buffers:       {}", caps->limits.maxIndexBuffers);
        Console.Log("  Vertex Buffers:      {}", caps->limits.maxVertexBuffers);
        Console.Log("  Dynamic IBs:         {}", caps->limits.maxDynamicIndexBuffers);
        Console.Log("  Dynamic VBs:         {}", caps->limits.maxDynamicVertexBuffers);
        Console.Log("  Uniforms:            {}", caps->limits.maxUniforms);
        Console.Log("  Occlusion Queries:   {}", caps->limits.maxOcclusionQueries);
        Console.Log("  Encoder Threads:     {}", caps->limits.maxEncoders);
        Console.Log("  Minimum Resource Command Buffer Size: {}", caps->limits.minResourceCbSize);
        Console.Log("  Transient VB Size:   {}", caps->limits.transientVbSize);
        Console.Log("  Transient IB Size:   {}", caps->limits.transientIbSize);
    });
}