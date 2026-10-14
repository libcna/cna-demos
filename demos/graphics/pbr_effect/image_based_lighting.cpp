// SPDX-License-Identifier: MS-PL
// CNAEXT PbrEffect image-based lighting: diffuse irradiance, roughness mips, BRDF LUT and intensity.
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/ImageBasedLightEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
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

    void FillCube(TextureCube& cube, const std::vector<Color>& mipColors)
    {
        for (int face = 0; face < 6; ++face)
        {
            int side = cube.getSizeProperty();
            for (int level = 0; level < cube.getLevelCountProperty(); ++level)
            {
                const std::vector<Color> pixels(static_cast<std::size_t>(side * side), mipColors.at(level));
                cube.SetData(static_cast<CubeMapFace>(face), level, nullptr, pixels.data(), 0,
                             static_cast<int>(pixels.size()));
                side = std::max(1, side / 2);
            }
        }
    }

    constexpr std::array<float, 5> PanelX = {-2.8f, -1.4f, 0.0f, 1.4f, 2.8f};

    class ImageBasedLightingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                Context().Skip(std::string(renderer) + " does not sample PbrEffect image-based lighting");

            irradiance_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            FillCube(*irradiance_, {Color::Lime});

            blackSpecular_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            FillCube(*blackSpecular_, {Color::Black});

            // Roughness maps to level 0..2. Solid faces make the selected level observable without
            // depending on cube orientation: a smooth metal is red and a rough metal is blue.
            specular_ = std::make_unique<TextureCube>(device, 4, true, SurfaceFormat::Color);
            FillCube(*specular_, {Color::Red, Color::Magenta, Color::Blue});

            brdfLut_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color brdfScaleBias(255, 0, 0, 255); // scale=1, bias=0
            brdfLut_->SetData(&brdfScaleBias, 1);

            constexpr float half = 0.55f;
            const Vector3 normal(0.0f, 0.0f, 1.0f);
            const Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
            quad_ = {PbrVertex{Vector3(-half, half, 0.0f), normal, tangent, Vector2(0.0f, 0.0f)},
                     PbrVertex{Vector3(half, half, 0.0f), normal, tangent, Vector2(1.0f, 0.0f)},
                     PbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, Vector2(0.0f, 1.0f)},
                     PbrVertex{Vector3(half, half, 0.0f), normal, tangent, Vector2(1.0f, 0.0f)},
                     PbrVertex{Vector3(half, -half, 0.0f), normal, tangent, Vector2(1.0f, 1.0f)},
                     PbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, Vector2(0.0f, 1.0f)}};

            effect_ = std::make_unique<PbrEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 7.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setDiffuseColorProperty(Vector3::One);
            effect_->setEncodeOutputToSrgbEXTProperty(false);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);

            invalid_.Irradiance = irradiance_.get();
            Context().Check(!invalid_.IsValidEXT(), "a partial environment remains invalid");

            diffuse_.Irradiance = irradiance_.get();
            diffuse_.PrefilteredSpecular = blackSpecular_.get();
            diffuse_.BrdfLut = brdfLut_.get();
            diffuse_.PrefilteredMipCount = 1;
            diffuse_.Intensity = 1.0f;
            Context().Check(diffuse_.IsValidEXT(), "a complete diffuse environment is valid");

            glossy_ = diffuse_;
            glossy_.PrefilteredSpecular = specular_.get();
            glossy_.PrefilteredMipCount = specular_->getLevelCountProperty();
            effect_->setImageBasedLightEXT(glossy_);
            const ImageBasedLightEXT& stored = effect_->getImageBasedLightEXT();
            Context().Check(stored.Irradiance == irradiance_.get() &&
                                stored.PrefilteredSpecular == specular_.get() &&
                                stored.BrdfLut == brdfLut_.get() && stored.PrefilteredMipCount == 3 &&
                                stored.Intensity == 1.0f,
                            "PbrEffect retains the complete environment bundle");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(20, 22, 28));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            for (std::size_t panel = 0; panel < PanelX.size(); ++panel)
            {
                ImageBasedLightEXT light;
                if (panel == 0)
                {
                    light = invalid_;
                    effect_->setAmbientLightColorProperty(Vector3(1.0f, 1.0f, 0.0f));
                    effect_->setMetallicFactorProperty(0.0f);
                    effect_->setRoughnessFactorProperty(0.5f);
                }
                else if (panel <= 2)
                {
                    light = diffuse_;
                    light.Intensity = panel == 1 ? 1.0f : 0.25f;
                    effect_->setAmbientLightColorProperty(Vector3(1.0f, 0.0f, 0.0f));
                    effect_->setMetallicFactorProperty(0.0f);
                    effect_->setRoughnessFactorProperty(0.5f);
                }
                else
                {
                    light = glossy_;
                    effect_->setAmbientLightColorProperty(Vector3(1.0f, 0.0f, 0.0f));
                    effect_->setMetallicFactorProperty(1.0f);
                    effect_->setRoughnessFactorProperty(panel == 3 ? 0.0f : 1.0f);
                }
                effect_->setImageBasedLightEXT(light);
                effect_->setWorldProperty(Matrix::CreateTranslation(PanelX[panel], 0.0f, 0.0f));
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
            const Vector3 p = getGraphicsDeviceProperty().getViewportProperty().Project(
                Vector3(PanelX.at(panel), 0.0f, 0.0f), effect_->getProjectionProperty(), effect_->getViewProperty(),
                Matrix::getIdentityProperty());
            return Point(static_cast<int>(p.X), static_cast<int>(p.Y));
        }

        void Check(GraphicsDevice& device)
        {
            const CnaDemos::BackBufferPixels pixels(device);
            std::array<Color, 5> colors;
            for (std::size_t i = 0; i < colors.size(); ++i)
            {
                const Point p = PanelCenter(i);
                colors[i] = pixels.At(p.X, p.Y);
            }
            Context().Print("panel centres: fallback {}, diffuse {}, quarter {}, smooth {}, rough {}",
                            colors[0].ToString(), colors[1].ToString(), colors[2].ToString(),
                            colors[3].ToString(), colors[4].ToString());
            Context().Check(colors[0].getRProperty() > 180 && colors[0].getGProperty() > 180 &&
                                colors[0].getBProperty() < 30,
                            "an invalid bundle leaves the flat yellow ambient term in charge");
            Context().Check(colors[1].getGProperty() > colors[1].getRProperty() + 120 &&
                                colors[1].getGProperty() > colors[1].getBProperty() + 120,
                            "valid green irradiance replaces the red flat ambient term");
            Context().Check(colors[1].getGProperty() > colors[2].getGProperty() + 100 &&
                                colors[2].getGProperty() > 30,
                            "environment intensity scales the sampled indirect light");
            Context().Check(colors[3].getRProperty() > colors[3].getBProperty() + 150,
                            "a smooth metal samples the red first prefiltered mip");
            Context().Check(colors[4].getBProperty() > colors[4].getRProperty() + 150,
                            "a rough metal samples the blue last prefiltered mip");
        }

        std::unique_ptr<PbrEffect> effect_;
        std::unique_ptr<TextureCube> irradiance_;
        std::unique_ptr<TextureCube> blackSpecular_;
        std::unique_ptr<TextureCube> specular_;
        std::unique_ptr<Texture2D> brdfLut_;
        ImageBasedLightEXT invalid_;
        ImageBasedLightEXT diffuse_;
        ImageBasedLightEXT glossy_;
        std::vector<PbrVertex> quad_;
    };
}

CNA_DEMOS_GAME(ImageBasedLightingDemo, {
    .id = "graphics.pbr_effect.image_based_lighting",
    .title = "PBR image-based lighting",
    .description = "Uses a complete ImageBasedLightEXT bundle to replace flat ambient light, scales diffuse "
                   "irradiance by intensity and selects authored specular cube mips from surface roughness.",
    .apis = {"ImageBasedLightEXT", "ImageBasedLightEXT::IsValidEXT", "ImageBasedLightEXT::Irradiance",
             "ImageBasedLightEXT::PrefilteredSpecular", "ImageBasedLightEXT::BrdfLut",
             "ImageBasedLightEXT::PrefilteredMipCount", "ImageBasedLightEXT::Intensity",
             "PbrEffect::ImageBasedLightEXT", "PbrEffect::MetallicFactor", "PbrEffect::RoughnessFactor",
             "TextureCube::SetData", "Texture2D::SetData"},
    .tags = {"3d", "pbr", "ibl", "irradiance", "cubemap", "roughness", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Skipped on SOFTWARE and HEADLESS, which do not sample PbrEffect image-based lighting.",
})
