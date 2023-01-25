#pragma once

#include "platform/Window.h"
#include "math/Math.h"
#include "math/Color.h"
#include "render/TextureFormat.h"
#include "render/BlendMode.h"

#include <functional>
#include <string_view>

namespace engine { struct RenderSystem; struct Mesh; struct Texture; }

namespace engine::render
{
    // TODO: Consistent API for GPU child objects and uploading

    struct Shader {};

    struct RenderTarget
    {
        using ReadBackFunc = std::function<void(float* ptr, size_t size, size_t with)>;

        virtual ~RenderTarget() {}

        // TODO: MRT attachment configuration
        virtual void* GetTexture() const = 0;
        virtual void* GetDepthTexture() const = 0;
        virtual uint2 GetSize() const = 0;
        virtual void Resize(uint width, uint height) = 0;

        // TODO: Should be settable as a flag on creation
        virtual void SetReadBack(bool readBack) = 0;
        virtual void ReadTexture(ReadBackFunc func) = 0;
    };

    struct Handle
    {
        virtual void* Value() const = 0;
        virtual ~Handle() {}
    };

    enum class CompareFunc { Disabled, Never, Always, Less, LessEqual, Greater, GreaterEqual, Equal, NotEqual };
    enum class PrimitiveType { Points, Lines, Triangles, TriStrip, LineStrip  };
    enum class PolygonMode { Points, Wireframe, Fill };

    class Render
    {
        friend struct ::engine::RenderSystem;
    protected:
        virtual void Init(Window* window) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Shutdown() = 0;
    public:
        static Render* Create();
        virtual ~Render() {}

    // Resource Uploading //

        virtual void UploadMesh(Mesh* mesh) = 0;
        virtual void UploadTexture(Texture* texture, bool release = true) = 0;

    // Resource Creation and Loading //

        // Simple RT with optional depth
        virtual RenderTarget* CreateRenderTarget(
            uint width, uint height,
            TextureFormat format = TextureFormat::RGBA32F,
            TextureFormat depth = TextureFormat::D32F) = 0;

        virtual Shader* LoadShader(const char* vertexShader, const char* pixelShader) = 0; // TODO: Probably replace this
        virtual Shader* LoadShader(const char* name) { return LoadShader((std::string(name) + ".vs").c_str(), (std::string(name) + ".fs").c_str()); }

    // Per-Camera State //

        // nullptr = default backbuffer
        virtual void SetRenderTarget(RenderTarget* rt) = 0;
        virtual void SetViewTransform(const mat4x4& view, const mat4x4& proj) = 0;
        virtual void SetClearColor(bool clear, Color color = Colors.Black) = 0;
        virtual void SetClearDepth(bool clear, float depth = 1.0f) = 0;

    // Per-Object State //

        virtual void SetDepthTest(CompareFunc func) = 0;
        virtual void SetDepthWrite(bool write) = 0;
        virtual void SetBlendFunc(BlendFunc func) = 0;
        virtual void SetPrimitiveType(PrimitiveType type) = 0;
        virtual void SetPolygonMode(PolygonMode mode) = 0;
        virtual void SetTransform(const mat4x4& matrix) = 0;
        virtual void SetShader(Shader* shader) = 0;
        virtual void SetTexture(uint slot, Texture* texture) = 0;
        // TODO: Hashed strings...
        virtual void SetUniform(std::string_view name, void* value, uint stride = 4, uint count = 1) = 0;
        void SetUniform(std::string_view name, vec4 value, uint count = 1) { SetUniform(name, &value[0], 4, count); };
        void SetUniform(std::string_view name, mat4x4 value, uint count = 1) { SetUniform(name, &value[0][0], 16, count); };

    // Draw Calls //

        virtual void DrawMesh(Mesh* mesh) = 0;

    };
}