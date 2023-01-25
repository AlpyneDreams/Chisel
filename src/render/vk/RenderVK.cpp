#include "common/Common.h"
#include "platform/Platform.h"

#include <vku/vku_framework.hpp>

#include <vector>
#include <stdexcept>

#include "platform/Window.h"
#include "render/Render.h"

namespace engine::render
{   
    class RenderVK final : public Render
    {
        struct RenderState {
            uint width, height;
            /*bgfx::ViewId clearView;

            struct ClearState {
                uint16 flags = BGFX_CLEAR_NONE;
                uint32 rgba  = 0x000000FF;
                float depth = 1.0f;
                uint8 stencil = 0;
            } clear;*/
        } state;

        vku::Framework  fw;
        vk::Device      device;
        vku::Window     window;

        vku::ShaderModule   vert;
        vku::ShaderModule   frag;
        vk::UniquePipeline  pipeline;
        vk::UniquePipelineLayout    pipelineLayout;

        struct Vertex { vec2 pos; vec3 color; };

        const std::vector<Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        vku::HostVertexBuffer buffer;

    protected:
        void Init(Window* win)
        {
            fw = vku::Framework(win->GetTitle());
            if (!fw.ok())
                throw std::runtime_error("[VKU] Failed to create framework!");

            device = fw.device();

            // Attach to window surface
            {
                auto display = (Display*)win->GetNativeDisplay();
                auto x11win = (::Window)win->GetNativeWindow();
                auto info = vk::XlibSurfaceCreateInfoKHR{{}, display, x11win};
                auto surface = fw.instance().createXlibSurfaceKHR(info);
                window = vku::Window {fw.instance(), device, fw.physicalDevice(), fw.graphicsQueueFamilyIndex(), surface};
                if (!window.ok())
                    throw std::runtime_error("[VKU] Failed to attach to window.");
            }

            vert = vku::ShaderModule(device, "./core/shaders/spirv/triangle.vert.spv");
            frag = vku::ShaderModule(device, "./core/shaders/spirv/triangle.frag.spv");

            vku::PipelineLayoutMaker plm;
            pipelineLayout = plm.createUnique(device);
            
            buffer = vku::HostVertexBuffer(device, fw.memprops(), vertices);

            auto buildPipeline = [&]() {
                // Make a pipeline to use the vertex format and shaders.
                vku::PipelineMaker pm{window.width(), window.height()};
                pm.shader(vk::ShaderStageFlagBits::eVertex, vert);
                pm.shader(vk::ShaderStageFlagBits::eFragment, frag);
                pm.vertexBinding(0, (uint32_t)sizeof(Vertex));
                pm.vertexAttribute(0, 0, vk::Format::eR32G32Sfloat,
                                    (uint32_t)offsetof(Vertex, pos));
                pm.vertexAttribute(1, 0, vk::Format::eR32G32B32Sfloat,
                                    (uint32_t)offsetof(Vertex, color));

                // Create a pipeline using a renderPass built for our window.
                auto renderPass = window.renderPass();
                auto cache = fw.pipelineCache();

                return pm.createUnique(device, cache, *pipelineLayout, renderPass);
            };

            pipeline = buildPipeline();
            // We only need to create the command buffer(s) once.
            // This simple function lets us do that.
            window.setStaticCommands([&, this](
                                        vk::CommandBuffer cb, int imageIndex,
                                        vk::RenderPassBeginInfo &rpbi) {
                static auto ww = window.width();
                static auto wh = window.height();
                if (ww != window.width() || wh != window.height()) {
                    ww = window.width();
                    wh = window.height();
                    pipeline = buildPipeline();
                }
                vk::CommandBufferBeginInfo bi{};
                cb.begin(bi);
                cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);
                cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
                cb.bindVertexBuffers(0, buffer.buffer(), vk::DeviceSize(0));
                cb.draw(3, 1, 0, 0);
                cb.endRenderPass();
                cb.end();
            });
            

            

            /*// Signal to BGFX not to create a render thread
            bgfx::renderFrame();
            
            bgfx::Init init;

            init.platformData.ndt = window->GetNativeDisplay();
            init.platformData.nwh = window->GetNativeWindow();

            auto [width, height] = window->GetSize();
            init.resolution.width = uint32(width);
            init.resolution.height = uint32(height);

            init.resolution.reset = BGFX_RESET_VSYNC;

            state.width = width, state.height = height;

            // Handle window resize
            window->SetResizeCallback([=, this](uint width, uint height) {
                bgfx::reset(uint32(width), uint32(height), BGFX_RESET_VSYNC);
                bgfx::setViewRect(state.clearView, 0, 0, bgfx::BackbufferRatio::Equal);
            });

            if (!bgfx::init(init)) {
                throw std::runtime_error("[BGFX] Failed to initialize!");
            }

            state.clearView = 0;
            bgfx::setViewRect(state.clearView, 0, 0, bgfx::BackbufferRatio::Equal);*/
        }

        void BeginFrame()
        {
            // Always clear this view even if no draw calls are made
            //bgfx::touch(state.clearView);
        }

        void EndFrame()
        {
            window.draw(device, fw.graphicsQueue());

            // Very crude method to prevent your GPU from overheating.
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        void Shutdown()
        {
            device.waitIdle();
        }

    public:

        float GetAspectRatio()
        {
            return float(state.width) / float(state.height);
        }

        void SetViewTransform(mat4x4& view, mat4x4& proj)
        {
            //bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
        }

        void SetTransform(mat4x4& matrix)
        {
            //bgfx::setTransform(&matrix[0][0]);
        }

        static inline void UpdateClearState(const RenderState& state)
        {
            //bgfx::setViewClear(state.clearView, state.clear.flags, state.clear.rgba, state.clear.depth, state.clear.stencil);
        }

        void SetClearColor(bool clear, Color color)
        {
            /*if (clear)  state.clear.flags |= BGFX_CLEAR_COLOR;
            else        state.clear.flags &= ~BGFX_CLEAR_COLOR;
            state.clear.rgba = color.Pack();
            UpdateClearState(state);*/
        }

        void SetClearDepth(bool clear, float depth)
        {
            /*if (clear)  state.clear.flags |= BGFX_CLEAR_DEPTH;
            else        state.clear.flags &= ~BGFX_CLEAR_COLOR;
            state.clear.depth = depth;
            UpdateClearState(state);*/
        }
    };
    
    Render* Render::Create()
    {
        return new RenderVK();
    }

}