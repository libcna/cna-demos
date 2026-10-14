// SPDX-License-Identifier: MS-PL
// CNAEXT PbrEffect dual-UV selection and independent KHR_texture_transform state.
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
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
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
    // CNA's canonical rigid dual-UV PBR record. The final packed colour occupies what was once
    // padding; opaque white is the identity even if vertex colour is enabled later.
    struct DualUvPbrVertex
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
    static_assert(std::is_trivially_copyable_v<DualUvPbrVertex> && sizeof(DualUvPbrVertex) == 60);
    static_assert(offsetof(DualUvPbrVertex, TextureCoordinate0) == 40);
    static_assert(offsetof(DualUvPbrVertex, TextureCoordinate1) == 48);
    static_assert(offsetof(DualUvPbrVertex, R) == 56);

    const VertexDeclaration& DualUvPbrVertexDeclaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(static_cast<int>(offsetof(DualUvPbrVertex, Position)), VertexElementFormat::Vector3,
                          VertexElementUsage::Position, 0),
            VertexElement(static_cast<int>(offsetof(DualUvPbrVertex, Normal)), VertexElementFormat::Vector3,
                          VertexElementUsage::Normal, 0),
            VertexElement(static_cast<int>(offsetof(DualUvPbrVertex, Tangent)), VertexElementFormat::Vector4,
                          VertexElementUsage::Tangent, 0),
            VertexElement(static_cast<int>(offsetof(DualUvPbrVertex, TextureCoordinate0)),
                          VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 0),
            VertexElement(static_cast<int>(offsetof(DualUvPbrVertex, TextureCoordinate1)),
                          VertexElementFormat::Vector2, VertexElementUsage::TextureCoordinate, 1),
            VertexElement(static_cast<int>(offsetof(DualUvPbrVertex, R)), VertexElementFormat::Color,
                          VertexElementUsage::Color, 0),
        };
        return declaration;
    }

    constexpr std::array<Vector2, 6> PanelPositions = {
        Vector2(-1.8f, 0.75f), Vector2(0.0f, 0.75f), Vector2(1.8f, 0.75f),
        Vector2(-1.8f, -0.75f), Vector2(0.0f, -0.75f), Vector2(1.8f, -0.75f),
    };

    const TextureTransformEXT& AuthoredTransform()
    {
        // UV0 (.25,.25), scaled by .5, rotated 90 degrees, then offset, becomes (.75,.25):
        // the centre of the second texel. This makes every transform component observable.
        static const TextureTransformEXT transform{
            Vector2(0.875f, 0.125f), Vector2(0.5f, 0.5f), MathHelper::PiOver2};
        return transform;
    }

    class TextureCoordinatesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                Context().Skip(std::string(renderer) + " does not sample PbrEffect texture-coordinate state");

            baseMap_ = std::make_unique<Texture2D>(device, 2, 1, false, SurfaceFormat::Color);
            const std::array<Color, 2> baseTexels = {Color(192, 0, 0), Color(0, 192, 0)};
            baseMap_->SetData(baseTexels.data(), static_cast<int>(baseTexels.size()));

            emissiveMap_ = std::make_unique<Texture2D>(device, 2, 1, false, SurfaceFormat::Color);
            const std::array<Color, 2> emissiveTexels = {Color::Black, Color(0, 0, 192)};
            emissiveMap_->SetData(emissiveTexels.data(), static_cast<int>(emissiveTexels.size()));

            constexpr float half = 0.52f;
            const Vector3 normal(0.0f, 0.0f, 1.0f);
            const Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
            const Vector2 uv0(0.25f, 0.25f), uv1(0.75f, 0.25f);
            quad_ = {
                DualUvPbrVertex{Vector3(-half, half, 0.0f), normal, tangent, uv0, uv1},
                DualUvPbrVertex{Vector3(half, half, 0.0f), normal, tangent, uv0, uv1},
                DualUvPbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, uv0, uv1},
                DualUvPbrVertex{Vector3(half, half, 0.0f), normal, tangent, uv0, uv1},
                DualUvPbrVertex{Vector3(half, -half, 0.0f), normal, tangent, uv0, uv1},
                DualUvPbrVertex{Vector3(-half, -half, 0.0f), normal, tangent, uv0, uv1},
            };

            effect_ = std::make_unique<PbrEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setDiffuseColorProperty(Vector3::One);
            effect_->setMetallicFactorProperty(0.0f);
            effect_->setRoughnessFactorProperty(1.0f);
            effect_->setBaseColorTextureIsSrgbEXTProperty(false);
            effect_->setEmissiveTextureIsSrgbEXTProperty(false);
            effect_->setEncodeOutputToSrgbEXTProperty(false);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);

            const TextureTransformEXT identity;
            Context().Check(effect_->getTextureCoordinateSetsEXTProperty() ==
                                (std::array<int, 5>{0, 0, 0, 0, 0}) &&
                                effect_->getTextureTransformsEXTProperty() ==
                                (std::array<TextureTransformEXT, 5>{identity, identity, identity, identity, identity}),
                            "all five texture slots default to UV0 and identity transforms");
            effect_->setTextureCoordinateSetEXTProperty(0, 1);
            effect_->setTextureTransformEXTProperty(0, AuthoredTransform());
            Context().Check(effect_->getTextureCoordinateSetsEXTProperty() ==
                                (std::array<int, 5>{1, 0, 0, 0, 0}) &&
                                effect_->getTextureTransformsEXTProperty()[0] == AuthoredTransform() &&
                                effect_->getTextureTransformsEXTProperty()[3] == identity,
                            "base-color UV selection and transform leave the emissive slot independent");

            device.getSamplerStatesProperty()(0, SamplerState::PointClamp);
            device.getSamplerStatesProperty()(3, SamplerState::PointClamp);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(20, 22, 28));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            const TextureTransformEXT identity;
            for (std::size_t panel = 0; panel < PanelPositions.size(); ++panel)
            {
                const bool baseColor = panel < 3;
                const int mode = static_cast<int>(panel % 3);

                effect_->setTextureProperty(baseColor ? baseMap_.get() : nullptr);
                effect_->setAmbientLightColorProperty(baseColor ? Vector3::One : Vector3::Zero);
                effect_->setEmissiveMapProperty(baseColor ? nullptr : emissiveMap_.get());
                effect_->setEmissiveFactorProperty(baseColor ? Vector3::Zero : Vector3::One);

                effect_->setTextureCoordinateSetEXTProperty(0, 0);
                effect_->setTextureCoordinateSetEXTProperty(3, 0);
                effect_->setTextureTransformEXTProperty(0, identity);
                effect_->setTextureTransformEXTProperty(3, identity);
                const int slot = baseColor ? 0 : 3;
                if (mode == 1)
                    effect_->setTextureCoordinateSetEXTProperty(slot, 1);
                else if (mode == 2)
                    effect_->setTextureTransformEXTProperty(slot, AuthoredTransform());

                effect_->setWorldProperty(Matrix::CreateTranslation(
                    PanelPositions[panel].X, PanelPositions[panel].Y, 0.0f));
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, quad_.data(), 0, 2,
                                              DualUvPbrVertexDeclaration());
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
            Context().Print("centres: base UV0 {}, UV1 {}, transformed {}; emissive UV0 {}, UV1 {}, transformed {}",
                            colors[0].ToString(), colors[1].ToString(), colors[2].ToString(),
                            colors[3].ToString(), colors[4].ToString(), colors[5].ToString());

            const auto near = [](const Color& actual, int r, int g, int b) {
                return std::abs(static_cast<int>(actual.getRProperty()) - r) <= 2 &&
                       std::abs(static_cast<int>(actual.getGProperty()) - g) <= 2 &&
                       std::abs(static_cast<int>(actual.getBProperty()) - b) <= 2;
            };
            Context().Check(near(colors[0], 192, 0, 0), "the base-color slot samples UV0's red texel");
            Context().Check(near(colors[1], 0, 192, 0), "the base-color slot can select UV1's green texel");
            Context().Check(near(colors[2], 0, 192, 0),
                            "the base-color transform maps UV0 to the green texel");
            Context().Check(near(colors[3], 0, 0, 0), "the emissive slot samples UV0's black texel");
            Context().Check(near(colors[4], 0, 0, 192), "the emissive slot independently selects UV1's blue texel");
            Context().Check(near(colors[5], 0, 0, 192),
                            "the emissive transform independently maps UV0 to the blue texel");
        }

        std::unique_ptr<PbrEffect> effect_;
        std::unique_ptr<Texture2D> baseMap_;
        std::unique_ptr<Texture2D> emissiveMap_;
        std::vector<DualUvPbrVertex> quad_;
    };
}

CNA_DEMOS_GAME(TextureCoordinatesDemo, {
    .id = "graphics.pbr_effect.texture_coordinates",
    .title = "PBR texture coordinates",
    .description = "Uses CNA's canonical dual-UV PBR vertex to compare UV0, UV1 and an authored "
                   "scale-rotate-translate transform independently on base-color and emissive maps.",
    .apis = {"PbrEffect::TextureCoordinateSetsEXT", "PbrEffect::TextureTransformsEXT",
             "PbrEffect::BaseColorTextureIsSrgbEXT", "PbrEffect::EmissiveTextureIsSrgbEXT",
             "PbrEffect::EncodeOutputToSrgbEXT", "TextureTransformEXT", "VertexElementUsage::TextureCoordinate"},
    .tags = {"3d", "pbr", "texture", "uv", "transform", "gltf", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Skipped on SOFTWARE and HEADLESS, which do not sample PbrEffect texture-coordinate state.",
})
