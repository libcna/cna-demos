// SPDX-License-Identifier: MS-PL
// CNAEXT KHR_materials_ior/specular: dielectric Fresnel factors and both optional texture maps.
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

    void FillCube(TextureCube& cube, const Color& color)
    {
        for (int face = 0; face < 6; ++face)
            cube.SetData(static_cast<CubeMapFace>(face), &color, 1);
    }

    constexpr std::array<Vector2, 6> PanelPositions = {
        Vector2(-1.8f, 0.75f), Vector2(0.0f, 0.75f), Vector2(1.8f, 0.75f),
        Vector2(-1.8f, -0.75f), Vector2(0.0f, -0.75f), Vector2(1.8f, -0.75f),
    };

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
            strengthMap_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color halfStrength(255, 255, 255, 128);
            strengthMap_->SetData(&halfStrength, 1);
            colorMap_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color blue = Color::Blue;
            colorMap_->SetData(&blue, 1);

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
                                effect_->getSpecularColorMapEXTProperty() == nullptr,
                            "core glTF dielectric specular defaults are neutral");
            effect_->setIorEXTProperty(2.0f);
            effect_->setSpecularFactorEXTProperty(0.5f);
            effect_->setSpecularColorFactorEXTProperty(Vector3(1.0f, 0.25f, 0.5f));
            effect_->setSpecularMapEXTProperty(strengthMap_.get());
            effect_->setSpecularColorMapEXTProperty(colorMap_.get());
            Context().Check(effect_->getIorEXTProperty() == 2.0f &&
                                effect_->getSpecularFactorEXTProperty() == 0.5f &&
                                effect_->getSpecularColorFactorEXTProperty() == Vector3(1.0f, 0.25f, 0.5f) &&
                                effect_->getSpecularMapEXTProperty() == strengthMap_.get() &&
                                effect_->getSpecularColorMapEXTProperty() == colorMap_.get(),
                            "IOR, factors and both specular texture identities round-trip together");

            constexpr float half = 0.52f;
            const Vector3 normal(0.0f, 0.0f, 1.0f);
            const Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
            quad_ = {PbrVertex{Vector3(-half, half, 0.0f), normal, tangent, Vector2(0.0f, 0.0f)},
                     PbrVertex{Vector3(half, half, 0.0f), normal, tangent, Vector2(1.0f, 0.0f)},
                     PbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, Vector2(0.0f, 1.0f)},
                     PbrVertex{Vector3(half, half, 0.0f), normal, tangent, Vector2(1.0f, 0.0f)},
                     PbrVertex{Vector3(half, -half, 0.0f), normal, tangent, Vector2(1.0f, 1.0f)},
                     PbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, Vector2(0.0f, 1.0f)}};
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
                effect_->setSpecularMapEXTProperty(panel == 4 ? strengthMap_.get() : nullptr);
                effect_->setSpecularColorMapEXTProperty(panel == 5 ? colorMap_.get() : nullptr);
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
            std::array<Color, 6> colors;
            for (std::size_t i = 0; i < colors.size(); ++i)
            {
                const Point p = PanelCenter(i);
                colors[i] = pixels.At(p.X, p.Y);
            }
            Context().Print("specular centres: default {}, IOR2 {}, zero {}, red {}, half {}, blue {}",
                            colors[0].ToString(), colors[1].ToString(), colors[2].ToString(),
                            colors[3].ToString(), colors[4].ToString(), colors[5].ToString());
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
                   "KHR_materials_specular: scalar/color factors plus strength-alpha and color maps.",
    .apis = {"PbrEffect::IorEXT", "PbrEffect::SpecularFactorEXT", "PbrEffect::SpecularColorFactorEXT",
             "PbrEffect::SpecularMapEXT", "PbrEffect::SpecularColorMapEXT",
             "PbrEffect::SpecularColorTextureIsSrgbEXT", "ImageBasedLightEXT"},
    .tags = {"3d", "pbr", "specular", "ior", "texture", "gltf", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Skipped on SOFTWARE and HEADLESS, which do not sample PbrEffect material state.",
})
