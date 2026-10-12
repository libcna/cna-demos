// SPDX-License-Identifier: MS-PL
// Clone state, separate programs and disposal under a live device.
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "CNA/Graphics/ShaderCodeEXT.hpp"
#include "CNA/ShaderDialectEXT.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsResource.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/ShaderEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr const char* Vertex = R"(
layout(location = 0) in vec3 aPosition;
uniform mat4 World;
uniform mat4 View;
uniform mat4 Projection;
void main() { gl_Position = Projection * View * World * vec4(aPosition, 1.0); }
)";
    constexpr const char* Fragment = R"(
out vec4 FragColor;
void main() { FragColor = vec4(0.0, 1.0, 1.0, 1.0); }
)";

    class ShaderCloneDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const auto dialect = device.GetShaderDialectEXT();
            const bool es = dialect == CNA::ShaderDialectEXT::GlslEs;
            if (!es && dialect != CNA::ShaderDialectEXT::GlslDesktop)
                Context().Skip("the demo supplies GLSL ES and desktop GLSL only");
            if (!device.ExecutesShaderEffectSourceEXT())
                Context().Skip("the live renderer does not execute shader source");
            const auto language = es ? CNA::ShaderLanguageEXT::GlslEs : CNA::ShaderLanguageEXT::GlslDesktop;
            const std::string header = es ? "#version 300 es\nprecision highp float;\n" : "#version 330 core\n";
            const CNA::Graphics::ShaderCodeEXT vertex(language, CNA::ShaderStageEXT::Vertex,
                                                     "main", "clone-state.vert", header + Vertex);
            const CNA::Graphics::ShaderCodeEXT fragment(language, CNA::ShaderStageEXT::Fragment,
                                                       "main", "clone-state.frag", header + Fragment);
            original_ = std::make_unique<ShaderEffect>(device, vertex, fragment);
            if (!Context().Check(original_->IsEffectValid() && original_->HasRenderer(),
                                 "the original effect compiles a live constant-color program"))
                throw std::runtime_error(original_->GetCompileErrorEXT());
            const Matrix world = Matrix::CreateScale(.6f, .6f, 1) * Matrix::CreateTranslation(-.75f, 0, -3);
            const Matrix view = Matrix::CreateTranslation(.25f, .15f, 0);
            const Matrix projection = Matrix::CreateOrthographic(4, 4, .1f, 10);
            original_->setWorldProperty(world);
            original_->setViewProperty(view);
            original_->setProjectionProperty(projection);
            cloneOwner_.reset(original_->Clone()); // public raw owning return, immediately owned
            clone_ = dynamic_cast<ShaderEffect*>(cloneOwner_.get());
            if (!Context().Check(clone_ && clone_ != original_.get() && clone_->IsEffectValid() &&
                                 clone_->GetSelectedShaderLanguageEXT() == language &&
                                 clone_->getVertexSourceProperty() == original_->getVertexSourceProperty() &&
                                 clone_->getFragmentSourceProperty() == original_->getFragmentSourceProperty(),
                                 "Clone returns an independent compiled ShaderEffect with the same sources and explicit language"))
                throw std::runtime_error("Clone did not return the expected live ShaderEffect");
            Context().Print("World.X: {} -> {}; View.X: {} -> {}; Projection.X scale: {} -> {}",
                            world.M41, clone_->getWorldProperty().M41, view.M41, clone_->getViewProperty().M41,
                            projection.M11, clone_->getProjectionProperty().M11);
            Context().Check(clone_->getWorldProperty() == world && clone_->getViewProperty() == view &&
                            clone_->getProjectionProperty() == projection,
                            "Clone retains the original's current World View and Projection values");
            const Matrix observedCloneWorld = clone_->getWorldProperty();
            const Matrix changed = Matrix::CreateTranslation(2, 1, -4);
            clone_->setWorldProperty(changed);
            Context().Check(clone_->getWorldProperty() == changed && original_->getWorldProperty() == world,
                            "changing the clone's World leaves the original's World unchanged");
            // Restore the observed clone state so rendering still exposes any lost values.
            clone_->setWorldProperty(observedCloneWorld);
            const std::array<VertexPositionNormalTexture, 3> triangle{
                VertexPositionNormalTexture(Vector3(-.5f, -.5f, 0), Vector3::UnitZ, Vector2::Zero),
                VertexPositionNormalTexture(Vector3(.5f, -.5f, 0), Vector3::UnitZ, Vector2::Zero),
                VertexPositionNormalTexture(Vector3(0, .5f, 0), Vector3::UnitZ, Vector2::Zero)};
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionNormalTexture::getVertexDeclarationStatic(),
                                                      3, BufferUsage::WriteOnly);
            vertices_->SetData(triangle.data(), 3);
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            const Viewport full = device.getViewportProperty();
            const int half = full.getWidthProperty() / 2;
            const Color background(24, 28, 36);
            device.Clear(background);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            for (int side = 0; side < 2; ++side)
            {
                if (side == 0 && FramesDrawn() > 0) continue;
                device.setViewportProperty(Viewport(side * half, 0, half, full.getHeightProperty()));
                (side == 0 ? original_.get() : clone_)->Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList, 0, 1);
            }
            device.setViewportProperty(full);
            batch_->Begin();
            batch_->DrawString(*font_, FramesDrawn() == 0 ? "Original" : "Original disposed", Vector2(24, 24), Color::White);
            batch_->DrawString(*font_, "Clone", Vector2(static_cast<float>(half + 24), 24), Color::White);
            batch_->End();
            if (FramesDrawn() < 2)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                std::vector<std::uint32_t> left, right;
                int leftVisible = 0, rightVisible = 0;
                for (int y = full.getHeightProperty() / 5; y < full.getHeightProperty() * 4 / 5; ++y)
                    for (int x = half / 10; x < half * 9 / 10; ++x)
                    {
                        const Color l = pixels.At(x, y), r = pixels.At(x + half, y);
                        left.push_back(l.getPackedValueProperty()); right.push_back(r.getPackedValueProperty());
                        leftVisible += l != background; rightVisible += r != background;
                    }
                if (FramesDrawn() == 0)
                {
                    Context().Check(leftVisible > 0 && rightVisible > 0,
                                    "both separately compiled programs draw visible triangles in equal viewports");
                    Context().Check(left == right,
                                    "an unchanged clone reproduces the original geometry in the matching viewport");
                    clonePixels_ = right;
                    // Invoke public disposal through GraphicsResource; ShaderEffect's protected
                    // bool overload hides the inherited parameterless name in C++.
                    static_cast<GraphicsResource&>(*original_).Dispose();
                    Context().Check(original_->getIsDisposedProperty() && !original_->HasRenderer() &&
                                    clone_->HasRenderer() && clone_->IsEffectValid(),
                                    "disposing the original releases its program while the clone remains valid");
                    bool refused = false;
                    try { std::unique_ptr<Effect> unexpected(original_->Clone()); }
                    catch (const System::ObjectDisposedException&) { refused = true; }
                    Context().Check(refused, "cloning a disposed effect throws ObjectDisposedException");
                }
                else
                    Context().Check(rightVisible > 0 && right == clonePixels_,
                                    "the surviving clone draws the same pixels after original program disposal");
            }
            DemoGame::Draw(time);
        }
        std::unique_ptr<ShaderEffect> original_;
        std::unique_ptr<Effect> cloneOwner_;
        ShaderEffect* clone_ = nullptr; // borrowed from cloneOwner_, not separately owned
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        std::vector<std::uint32_t> clonePixels_;
    };
}

CNA_DEMOS_GAME(ShaderCloneDemo, {
    .id = "graphics.shader_effect.clone_state",
    .title = "Shader clone state and program lifetime",
    .description = "Clones an explicitly compiled shader with nonidentity matrices, compares equal viewport output, "
                   "then disposes the original and checks the surviving clone and disposed-clone refusal.",
    .apis = {"ShaderEffect::Clone", "Effect::Clone", "ShaderEffect::HasRenderer", "ShaderEffect::IsEffectValid",
             "ShaderEffect::GetSelectedShaderLanguageEXT", "IEffectMatrices::World", "IEffectMatrices::View",
             "IEffectMatrices::Projection", "GraphicsResource::Dispose"},
    .tags = {"shader", "clone", "ownership", "matrices", "cnaext"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::CustomEffects,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "GLSL only; nine checks require at least two frames. No raw uniform or texture-binding copy contract is inferred.",
    .knownIssue = "CNA-DEMOS-ISSUE-055",
})
