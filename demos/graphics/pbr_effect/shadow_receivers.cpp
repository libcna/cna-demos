// SPDX-License-Identifier: MS-PL
// Directional and punctual shadow reception through both rigid and skinned PBR renderer programs.
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
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/PunctualLightEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedPbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
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
    void ConfigureMaterial(TEffect& effect)
    {
        effect.setViewProperty(Matrix::getIdentityProperty());
        effect.setProjectionProperty(Matrix::CreateOrthographic(1.0f, 1.2f, 0.1f, 10.0f));
        effect.setDiffuseColorProperty(Vector3(0.6f, 0.4f, 0.2f));
        effect.setMetallicFactorProperty(0.0f);
        effect.setRoughnessFactorProperty(1.0f);
        effect.setSpecularFactorEXTProperty(0.0f);
        effect.setAmbientLightColorProperty(Vector3(0.1f, 0.2f, 0.3f));
        effect.setEmissiveFactorProperty(Vector3(0.02f, 0.03f, 0.04f));
        effect.setEncodeOutputToSrgbEXTProperty(false);
        effect.getDirectionalLight1Property().setEnabledProperty(false);
        effect.getDirectionalLight2Property().setEnabledProperty(false);
        effect.setLightViewProjectionEXT(Matrix::CreateTranslation(0.0f, 0.0f, 3.0f));
        effect.setShadowDepthBiasEXT(0.0f);
        effect.setShadowFilterRadiusEXT(0);
    }

    template<typename TEffect>
    void SelectPanel(TEffect& effect, int column, Texture2D& directionalShadow, TextureCube& pointShadow)
    {
        const bool directional = column < 2;
        const bool shadowed = column % 2 == 1;
        DirectionalLight& light = effect.getDirectionalLight0Property();
        light.setEnabledProperty(directional);
        light.setDirectionProperty(Vector3(0.0f, 0.0f, -1.0f));
        light.setDiffuseColorProperty(Vector3(3.0f, 3.0f, 3.0f));
        light.setSpecularColorProperty(Vector3::Zero);

        effect.setShadowMapEXT(&directionalShadow);
        effect.setShadowsEnabledEXT(directional && shadowed);
        PunctualLightEXT punctual;
        if (!directional)
        {
            punctual.Kind = PunctualLightKindEXT::Point;
            punctual.Position = Vector3::Zero;
            punctual.DiffuseColor = Vector3(20.0f, 20.0f, 20.0f);
            punctual.Range = 6.0f;
            punctual.ShadowDepthBias = 0.0f;
            if (shadowed)
                punctual.ShadowCube = &pointShadow;
        }
        effect.setPunctualLightEXT(punctual);
    }

    int Brightness(const Color& color)
    {
        return static_cast<int>(color.getRProperty()) + static_cast<int>(color.getGProperty()) +
               static_cast<int>(color.getBProperty());
    }

    class PbrShadowReceiversDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer != "OPENGLES3" && renderer != "OPENGL33")
                Context().Skip("this PBR receiver example requires the EasyGL shadow-sampling path");

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

            directionalShadow_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            pointShadow_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            const Color storedDistance(64, 0, 0, 255);
            directionalShadow_->SetData(&storedDistance, 1);
            for (int face = 0; face < 6; ++face)
                pointShadow_->SetData(static_cast<CubeMapFace>(face), &storedDistance, 1);

            rigid_ = std::make_unique<PbrEffect>(device);
            ConfigureMaterial(*rigid_);
            rigid_->setWorldProperty(Matrix::CreateTranslation(0.0f, 0.0f, -3.0f));
            skinned_ = std::make_unique<SkinnedPbrEffect>(device);
            ConfigureMaterial(*skinned_);
            skinned_->setWorldProperty(Matrix::getIdentityProperty());
            skinned_->setWeightsPerVertexProperty(1);
            skinned_->SetBoneTransforms({Matrix::CreateTranslation(0.0f, 0.0f, -3.0f)});

            Context().Check(rigid_->getShadowMapEXT() == nullptr && !rigid_->isShadowsEnabledEXT() &&
                                rigid_->getPunctualLightEXT().Kind == PunctualLightKindEXT::None,
                            "PbrEffect starts with both optional receiver paths inactive");
            Context().Check(skinned_->getShadowMapEXT() == nullptr && !skinned_->isShadowsEnabledEXT() &&
                                skinned_->getPunctualLightEXT().Kind == PunctualLightKindEXT::None,
                            "SkinnedPbrEffect starts with both optional receiver paths inactive");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const Viewport full = device.getViewportProperty();
            device.Clear(Color::Black);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.setIndicesProperty(indices_.get());

            for (int panel = 0; panel < 8; ++panel)
            {
                const int column = panel % 4;
                const int row = panel / 4;
                const int left = full.getWidthProperty() * column / 4;
                const int right = full.getWidthProperty() * (column + 1) / 4;
                device.setViewportProperty(Viewport(left, row * full.getHeightProperty() / 2,
                                                   right - left, full.getHeightProperty() / 2));
                if (row == 0)
                {
                    SelectPanel(*rigid_, column, *directionalShadow_, *pointShadow_);
                    device.SetVertexBuffer(rigidVertices_.get());
                    for (EffectPass& pass : rigid_->getCurrentTechniqueProperty()->getPassesProperty())
                    {
                        pass.Apply();
                        device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                    }
                }
                else
                {
                    SelectPanel(*skinned_, column, *directionalShadow_, *pointShadow_);
                    device.SetVertexBuffer(skinnedVertices_.get());
                    for (EffectPass& pass : skinned_->getCurrentTechniqueProperty()->getPassesProperty())
                    {
                        pass.Apply();
                        device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                    }
                }
            }
            device.setViewportProperty(full);

            if (FramesDrawn() == 0)
                Check(device);
            Game::Draw(gameTime);
        }

    private:
        void Check(GraphicsDevice& device)
        {
            const CnaDemos::BackBufferPixels pixels(device);
            std::array<Color, 8> colors;
            for (int panel = 0; panel < 8; ++panel)
                colors[panel] = pixels.At(
                    pixels.Width() * (2 * (panel % 4) + 1) / 8,
                    pixels.Height() * (2 * (panel / 4) + 1) / 4);
            Context().Print("rigid directional {}/{} point {}/{}; skinned directional {}/{} point {}/{}",
                            colors[0].ToString(), colors[1].ToString(), colors[2].ToString(),
                            colors[3].ToString(), colors[4].ToString(), colors[5].ToString(),
                            colors[6].ToString(), colors[7].ToString());

            const auto pairMatches = [&](int rigid, int skinned) {
                return CnaDemos::Near(colors[rigid], colors[skinned], 1);
            };
            Context().Check(Brightness(colors[0]) > Brightness(colors[1]) + 80 &&
                                pairMatches(0, 4) && pairMatches(1, 5),
                            "both PBR programs remove directional direct light through the authored depth map");
            Context().Check(Brightness(colors[2]) > Brightness(colors[3]) + 80 &&
                                pairMatches(2, 6) && pairMatches(3, 7),
                            "both PBR programs remove point direct light through the authored distance cube");
            Context().Check(rigid_->getShadowMapEXT() == directionalShadow_.get() &&
                                rigid_->isShadowsEnabledEXT() == false &&
                                rigid_->getPunctualLightEXT().ShadowCube == pointShadow_.get(),
                            "PbrEffect exposes the final selected receiver state");
            Context().Check(skinned_->getShadowMapEXT() == directionalShadow_.get() &&
                                skinned_->isShadowsEnabledEXT() == false &&
                                skinned_->getPunctualLightEXT().ShadowCube == pointShadow_.get(),
                            "SkinnedPbrEffect exposes the final selected receiver state");
        }

        std::unique_ptr<VertexBuffer> rigidVertices_;
        std::unique_ptr<VertexBuffer> skinnedVertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> directionalShadow_;
        std::unique_ptr<TextureCube> pointShadow_;
        std::unique_ptr<PbrEffect> rigid_;
        std::unique_ptr<SkinnedPbrEffect> skinned_;
    };
}

CNA_DEMOS_GAME(PbrShadowReceiversDemo, {
    .id = "graphics.pbr_effect.shadow_receivers",
    .title = "Rigid and skinned PBR shadow receivers",
    .description = "Runs equivalent rigid and one-bone skinned PBR quads through unshadowed and authored-map "
                   "directional and point-light receiver branches.",
    .apis = {"PbrEffect", "SkinnedPbrEffect", "SkinnedPbrEffect::SetBoneTransforms",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowMapEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setShadowsEnabledEXT",
             "Microsoft::Xna::Framework::Graphics::IShadowReceiverEXT::setPunctualLightEXT",
             "Microsoft::Xna::Framework::Graphics::PunctualLightEXT"},
    .tags = {"pbr", "skinning", "shadows", "point-light", "gltf", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "EasyGL OPENGLES3/OPENGL33 stock PBR receiver paths only; other renderers explicitly skipped. "
                    "Authored constant depth/distance maps; detailed filtering, bias, cascades, spot lights and "
                    "shadow generation remain covered by the existing SkinnedEffect receiver scenes.",
})
