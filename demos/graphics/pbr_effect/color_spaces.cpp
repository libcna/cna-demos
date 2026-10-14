// SPDX-License-Identifier: MS-PL
// CNAEXT PbrEffect texture decode and final-output encode controls.
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/ImageBasedLightEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    struct PbrVertex
    {
        Vector3 Position;
        Vector3 Normal;
        Vector4 Tangent;
        Vector2 TextureCoordinate;
    };
    static_assert(std::is_trivially_copyable_v<PbrVertex> && sizeof(PbrVertex) == 48);

    const VertexDeclaration& PbrVertexDeclaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(static_cast<int>(offsetof(PbrVertex, Position)), VertexElementFormat::Vector3,
                          VertexElementUsage::Position, 0),
            VertexElement(static_cast<int>(offsetof(PbrVertex, Normal)), VertexElementFormat::Vector3,
                          VertexElementUsage::Normal, 0),
            VertexElement(static_cast<int>(offsetof(PbrVertex, Tangent)), VertexElementFormat::Vector4,
                          VertexElementUsage::Tangent, 0),
            VertexElement(static_cast<int>(offsetof(PbrVertex, TextureCoordinate)), VertexElementFormat::Vector2,
                          VertexElementUsage::TextureCoordinate, 0),
        };
        return declaration;
    }

    void FillCube(TextureCube& cube, const Color& color)
    {
        for (int face = 0; face < 6; ++face)
            cube.SetData(static_cast<CubeMapFace>(face), &color, 1);
    }

    constexpr std::array<Vector2, 8> PanelPositions = {
        Vector2(-2.7f, 0.75f), Vector2(-0.9f, 0.75f), Vector2(0.9f, 0.75f), Vector2(2.7f, 0.75f),
        Vector2(-2.7f, -0.75f), Vector2(-0.9f, -0.75f), Vector2(0.9f, -0.75f), Vector2(2.7f, -0.75f),
    };

    class ColorSpacesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                Context().Skip(std::string(renderer) + " does not sample PbrEffect color-space state");

            grayMap_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color gray(128, 128, 128, 255);
            grayMap_->SetData(&gray, 1);
            outputMap_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color grayWithCoverage(128, 128, 128, 96);
            outputMap_->SetData(&grayWithCoverage, 1);

            blackIrradiance_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            whiteSpecular_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            FillCube(*blackIrradiance_, Color::Black);
            FillCube(*whiteSpecular_, Color::White);
            brdfLut_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color scaleOneBiasZero(255, 0, 0, 255);
            brdfLut_->SetData(&scaleOneBiasZero, 1);
            specularLight_.Irradiance = blackIrradiance_.get();
            specularLight_.PrefilteredSpecular = whiteSpecular_.get();
            specularLight_.BrdfLut = brdfLut_.get();
            specularLight_.PrefilteredMipCount = 1;
            // IOR2 has F0=1/9. Intensity9 makes the isolated specular result equal to the sampled
            // specular-colour texture, so its transfer curve can be compared directly.
            specularLight_.Intensity = 9.0f;

            constexpr float half = 0.43f;
            const Vector3 normal(0.0f, 0.0f, 1.0f);
            const Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
            quad_ = {PbrVertex{Vector3(-half, half, 0.0f), normal, tangent, Vector2(0.0f, 0.0f)},
                     PbrVertex{Vector3(half, half, 0.0f), normal, tangent, Vector2(1.0f, 0.0f)},
                     PbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, Vector2(0.0f, 1.0f)},
                     PbrVertex{Vector3(half, half, 0.0f), normal, tangent, Vector2(1.0f, 0.0f)},
                     PbrVertex{Vector3(half, -half, 0.0f), normal, tangent, Vector2(1.0f, 1.0f)},
                     PbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, Vector2(0.0f, 1.0f)}};

            effect_ = std::make_unique<PbrEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 7.2f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setMetallicFactorProperty(0.0f);
            effect_->setRoughnessFactorProperty(1.0f);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);

            Context().Check(effect_->getBaseColorTextureIsSrgbEXTProperty() &&
                                effect_->getEmissiveTextureIsSrgbEXTProperty() &&
                                effect_->getSpecularColorTextureIsSrgbEXTProperty() &&
                                effect_->getEncodeOutputToSrgbEXTProperty(),
                            "texture decoders and final output encoding default to enabled");
            effect_->setBaseColorTextureIsSrgbEXTProperty(false);
            effect_->setEmissiveTextureIsSrgbEXTProperty(false);
            effect_->setSpecularColorTextureIsSrgbEXTProperty(false);
            effect_->setEncodeOutputToSrgbEXTProperty(false);
            Context().Check(!effect_->getBaseColorTextureIsSrgbEXTProperty() &&
                                !effect_->getEmissiveTextureIsSrgbEXTProperty() &&
                                !effect_->getSpecularColorTextureIsSrgbEXTProperty() &&
                                !effect_->getEncodeOutputToSrgbEXTProperty(),
                            "all four color-space switches round-trip independently of material values");

            device.getSamplerStatesProperty()(0, SamplerState::PointClamp);
            device.getSamplerStatesProperty()(3, SamplerState::PointClamp);
            device.getSamplerStatesProperty()(6, SamplerState::PointClamp);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(20, 22, 28));
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            for (std::size_t panel = 0; panel < PanelPositions.size(); ++panel)
            {
                const std::size_t path = panel / 2;
                const bool transfer = panel % 2 == 1;

                effect_->setTextureProperty(nullptr);
                effect_->setDiffuseColorProperty(Vector3::One);
                effect_->setAmbientLightColorProperty(Vector3::Zero);
                effect_->setEmissiveMapProperty(nullptr);
                effect_->setEmissiveFactorProperty(Vector3::Zero);
                effect_->setSpecularColorMapEXTProperty(nullptr);
                effect_->setIorEXTProperty(1.5f);
                effect_->setImageBasedLightEXT(ImageBasedLightEXT{});
                effect_->setBaseColorTextureIsSrgbEXTProperty(false);
                effect_->setEmissiveTextureIsSrgbEXTProperty(false);
                effect_->setSpecularColorTextureIsSrgbEXTProperty(false);
                effect_->setEncodeOutputToSrgbEXTProperty(false);

                if (path == 0)
                {
                    effect_->setTextureProperty(grayMap_.get());
                    effect_->setAmbientLightColorProperty(Vector3::One);
                    effect_->setBaseColorTextureIsSrgbEXTProperty(transfer);
                }
                else if (path == 1)
                {
                    effect_->setDiffuseColorProperty(Vector3::Zero);
                    effect_->setEmissiveMapProperty(grayMap_.get());
                    effect_->setEmissiveFactorProperty(Vector3::One);
                    effect_->setEmissiveTextureIsSrgbEXTProperty(transfer);
                }
                else if (path == 2)
                {
                    effect_->setDiffuseColorProperty(Vector3::Zero);
                    effect_->setSpecularColorMapEXTProperty(grayMap_.get());
                    effect_->setSpecularColorTextureIsSrgbEXTProperty(transfer);
                    effect_->setIorEXTProperty(2.0f);
                    effect_->setImageBasedLightEXT(specularLight_);
                }
                else
                {
                    effect_->setTextureProperty(outputMap_.get());
                    effect_->setAmbientLightColorProperty(Vector3::One);
                    effect_->setEncodeOutputToSrgbEXTProperty(transfer);
                }

                effect_->setWorldProperty(Matrix::CreateTranslation(
                    PanelPositions[panel].X, PanelPositions[panel].Y, 0.0f));
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, quad_, 0, 2,
                                              PbrVertexDeclaration());
                }
            }

            if (FramesDrawn() == 0)
                Check(device);
            Game::Draw(gameTime);
        }

    private:
        Point PanelCenter(std::size_t panel)
        {
            const Vector2 position = PanelPositions.at(panel);
            const Vector3 projected = getGraphicsDeviceProperty().getViewportProperty().Project(
                Vector3(position.X, position.Y, 0.0f), effect_->getProjectionProperty(), effect_->getViewProperty(),
                Matrix::getIdentityProperty());
            return Point(static_cast<int>(projected.X), static_cast<int>(projected.Y));
        }

        void Check(GraphicsDevice& device)
        {
            const CnaDemos::BackBufferPixels pixels(device);
            std::array<Color, 8> colors;
            for (std::size_t i = 0; i < colors.size(); ++i)
            {
                const Point p = PanelCenter(i);
                colors[i] = pixels.At(p.X, p.Y);
            }
            Context().Print("centres: base {} / {}, emissive {} / {}, specular {} / {}, output {} / {}",
                            colors[0].ToString(), colors[1].ToString(), colors[2].ToString(),
                            colors[3].ToString(), colors[4].ToString(), colors[5].ToString(),
                            colors[6].ToString(), colors[7].ToString());

            const auto gray = [](const Color& actual, int expected, int alpha = 255) {
                return std::abs(static_cast<int>(actual.getRProperty()) - expected) <= 2 &&
                       std::abs(static_cast<int>(actual.getGProperty()) - expected) <= 2 &&
                       std::abs(static_cast<int>(actual.getBProperty()) - expected) <= 2 &&
                       actual.getAProperty() == alpha;
            };
            Context().Check(gray(colors[0], 128), "a linear base-color sample remains 128");
            Context().Check(gray(colors[1], 55), "an sRGB base-color sample decodes to linear 55");
            Context().Check(gray(colors[2], 128), "a linear emissive sample remains 128");
            Context().Check(gray(colors[3], 55), "an sRGB emissive sample decodes to linear 55");
            Context().Check(gray(colors[4], 128), "a linear specular-color sample remains 128");
            Context().Check(gray(colors[5], 55), "an sRGB specular-color sample decodes to linear 55");
            Context().Check(gray(colors[6], 128, 96), "linear output preserves RGB and alpha");
            Context().Check(gray(colors[7], 188, 96), "output encoding raises linear RGB to sRGB and preserves alpha");
        }

        std::unique_ptr<PbrEffect> effect_;
        std::unique_ptr<Texture2D> grayMap_;
        std::unique_ptr<Texture2D> outputMap_;
        std::unique_ptr<TextureCube> blackIrradiance_;
        std::unique_ptr<TextureCube> whiteSpecular_;
        std::unique_ptr<Texture2D> brdfLut_;
        ImageBasedLightEXT specularLight_;
        std::vector<PbrVertex> quad_;
    };
}

CNA_DEMOS_GAME(ColorSpacesDemo, {
    .id = "graphics.pbr_effect.color_spaces",
    .title = "PBR color spaces",
    .description = "Compares linear and sRGB decoding for base-color, emissive and specular-color "
                   "textures, then shows final linear-to-sRGB output encoding without changing alpha.",
    .apis = {"PbrEffect::BaseColorTextureIsSrgbEXT", "PbrEffect::EmissiveTextureIsSrgbEXT",
             "PbrEffect::SpecularColorTextureIsSrgbEXT", "PbrEffect::EncodeOutputToSrgbEXT",
             "PbrEffect::SpecularColorMapEXT", "ImageBasedLightEXT"},
    .tags = {"3d", "pbr", "texture", "srgb", "linear", "gltf", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Skipped on SOFTWARE and HEADLESS, which do not sample PbrEffect color-space state.",
})
