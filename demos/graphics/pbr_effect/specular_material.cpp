// SPDX-License-Identifier: MS-PL
// CNAEXT KHR_materials_ior/specular: dielectric Fresnel factors and both optional texture maps.
#include <array>
#include <cstddef>
#include <cstdint>
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
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureTransformEXT.hpp"
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
        Vector2 TextureCoordinate0;
        Vector2 TextureCoordinate1;
        std::uint8_t R = 255;
        std::uint8_t G = 255;
        std::uint8_t B = 255;
        std::uint8_t A = 255;
    };
    static_assert(std::is_trivially_copyable_v<PbrVertex> && sizeof(PbrVertex) == 60);

    const VertexDeclaration& PbrVertexDeclaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(static_cast<int>(offsetof(PbrVertex, Position)), VertexElementFormat::Vector3,
                          VertexElementUsage::Position, 0),
            VertexElement(static_cast<int>(offsetof(PbrVertex, Normal)), VertexElementFormat::Vector3,
                          VertexElementUsage::Normal, 0),
            VertexElement(static_cast<int>(offsetof(PbrVertex, Tangent)), VertexElementFormat::Vector4,
                          VertexElementUsage::Tangent, 0),
            VertexElement(static_cast<int>(offsetof(PbrVertex, TextureCoordinate0)), VertexElementFormat::Vector2,
                          VertexElementUsage::TextureCoordinate, 0),
            VertexElement(static_cast<int>(offsetof(PbrVertex, TextureCoordinate1)), VertexElementFormat::Vector2,
                          VertexElementUsage::TextureCoordinate, 1),
            VertexElement(static_cast<int>(offsetof(PbrVertex, R)), VertexElementFormat::Color,
                          VertexElementUsage::Color, 0),
        };
        return declaration;
    }

    void FillCube(TextureCube& cube, const Color& color)
    {
        for (int face = 0; face < 6; ++face)
            cube.SetData(static_cast<CubeMapFace>(face), &color, 1);
    }

    constexpr std::array<Vector2, 10> PanelPositions = {
        Vector2(-3.0f, 0.75f), Vector2(-1.5f, 0.75f), Vector2(0.0f, 0.75f),
        Vector2(1.5f, 0.75f), Vector2(3.0f, 0.75f),
        Vector2(-3.0f, -0.75f), Vector2(-1.5f, -0.75f), Vector2(0.0f, -0.75f),
        Vector2(1.5f, -0.75f), Vector2(3.0f, -0.75f),
    };

    const TextureTransformEXT& AuthoredTransform()
    {
        static const TextureTransformEXT transform{
            Vector2(0.875f, 0.125f), Vector2(0.5f, 0.5f), MathHelper::PiOver2};
        return transform;
    }

    class SpecularMaterialDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                Context().Skip(std::string(renderer) + " does not sample PbrEffect specular material state");

            irradiance_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            specular_ = std::make_unique<TextureCube>(device, 1, false, SurfaceFormat::Color);
            FillCube(*irradiance_, Color::Black);
            FillCube(*specular_, Color::White);

            brdfLut_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color brdfScaleBias(255, 0, 0, 255);
            brdfLut_->SetData(&brdfScaleBias, 1);
            strengthMap_ = std::make_unique<Texture2D>(device, 2, 1, false, SurfaceFormat::Color);
            const std::array<Color, 2> strengths = {
                Color(255, 255, 255, 128), Color(255, 255, 255, 64)};
            strengthMap_->SetData(strengths.data(), static_cast<int>(strengths.size()));
            colorMap_ = std::make_unique<Texture2D>(device, 2, 1, false, SurfaceFormat::Color);
            const std::array<Color, 2> colors = {Color::Blue, Color(0, 255, 0)};
            colorMap_->SetData(colors.data(), static_cast<int>(colors.size()));

            ImageBasedLightEXT light;
            light.Irradiance = irradiance_.get();
            light.PrefilteredSpecular = specular_.get();
            light.BrdfLut = brdfLut_.get();
            light.Intensity = 4.0f;

            effect_ = std::make_unique<PbrEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setDiffuseColorProperty(Vector3::Zero);
            effect_->setMetallicFactorProperty(0.0f);
            effect_->setRoughnessFactorProperty(0.5f);
            effect_->setEncodeOutputToSrgbEXTProperty(false);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            effect_->setImageBasedLightEXT(light);

            Context().Check(effect_->getIorEXTProperty() == 1.5f &&
                                effect_->getSpecularFactorEXTProperty() == 1.0f &&
                                effect_->getSpecularColorFactorEXTProperty() == Vector3::One &&
                                effect_->getSpecularMapEXTProperty() == nullptr &&
                                effect_->getSpecularColorMapEXTProperty() == nullptr &&
                                effect_->getSpecularTextureCoordinateSetEXTProperty() == 0 &&
                                effect_->getSpecularColorTextureCoordinateSetEXTProperty() == 0 &&
                                effect_->getSpecularTextureTransformEXTProperty() == TextureTransformEXT{} &&
                                effect_->getSpecularColorTextureTransformEXTProperty() == TextureTransformEXT{},
                            "core glTF dielectric specular defaults are neutral");
            effect_->setIorEXTProperty(2.0f);
            effect_->setSpecularFactorEXTProperty(0.5f);
            effect_->setSpecularColorFactorEXTProperty(Vector3(1.0f, 0.25f, 0.5f));
            effect_->setSpecularMapEXTProperty(strengthMap_.get());
            effect_->setSpecularColorMapEXTProperty(colorMap_.get());
            effect_->setSpecularTextureCoordinateSetEXTProperty(1);
            effect_->setSpecularColorTextureCoordinateSetEXTProperty(1);
            effect_->setSpecularTextureTransformEXTProperty(AuthoredTransform());
            effect_->setSpecularColorTextureTransformEXTProperty(AuthoredTransform());
            Context().Check(effect_->getIorEXTProperty() == 2.0f &&
                                effect_->getSpecularFactorEXTProperty() == 0.5f &&
                                effect_->getSpecularColorFactorEXTProperty() == Vector3(1.0f, 0.25f, 0.5f) &&
                                effect_->getSpecularMapEXTProperty() == strengthMap_.get() &&
                                effect_->getSpecularColorMapEXTProperty() == colorMap_.get() &&
                                effect_->getSpecularTextureCoordinateSetEXTProperty() == 1 &&
                                effect_->getSpecularColorTextureCoordinateSetEXTProperty() == 1 &&
                                effect_->getSpecularTextureTransformEXTProperty() == AuthoredTransform() &&
                                effect_->getSpecularColorTextureTransformEXTProperty() == AuthoredTransform(),
                            "factors, maps, selectors and transforms round-trip together");

            constexpr float half = 0.35f;
            const Vector3 normal(0.0f, 0.0f, 1.0f);
            const Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
            const Vector2 uv0(0.25f, 0.25f), uv1(0.75f, 0.25f);
            quad_ = {PbrVertex{Vector3(-half, half, 0.0f), normal, tangent, uv0, uv1},
                     PbrVertex{Vector3(half, half, 0.0f), normal, tangent, uv0, uv1},
                     PbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, uv0, uv1},
                     PbrVertex{Vector3(half, half, 0.0f), normal, tangent, uv0, uv1},
                     PbrVertex{Vector3(half, -half, 0.0f), normal, tangent, uv0, uv1},
                     PbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, uv0, uv1}};

            device.getSamplerStatesProperty()(5, SamplerState::PointClamp);
            device.getSamplerStatesProperty()(6, SamplerState::PointClamp);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(20, 22, 28));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            for (std::size_t panel = 0; panel < PanelPositions.size(); ++panel)
            {
                effect_->setIorEXTProperty(panel == 1 ? 2.0f : 1.5f);
                effect_->setSpecularFactorEXTProperty(panel == 2 ? 0.0f : 1.0f);
                effect_->setSpecularColorFactorEXTProperty(panel == 3 ? Vector3(1.0f, 0.0f, 0.0f)
                                                                         : Vector3::One);
                effect_->setSpecularMapEXTProperty(
                    panel == 4 || panel == 6 || panel == 7 ? strengthMap_.get() : nullptr);
                effect_->setSpecularColorMapEXTProperty(
                    panel == 5 || panel == 8 || panel == 9 ? colorMap_.get() : nullptr);
                effect_->setSpecularTextureCoordinateSetEXTProperty(panel == 6 ? 1 : 0);
                effect_->setSpecularColorTextureCoordinateSetEXTProperty(panel == 8 ? 1 : 0);
                effect_->setSpecularTextureTransformEXTProperty(
                    panel == 7 ? AuthoredTransform() : TextureTransformEXT{});
                effect_->setSpecularColorTextureTransformEXTProperty(
                    panel == 9 ? AuthoredTransform() : TextureTransformEXT{});
                effect_->setSpecularColorTextureIsSrgbEXTProperty(false);
                effect_->setWorldProperty(Matrix::CreateTranslation(PanelPositions[panel].X,
                                                                    PanelPositions[panel].Y, 0.0f));
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
            std::array<Color, 10> colors;
            for (std::size_t i = 0; i < colors.size(); ++i)
            {
                const Point p = PanelCenter(i);
                colors[i] = pixels.At(p.X, p.Y);
            }
            Context().Print("specular centres: default {}, IOR2 {}, zero {}, red {}, half {}, blue {}, "
                            "strength UV1 {}, strength transformed {}, color UV1 {}, color transformed {}",
                            colors[0].ToString(), colors[1].ToString(), colors[2].ToString(),
                            colors[3].ToString(), colors[4].ToString(), colors[5].ToString(),
                            colors[6].ToString(), colors[7].ToString(), colors[8].ToString(),
                            colors[9].ToString());
            Context().Check(colors[0].getRProperty() >= 38 && colors[0].getRProperty() <= 43 &&
                                colors[0].getRProperty() == colors[0].getGProperty() &&
                                colors[0].getGProperty() == colors[0].getBProperty(),
                            "the default IOR1.5 produces core glTF's neutral four-percent F0");
            Context().Check(colors[1].getRProperty() > colors[0].getRProperty() * 2 &&
                                colors[1].getRProperty() == colors[1].getGProperty() &&
                                colors[1].getGProperty() == colors[1].getBProperty(),
                            "IOR2 raises the neutral dielectric reflection");
            Context().Check(colors[2].getRProperty() <= 2 && colors[2].getGProperty() <= 2 &&
                                colors[2].getBProperty() <= 2,
                            "specular factor zero removes the dielectric reflection");
            Context().Check(colors[3].getRProperty() >= 38 && colors[3].getGProperty() <= 2 &&
                                colors[3].getBProperty() <= 2,
                            "the specular color factor tints the dielectric reflection red");
            Context().Check(colors[4].getRProperty() >= 18 && colors[4].getRProperty() <= 23 &&
                                colors[4].getRProperty() == colors[4].getGProperty() &&
                                colors[4].getGProperty() == colors[4].getBProperty(),
                            "the specular strength map's half alpha halves the reflection");
            Context().Check(colors[5].getRProperty() <= 2 && colors[5].getGProperty() <= 2 &&
                                colors[5].getBProperty() >= 38,
                            "the linear specular color map tints the reflection blue");
            Context().Check(colors[6].getRProperty() >= 8 && colors[6].getRProperty() <= 12 &&
                                colors[6].getRProperty() == colors[6].getGProperty() &&
                                colors[6].getGProperty() == colors[6].getBProperty(),
                            "the strength map can select UV1's quarter-alpha texel");
            Context().Check(colors[7] == colors[6],
                            "the strength-map transform independently maps UV0 to that texel");
            Context().Check(colors[8].getRProperty() <= 2 && colors[8].getGProperty() >= 38 &&
                                colors[8].getBProperty() <= 2,
                            "the color map can select UV1's green texel");
            Context().Check(colors[9] == colors[8],
                            "the color-map transform independently maps UV0 to that texel");
        }

        std::unique_ptr<PbrEffect> effect_;
        std::unique_ptr<TextureCube> irradiance_;
        std::unique_ptr<TextureCube> specular_;
        std::unique_ptr<Texture2D> brdfLut_;
        std::unique_ptr<Texture2D> strengthMap_;
        std::unique_ptr<Texture2D> colorMap_;
        std::vector<PbrVertex> quad_;
    };
}

CNA_DEMOS_GAME(SpecularMaterialDemo, {
    .id = "graphics.pbr_effect.specular_material",
    .title = "PBR dielectric specular material",
    .description = "Uses a controlled white environment to isolate KHR_materials_ior and "
                   "KHR_materials_specular factors, maps, dual-UV selectors and texture transforms.",
    .apis = {"PbrEffect::IorEXT", "PbrEffect::SpecularFactorEXT", "PbrEffect::SpecularColorFactorEXT",
             "PbrEffect::SpecularMapEXT", "PbrEffect::SpecularColorMapEXT",
             "PbrEffect::SpecularTextureCoordinateSetEXT", "PbrEffect::SpecularColorTextureCoordinateSetEXT",
             "PbrEffect::SpecularTextureTransformEXT", "PbrEffect::SpecularColorTextureTransformEXT",
             "PbrEffect::SpecularColorTextureIsSrgbEXT", "TextureTransformEXT", "ImageBasedLightEXT"},
    .tags = {"3d", "pbr", "specular", "ior", "texture", "gltf", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Skipped on SOFTWARE and HEADLESS, which do not sample PbrEffect material state.",
})
