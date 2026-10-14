// SPDX-License-Identifier: MS-PL
// Linear fog follows rigid World depth and skinned bone depth through both PBR programs.
#include <array>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedPbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTangentTexture.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTangentTextureSkinned.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    template<typename TEffect>
    void Configure(TEffect& effect)
    {
        effect.setViewProperty(Matrix::getIdentityProperty());
        effect.setProjectionProperty(Matrix::CreateOrthographic(1.0f, 1.2f, 0.1f, 10.0f));
        effect.setDiffuseColorProperty(Vector3(1.0f, 0.0f, 0.0f));
        effect.setMetallicFactorProperty(0.0f);
        effect.setRoughnessFactorProperty(1.0f);
        effect.setSpecularFactorEXTProperty(0.0f);
        effect.setAmbientLightColorProperty(Vector3::One);
        effect.setEmissiveFactorProperty(Vector3::Zero);
        effect.setEncodeOutputToSrgbEXTProperty(false);
        effect.getDirectionalLight0Property().setEnabledProperty(false);
        effect.getDirectionalLight1Property().setEnabledProperty(false);
        effect.getDirectionalLight2Property().setEnabledProperty(false);
        effect.setFogColorProperty(Vector3(0.0f, 0.0f, 1.0f));
        effect.setFogStartProperty(2.0f);
        effect.setFogEndProperty(6.0f);
    }

    class PbrFogDepthDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer != "OPENGLES3" && renderer != "OPENGL33")
                Context().Skip("this PBR fog example requires the EasyGL stock PBR path");

            const std::array<Vector2, 4> positions = {
                Vector2(-0.35f, -0.36f), Vector2(0.35f, -0.36f),
                Vector2(0.35f, 0.36f), Vector2(-0.35f, 0.36f),
            };
            std::vector<VertexPositionNormalTangentTexture> rigidVertices;
            std::vector<VertexPositionNormalTangentTextureSkinned> skinnedVertices;
            for (const Vector2& position : positions)
            {
                const Vector3 xyz(position.X, position.Y, 0.0f);
                const Vector3 normal(0.0f, 0.0f, 1.0f);
                const Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
                rigidVertices.emplace_back(xyz, normal, tangent, Vector2::Zero);
                skinnedVertices.emplace_back(xyz, normal, tangent, Vector2::Zero,
                                             Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                                             std::array<std::uint8_t, 4>{0, 0, 0, 0});
            }
            rigidVertices_ = std::make_unique<VertexBuffer>(
                device, VertexPositionNormalTangentTexture::getVertexDeclarationStatic(), 4,
                BufferUsage::WriteOnly);
            rigidVertices_->SetData(rigidVertices.data(), 4);
            skinnedVertices_ = std::make_unique<VertexBuffer>(
                device, VertexPositionNormalTangentTextureSkinned::getVertexDeclarationStatic(), 4,
                BufferUsage::WriteOnly);
            skinnedVertices_->SetData(skinnedVertices.data(), 4);
            indices_ = std::make_unique<IndexBuffer>(
                device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            const std::array<std::uint16_t, 6> triangles{0, 1, 2, 0, 2, 3};
            indices_->SetData(triangles.data(), 6);

            rigid_ = std::make_unique<PbrEffect>(device);
            skinned_ = std::make_unique<SkinnedPbrEffect>(device);
            Configure(*rigid_);
            Configure(*skinned_);
            skinned_->setWeightsPerVertexProperty(1);
            Context().Check(!rigid_->getFogEnabledProperty() &&
                                rigid_->getFogColorProperty() == Vector3(0.0f, 0.0f, 1.0f) &&
                                rigid_->getFogStartProperty() == 2.0f && rigid_->getFogEndProperty() == 6.0f,
                            "PbrEffect retains the complete authored fog interval");
            Context().Check(!skinned_->getFogEnabledProperty() &&
                                skinned_->getFogColorProperty() == Vector3(0.0f, 0.0f, 1.0f) &&
                                skinned_->getFogStartProperty() == 2.0f && skinned_->getFogEndProperty() == 6.0f,
                            "SkinnedPbrEffect retains the complete authored fog interval");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.setIndicesProperty(indices_.get());

            if (FramesDrawn() == 0)
            {
                rigid_->setFogEnabledProperty(false);
                skinned_->setFogEnabledProperty(false);
                DrawPanels(device);
                CheckPlain(device);
            }

            rigid_->setFogEnabledProperty(true);
            skinned_->setFogEnabledProperty(true);
            DrawPanels(device);
            if (FramesDrawn() == 0)
                CheckFog(device);
            Game::Draw(gameTime);
        }

    private:
        void DrawPanels(GraphicsDevice& device)
        {
            const Viewport full = device.getViewportProperty();
            for (int panel = 0; panel < 6; ++panel)
            {
                const int column = panel % 3;
                const int row = panel / 3;
                const int left = full.getWidthProperty() * column / 3;
                const int right = full.getWidthProperty() * (column + 1) / 3;
                device.setViewportProperty(Viewport(left, row * full.getHeightProperty() / 2,
                                                   right - left, full.getHeightProperty() / 2));
                const float depth = 2.0f * static_cast<float>(column + 1);
                if (row == 0)
                {
                    rigid_->setWorldProperty(Matrix::CreateTranslation(0.0f, 0.0f, -depth));
                    device.SetVertexBuffer(rigidVertices_.get());
                    for (EffectPass& pass : rigid_->getCurrentTechniqueProperty()->getPassesProperty())
                    {
                        pass.Apply();
                        device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                    }
                }
                else
                {
                    skinned_->setWorldProperty(Matrix::getIdentityProperty());
                    skinned_->SetBoneTransforms({Matrix::CreateTranslation(0.0f, 0.0f, -depth)});
                    device.SetVertexBuffer(skinnedVertices_.get());
                    for (EffectPass& pass : skinned_->getCurrentTechniqueProperty()->getPassesProperty())
                    {
                        pass.Apply();
                        device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                    }
                }
            }
            device.setViewportProperty(full);
        }

        std::array<Color, 6> Centers(GraphicsDevice& device)
        {
            const CnaDemos::BackBufferPixels pixels(device);
            std::array<Color, 6> colors;
            for (int panel = 0; panel < 6; ++panel)
                colors[panel] = pixels.At(
                    pixels.Width() * (2 * (panel % 3) + 1) / 6,
                    pixels.Height() * (2 * (panel / 3) + 1) / 4);
            return colors;
        }

        void CheckPlain(GraphicsDevice& device)
        {
            const auto colors = Centers(device);
            Context().Check(colors[0] == Color::Red && colors[1] == Color::Red && colors[2] == Color::Red,
                            "disabled fog leaves all three rigid depths red");
            Context().Check(colors[3] == Color::Red && colors[4] == Color::Red && colors[5] == Color::Red,
                            "disabled fog leaves all three skinned depths red");
        }

        void CheckFog(GraphicsDevice& device)
        {
            const auto colors = Centers(device);
            Context().Print("rigid fog start/middle/end {}/{}/{}; skinned {}/{}/{}",
                            colors[0].ToString(), colors[1].ToString(), colors[2].ToString(),
                            colors[3].ToString(), colors[4].ToString(), colors[5].ToString());
            const std::array<Color, 3> expected{Color::Red, Color(128, 0, 128), Color::Blue};
            for (int column = 0; column < 3; ++column)
            {
                Context().Check(CnaDemos::Near(colors[column], expected[column], 1),
                                column == 0 ? "rigid World depth at FogStart remains material red" :
                                column == 1 ? "rigid World depth halfway through fog blends red and blue equally" :
                                              "rigid World depth at FogEnd becomes fog blue");
                Context().Check(CnaDemos::Near(colors[column + 3], expected[column], 1),
                                column == 0 ? "skinned bone depth at FogStart remains material red" :
                                column == 1 ? "skinned bone depth halfway through fog blends red and blue equally" :
                                              "skinned bone depth at FogEnd becomes fog blue");
            }
        }

        std::unique_ptr<VertexBuffer> rigidVertices_;
        std::unique_ptr<VertexBuffer> skinnedVertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<PbrEffect> rigid_;
        std::unique_ptr<SkinnedPbrEffect> skinned_;
    };
}

CNA_DEMOS_GAME(PbrFogDepthDemo, {
    .id = "graphics.pbr_effect.fog_depth",
    .title = "Fog through rigid and skinned PBR depth",
    .description = "Moves equivalent rigid and one-bone skinned PBR quads to FogStart, the interval midpoint and "
                   "FogEnd, then checks exact red-to-blue linear fog in both renderer programs.",
    .apis = {"PbrEffect::FogEnabled", "PbrEffect::FogColor", "PbrEffect::FogStart", "PbrEffect::FogEnd",
             "SkinnedPbrEffect::FogEnabled", "SkinnedPbrEffect::FogColor", "SkinnedPbrEffect::FogStart",
             "SkinnedPbrEffect::FogEnd", "SkinnedPbrEffect::SetBoneTransforms"},
    .tags = {"pbr", "skinning", "fog", "depth", "gltf", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "EasyGL OPENGLES3/OPENGL33 stock PBR paths only; other renderers explicitly skipped. "
                    "Uses orthographic view-space depth and linear LDR fog; volumetric, height and HDR fog are "
                    "outside the effect's public contract.",
})
