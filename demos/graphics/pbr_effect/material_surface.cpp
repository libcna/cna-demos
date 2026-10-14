// SPDX-License-Identifier: MS-PL
// CNAEXT glTF surface semantics: COLOR_0 multiplication, alpha coverage and double-sided shading.
#include <array>
#include <cmath>
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
#include "Microsoft/Xna/Framework/Graphics/AlphaModeEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DirectionalLight.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
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
    struct PbrColorVertex
    {
        Vector3 Position;
        Vector3 Normal;
        Vector4 Tangent;
        Vector2 TextureCoordinate0;
        Vector2 TextureCoordinate1;
        std::uint8_t R;
        std::uint8_t G;
        std::uint8_t B;
        std::uint8_t A;
    };
    static_assert(std::is_trivially_copyable_v<PbrColorVertex> && sizeof(PbrColorVertex) == 60);

    const VertexDeclaration& PbrColorVertexDeclaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(static_cast<int>(offsetof(PbrColorVertex, Position)), VertexElementFormat::Vector3,
                          VertexElementUsage::Position, 0),
            VertexElement(static_cast<int>(offsetof(PbrColorVertex, Normal)), VertexElementFormat::Vector3,
                          VertexElementUsage::Normal, 0),
            VertexElement(static_cast<int>(offsetof(PbrColorVertex, Tangent)), VertexElementFormat::Vector4,
                          VertexElementUsage::Tangent, 0),
            VertexElement(static_cast<int>(offsetof(PbrColorVertex, TextureCoordinate0)),
                          VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 0),
            VertexElement(static_cast<int>(offsetof(PbrColorVertex, TextureCoordinate1)),
                          VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 1),
            VertexElement(static_cast<int>(offsetof(PbrColorVertex, R)), VertexElementFormat::Color,
                          VertexElementUsage::Color, 0),
        };
        return declaration;
    }

    std::vector<PbrColorVertex> Quad(const Color& color)
    {
        constexpr float half = 0.4f;
        const Vector3 normal(0.0f, 0.0f, 1.0f);
        const Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
        const Vector2 uv(0.5f, 0.5f);
        const auto vertex = [&](const Vector3& position) {
            return PbrColorVertex{position, normal, tangent, uv, uv,
                                  color.getRProperty(), color.getGProperty(),
                                  color.getBProperty(), color.getAProperty()};
        };
        const Vector3 a(-half, half, 0.0f), b(half, half, 0.0f);
        const Vector3 c(-half, -half, 0.0f), d(half, -half, 0.0f);
        return {vertex(a), vertex(b), vertex(c), vertex(b), vertex(d), vertex(c)};
    }

    constexpr std::array<Vector2, 10> PanelPositions = {
        Vector2(-3.6f, 0.75f), Vector2(-1.8f, 0.75f), Vector2(0.0f, 0.75f),
        Vector2(1.8f, 0.75f), Vector2(3.6f, 0.75f),
        Vector2(-3.6f, -0.75f), Vector2(-1.8f, -0.75f), Vector2(0.0f, -0.75f),
        Vector2(1.8f, -0.75f), Vector2(3.6f, -0.75f),
    };

    class MaterialSurfaceDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                Context().Skip(std::string(renderer) + " does not shade PbrEffect material surfaces");

            belowCutoff_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            aboveCutoff_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Color);
            const Color red64(255, 0, 0, 64), red192(255, 0, 0, 192);
            belowCutoff_->SetData(&red64, 1);
            aboveCutoff_->SetData(&red192, 1);

            blue_ = Quad(Color(0, 0, 255, 255));
            halfBlue_ = Quad(Color(0, 0, 255, 128));
            white_ = Quad(Color::White);
            // Under XNA's clockwise/viewport convention this winding is a back face when culling
            // is disabled, so the same geometry can expose PbrEffect's basis reversal.
            backFace_ = Quad(Color::White);

            effect_ = std::make_unique<PbrEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 7.2f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setMetallicFactorProperty(0.0f);
            effect_->setRoughnessFactorProperty(1.0f);
            effect_->setBaseColorTextureIsSrgbEXTProperty(false);
            effect_->setEncodeOutputToSrgbEXTProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);

            Context().Check(effect_->getAlphaProperty() == 1.0f &&
                                effect_->getAlphaModeEXTProperty() == AlphaModeEXT::Opaque &&
                                effect_->getAlphaCutoffEXTProperty() == 0.5f &&
                                !effect_->getDoubleSidedEXTProperty() && !effect_->VertexColorEnabledEXT,
                            "glTF surface controls have their canonical defaults");
            effect_->setAlphaProperty(0.25f);
            effect_->setAlphaModeEXTProperty(AlphaModeEXT::Blend);
            effect_->setAlphaCutoffEXTProperty(0.25f);
            effect_->setDoubleSidedEXTProperty(true);
            effect_->VertexColorEnabledEXT = true;
            Context().Check(effect_->getAlphaProperty() == 0.25f &&
                                effect_->getAlphaModeEXTProperty() == AlphaModeEXT::Blend &&
                                effect_->getAlphaCutoffEXTProperty() == 0.25f &&
                                effect_->getDoubleSidedEXTProperty() && effect_->VertexColorEnabledEXT,
                            "alpha mode, cutoff, double-sided and vertex-color state round-trip together");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            for (std::size_t panel = 0; panel < PanelPositions.size(); ++panel)
            {
                device.setBlendStateProperty(BlendState::Opaque);
                effect_->setTextureProperty(nullptr);
                effect_->setDiffuseColorProperty(Vector3::One);
                effect_->setAlphaProperty(1.0f);
                effect_->setSpecularFactorEXTProperty(1.0f);
                effect_->setAmbientLightColorProperty(Vector3::One);
                effect_->setAlphaModeEXTProperty(AlphaModeEXT::Opaque);
                effect_->setAlphaCutoffEXTProperty(0.5f);
                effect_->setDoubleSidedEXTProperty(false);
                effect_->VertexColorEnabledEXT = false;
                effect_->getDirectionalLight0Property().setEnabledProperty(false);
                const std::vector<PbrColorVertex>* vertices = &white_;

                if (panel == 0 || panel == 1)
                {
                    vertices = &blue_;
                    effect_->VertexColorEnabledEXT = panel == 1;
                }
                else if (panel == 2)
                {
                    vertices = &halfBlue_;
                    effect_->VertexColorEnabledEXT = true;
                    effect_->setAlphaModeEXTProperty(AlphaModeEXT::Blend);
                    device.setBlendStateProperty(BlendState::NonPremultiplied);
                }
                else if (panel >= 3 && panel <= 5)
                {
                    effect_->setTextureProperty(panel == 4 ? aboveCutoff_.get() : belowCutoff_.get());
                    effect_->setAlphaModeEXTProperty(AlphaModeEXT::Mask);
                    effect_->setAlphaCutoffEXTProperty(panel == 5 ? 0.2f : 0.5f);
                }
                else if (panel <= 7)
                {
                    vertices = &backFace_;
                    effect_->setDiffuseColorProperty(Vector3(0.0f, 1.0f, 0.0f));
                    effect_->setSpecularFactorEXTProperty(0.0f);
                    effect_->setAmbientLightColorProperty(Vector3::Zero);
                    effect_->setDoubleSidedEXTProperty(panel == 7);
                    DirectionalLight& light = effect_->getDirectionalLight0Property();
                    light.setEnabledProperty(true);
                    // Keep L behind the authored +Z normal but slightly off the view axis. An
                    // exactly opposite view/light pair has a zero half-vector, which is not a
                    // useful oracle for the normal reversal itself.
                    light.setDirectionProperty(Vector3::Normalize(Vector3(0.4f, 0.0f, 1.0f)));
                    light.setDiffuseColorProperty(Vector3(3.3f, 3.3f, 3.3f));
                    light.setSpecularColorProperty(Vector3::Zero);
                }
                else
                {
                    vertices = panel == 8 ? &blue_ : &halfBlue_;
                    effect_->VertexColorEnabledEXT = true;
                    effect_->setAlphaProperty(0.5f);
                    effect_->setAlphaModeEXTProperty(AlphaModeEXT::Blend);
                    device.setBlendStateProperty(BlendState::NonPremultiplied);
                }

                effect_->setWorldProperty(Matrix::CreateTranslation(
                    PanelPositions[panel].X, PanelPositions[panel].Y, 0.0f));
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices->data(), 0, 2,
                                              PbrColorVertexDeclaration());
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
            Context().Print("centres: color off {}, on {}, blend {}, mask below {}, above {}, lower cutoff {}, "
                            "one-sided back {}, double-sided back {}, material alpha {}, combined alpha {}",
                            colors[0].ToString(), colors[1].ToString(), colors[2].ToString(),
                            colors[3].ToString(), colors[4].ToString(), colors[5].ToString(),
                            colors[6].ToString(), colors[7].ToString(), colors[8].ToString(),
                            colors[9].ToString());

            const auto near = [](const Color& actual, int r, int g, int b) {
                return std::abs(static_cast<int>(actual.getRProperty()) - r) <= 2 &&
                       std::abs(static_cast<int>(actual.getGProperty()) - g) <= 2 &&
                       std::abs(static_cast<int>(actual.getBProperty()) - b) <= 2;
            };
            Context().Check(near(colors[0], 255, 255, 255),
                            "disabled vertex color leaves the white material unchanged");
            Context().Check(near(colors[1], 0, 0, 255),
                            "enabled COLOR_0 multiplies the material to blue");
            Context().Check(near(colors[2], 0, 0, 128),
                            "Blend mode carries vertex alpha into straight-alpha compositing");
            Context().Check(near(colors[3], 0, 0, 0), "Mask discards alpha below its cutoff");
            Context().Check(near(colors[4], 255, 0, 0), "Mask keeps alpha above its cutoff");
            Context().Check(near(colors[5], 255, 0, 0),
                            "lowering the cutoff keeps the same quarter-alpha fragment");
            Context().Check(colors[6].getGProperty() <= 2,
                            "a one-sided back face keeps its authored forward normal");
            Context().Check(colors[7].getGProperty() >= 200 && colors[7].getRProperty() <= 2 &&
                                colors[7].getBProperty() <= 2,
                            "double-sided shading reverses the back-face basis toward the light");
            Context().Check(near(colors[8], 0, 0, 128),
                            "material Alpha one half reaches straight-alpha compositing");
            Context().Check(near(colors[9], 0, 0, 64),
                            "material Alpha multiplies vertex alpha before compositing");
        }

        std::unique_ptr<PbrEffect> effect_;
        std::unique_ptr<Texture2D> belowCutoff_;
        std::unique_ptr<Texture2D> aboveCutoff_;
        std::vector<PbrColorVertex> blue_;
        std::vector<PbrColorVertex> halfBlue_;
        std::vector<PbrColorVertex> white_;
        std::vector<PbrColorVertex> backFace_;
    };
}

CNA_DEMOS_GAME(MaterialSurfaceDemo, {
    .id = "graphics.pbr_effect.material_surface",
    .title = "PBR material surface modes",
    .description = "Shows glTF COLOR_0 RGB/alpha and material-alpha multiplication, opaque/blend/mask coverage "
                   "with cutoff, and the lighting-basis reversal used for double-sided back faces.",
    .apis = {"PbrEffect::Alpha", "PbrEffect::VertexColorEnabledEXT", "PbrEffect::AlphaModeEXT", "PbrEffect::AlphaCutoffEXT",
             "PbrEffect::DoubleSidedEXT", "AlphaModeEXT", "BlendState::NonPremultiplied",
             "VertexElementUsage::Color"},
    .tags = {"3d", "pbr", "vertex-color", "alpha", "mask", "double-sided", "gltf", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Skipped on SOFTWARE and HEADLESS, which do not shade PbrEffect material surfaces.",
})
