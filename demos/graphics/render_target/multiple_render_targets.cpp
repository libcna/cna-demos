// SPDX-License-Identifier: MS-PL
// SetRenderTargets with several RenderTargetBindings: one draw fills two textures at once (MRT).
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CNA/ShaderDialectEXT.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ColorWriteChannels.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetBinding.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using CNA::ShaderDialectEXT;

namespace
{
    constexpr int TargetSize = 360;

    constexpr const char* VertexBody = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
uniform mat4 World;
uniform mat4 View;
uniform mat4 Projection;
out vec3 vNormal;
void main()
{
    gl_Position = Projection * View * World * vec4(aPosition, 1.0);
    vNormal = mat3(World) * aNormal;
}
)";

    // Two outputs: output 0 goes to the first bound target, output 1 to the second -- the
    // surface colour and the surface normal, as a deferred renderer's G-buffer stores them.
    constexpr const char* FragmentBody = R"(
in vec3 vNormal;
layout(location = 0) out vec4 Albedo;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec4 Material;
layout(location = 3) out vec4 Emissive;
uniform vec3 uColor;
void main()
{
    Albedo = vec4(uColor, 1.0);
    Normal = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0);
    Material = vec4(0.2, 0.4, 0.6, 0.8);
    Emissive = vec4(0.7, 0.5, 0.3, 0.1);
}
)";

    class MultipleRenderTargetsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const ShaderDialectEXT dialect = device.GetShaderDialectEXT();
            std::string header;
            if (dialect == ShaderDialectEXT::GlslEs)
                header = "#version 300 es\nprecision mediump float;\n";
            else if (dialect == ShaderDialectEXT::GlslDesktop)
                header = "#version 330 core\n";
            else
                Context().Skip("this demo carries GLSL; the renderer wants another shading language");
            if (!device.ExecutesShaderEffectSourceEXT())
                Context().Skip("this renderer accepts shader source but does not execute it");

            effect_ = std::make_unique<ShaderEffect>(device, header + VertexBody, header + FragmentBody);
            Context().Check(effect_->IsEffectValid(), "the GLSL with two outputs compiled and linked");
            if (!effect_->IsEffectValid())
                Context().Print("compiler log:\n{}", effect_->GetCompileErrorEXT());
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 4.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(
                Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, 1.0f, 0.1f, 100.0f));

            // The targets of one SetRenderTargets call share their size.
            albedo_ = std::make_unique<RenderTarget2D>(device, TargetSize, TargetSize);
            normals_ = std::make_unique<RenderTarget2D>(device, TargetSize, TargetSize);
            material_ = std::make_unique<RenderTarget2D>(device, TargetSize, TargetSize);
            emissive_ = std::make_unique<RenderTarget2D>(device, TargetSize, TargetSize);
            maskedWrites_.setColorWriteChannels1Property(ColorWriteChannels::Red);
            maskedWrites_.setColorWriteChannels2Property(ColorWriteChannels::Green);
            maskedWrites_.setColorWriteChannels3Property(ColorWriteChannels::Blue);
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            const CnaDemos::Mesh sphere = CnaDemos::MakeSphere(1.4f);
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionNormalTexture::getVertexDeclarationStatic(),
                                                       sphere.VertexCount(), BufferUsage::WriteOnly);
            vertices_->SetData(sphere.vertices.data(), sphere.VertexCount());
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits,
                                                     static_cast<int>(sphere.indices.size()), BufferUsage::WriteOnly);
            indices_->SetData(sphere.indices.data(), static_cast<int>(sphere.indices.size()));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            if (FramesDrawn() == 0)
            {
                // Four active outputs isolate the three indexed write masks. The clear colour is
                // deliberately different in every channel, so a disabled channel is observable.
                device.SetRenderTargets({RenderTargetBinding(albedo_.get()), RenderTargetBinding(normals_.get()),
                                         RenderTargetBinding(material_.get()), RenderTargetBinding(emissive_.get())});
                device.Clear(Color(10, 20, 30, 40));
                device.setBlendStateProperty(maskedWrites_);
                device.setDepthStencilStateProperty(DepthStencilState::None);
                device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
                DrawSphere(device);
                device.SetRenderTarget(nullptr);

                const Color slot1 = Center(*normals_);
                const Color slot2 = Center(*material_);
                const Color slot3 = Center(*emissive_);
                Context().Print("indexed masks: slot1 {}, slot2 {}, slot3 {}", slot1.ToString(), slot2.ToString(),
                                slot3.ToString());
                Context().Check(CnaDemos::Near(slot1, Color(128, 20, 30, 40), 3),
                                "ColorWriteChannels1 writes red and preserves slot 1 green, blue and alpha");
                Context().Check(CnaDemos::Near(slot2, Color(10, 102, 30, 40), 3),
                                "ColorWriteChannels2 writes green and preserves slot 2 red, blue and alpha");
                Context().Check(CnaDemos::Near(slot3, Color(10, 20, 77, 40), 3),
                                "ColorWriteChannels3 writes blue and preserves slot 3 red, green and alpha");
            }

            // Bind both targets; Clear clears every bound target.
            const RenderTargetBinding albedoBinding(albedo_.get());
            const RenderTargetBinding normalBinding(normals_.get());
            device.SetRenderTargets({albedoBinding, normalBinding});
            if (FramesDrawn() == 0)
            {
                const auto bindings = device.GetRenderTargets();
                Context().Check(bindings.size() == 2 &&
                                    albedoBinding.getRenderTargetProperty() == albedo_.get() &&
                                    bindings[0].getRenderTargetProperty() == albedo_.get() &&
                                    bindings[1].getRenderTargetProperty() == normals_.get(),
                                "bindings and GetRenderTargets preserve both exact target identities");
            }
            device.Clear(Color::Black);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            DrawSphere(device);

            // Back to the back buffer: show the two textures the one draw produced.
            device.SetRenderTarget(nullptr);
            device.Clear(Color(24, 24, 32));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*albedo_, Vector2(30.0f, 60.0f), Color::White);
            spriteBatch_->Draw(*normals_, Vector2(410.0f, 60.0f), Color::White);
            spriteBatch_->DrawString(*font_, "target 0: colour", Vector2(30.0f, 24.0f), Color::White);
            spriteBatch_->DrawString(*font_, "target 1: normal", Vector2(410.0f, 24.0f), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                const int c = TargetSize / 2;
                const Color albedo = p.At(30 + c, 60 + c);
                const Color facing = p.At(410 + c, 60 + c);   // normal (0, 0, 1)
                const Color left = p.At(410 + c - 120, 60 + c); // normal pointing left: little red
                Context().Print("albedo {}, normal facing the camera {}, normal on the left {}", albedo.ToString(),
                                facing.ToString(), left.ToString());
                Context().Check(CnaDemos::Near(albedo, Color(230, 140, 26), 3), "target 0 holds the colour");
                Context().Check(CnaDemos::Near(facing, Color(128, 128, 255), 3),
                                "target 1 holds the normal: (0, 0, 1) encoded as (128, 128, 255)");
                Context().Check(left.getRProperty() < 60, "the normals turn left towards the sphere's left edge");
            }
            Game::Draw(gameTime);
        }

    private:
        void DrawSphere(GraphicsDevice& device)
        {
            effect_->Apply();
            effect_->SetUniformVec3("uColor", 0.9f, 0.55f, 0.1f);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, vertices_->getVertexCountProperty(), 0,
                                         indices_->getIndexCountProperty() / 3);
        }

        static Color Center(const RenderTarget2D& target)
        {
            const Rectangle centre(TargetSize / 2, TargetSize / 2, 1, 1);
            Color pixel;
            target.GetData(0, &centre, &pixel, 0, 1);
            return pixel;
        }

        std::unique_ptr<ShaderEffect> effect_;
        std::unique_ptr<RenderTarget2D> albedo_;
        std::unique_ptr<RenderTarget2D> normals_;
        std::unique_ptr<RenderTarget2D> material_;
        std::unique_ptr<RenderTarget2D> emissive_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        BlendState maskedWrites_;
    };
}

CNA_DEMOS_GAME(MultipleRenderTargetsDemo, {
    .id = "graphics.render_target.multiple_render_targets",
    .title = "Multiple render targets",
    .description = "Binds four RenderTarget2Ds to prove per-slot write masks, then uses two of them as a G-buffer: "
                   "one ShaderEffect draw writes the sphere's colour to the first and its normals to the second.",
    .apis = {"GraphicsDevice::SetRenderTargets", "GraphicsDevice::GetRenderTargets", "RenderTargetBinding",
             "RenderTargetBinding::RenderTargetBinding", "RenderTargetBinding::RenderTarget",
             "RenderTarget2D::RenderTarget2D", "GraphicsDevice::SetRenderTarget",
             "ShaderEffect::ShaderEffect", "BlendState::ColorWriteChannels1",
             "BlendState::ColorWriteChannels2", "BlendState::ColorWriteChannels3"},
    .tags = {"3d", "render-target", "mrt", "deferred", "g-buffer", "shader"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects | CnaDemos::Need::MultipleRenderTargets,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL only: renderers that take SPIR-V, HLSL, MSL or WGSL skip it.",
})
