// SPDX-License-Identifier: MS-PL
// CNAEXT PbrEffect texture maps: a normal map, an emissive map and an occlusion map, each on a
// panel of its own beside a panel with none.
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
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DirectionalLight.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
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
    // A normal map needs a tangent per vertex (xyz, and the bitangent's handedness in w): the
    // vertex layout glTF uses and CNA's PBR program reads. CNA has no stock type for it.
    struct PbrVertex
    {
        Vector3 Position;
        Vector3 Normal;
        Vector4 Tangent;
        Vector2 TextureCoordinate;
    };
    static_assert(std::is_trivially_copyable_v<PbrVertex> && sizeof(PbrVertex) == 48, "a plain 48-byte vertex");

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

    constexpr int Size = 64;  // texels per map side; 4 x 4 cells of 16

    // Round bumps: the tangent-space normal tilts away from each cell's centre.
    std::vector<Color> NormalMap()
    {
        std::vector<Color> texels(Size * Size);
        for (int y = 0; y < Size; ++y)
            for (int x = 0; x < Size; ++x)
            {
                const float dx = static_cast<float>(x % 16) - 7.5f, dy = static_cast<float>(y % 16) - 7.5f;
                Vector3 n(0.0f, 0.0f, 1.0f);
                if (dx * dx + dy * dy < 49.0f)
                    n = Vector3::Normalize(Vector3(dx / 7.0f, -dy / 7.0f, 0.6f));
                texels[static_cast<std::size_t>(y * Size + x)] =
                    Color(Vector3((n.X + 1.0f) / 2.0f, (n.Y + 1.0f) / 2.0f, (n.Z + 1.0f) / 2.0f));
            }
        return texels;
    }

    // Glowing stripes: rows 0-7 of every 16 emit warm light.
    std::vector<Color> EmissiveMap()
    {
        std::vector<Color> texels(Size * Size);
        for (int y = 0; y < Size; ++y)
            for (int x = 0; x < Size; ++x)
                texels[static_cast<std::size_t>(y * Size + x)] = y % 16 < 8 ? Color(255, 170, 60) : Color::Black;
        return texels;
    }

    // Grooves: the red channel (the occlusion) is low along a grid of lines.
    std::vector<Color> OcclusionMap()
    {
        std::vector<Color> texels(Size * Size);
        for (int y = 0; y < Size; ++y)
            for (int x = 0; x < Size; ++x)
                texels[static_cast<std::size_t>(y * Size + x)] =
                    x % 16 < 4 || y % 16 < 4 ? Color(40, 40, 40) : Color::White;
        return texels;
    }

    std::unique_ptr<Texture2D> MakeTexture(GraphicsDevice& device, const std::vector<Color>& texels)
    {
        auto texture = std::make_unique<Texture2D>(device, Size, Size);
        texture->SetData(texels.data(), static_cast<int>(texels.size()));
        return texture;
    }

    constexpr std::array<float, 4> PanelX = {-2.4f, -0.8f, 0.8f, 2.4f};

    class TextureMapsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // CNA documents PbrEffect as an unlit fallback on these renderers, with no capability
            // query for it (CNA-DEMOS-ISSUE-006).
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS" || renderer == "SDL_RENDERER")
                Context().Skip(std::string(renderer) + " draws PbrEffect as an unlit fallback");

            normalMap_ = MakeTexture(device, NormalMap());
            emissiveMap_ = MakeTexture(device, EmissiveMap());
            occlusionMap_ = MakeTexture(device, OcclusionMap());

            // One quad facing the camera; texture u runs along +x, so the tangent is +x.
            constexpr float H = 0.7f;
            const Vector3 normal(0.0f, 0.0f, 1.0f);
            const Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
            quad_ = {PbrVertex{Vector3(-H, H, 0.0f), normal, tangent, Vector2(0.0f, 0.0f)},
                     PbrVertex{Vector3(H, H, 0.0f), normal, tangent, Vector2(1.0f, 0.0f)},
                     PbrVertex{Vector3(-H, -H, 0.0f), normal, tangent, Vector2(0.0f, 1.0f)},
                     PbrVertex{Vector3(H, H, 0.0f), normal, tangent, Vector2(1.0f, 0.0f)},
                     PbrVertex{Vector3(H, -H, 0.0f), normal, tangent, Vector2(1.0f, 1.0f)},
                     PbrVertex{Vector3(-H, -H, 0.0f), normal, tangent, Vector2(0.0f, 1.0f)}};

            effect_ = std::make_unique<PbrEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 5.5f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setDiffuseColorProperty(Vector3(0.55f, 0.6f, 0.7f));
            effect_->setMetallicFactorProperty(0.0f);
            effect_->setRoughnessFactorProperty(0.5f);
            effect_->setLightingEnabledProperty(true);
            // A light grazing from the upper left, so bumps show a bright and a dark side.
            DirectionalLight& light = effect_->getDirectionalLight0Property();
            light.setDirectionProperty(Vector3::Normalize(Vector3(1.0f, -1.0f, -0.6f)));
            light.setDiffuseColorProperty(Vector3(2.5f, 2.5f, 2.5f));
            light.setSpecularColorProperty(Vector3(1.0f, 1.0f, 1.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(26, 28, 34));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            // 1: no maps. 2: NormalMap. 3: EmissiveMap (times EmissiveFactor). 4: OcclusionMap,
            // lit by ambient light only: occlusion darkens ambient (indirect) light, not lights.
            for (std::size_t panel = 0; panel < PanelX.size(); ++panel)
            {
                effect_->setNormalMapProperty(panel == 1 ? normalMap_.get() : nullptr);
                effect_->setEmissiveMapProperty(panel == 2 ? emissiveMap_.get() : nullptr);
                effect_->setEmissiveFactorProperty(panel == 2 ? Vector3(1.0f, 1.0f, 1.0f) : Vector3::Zero);
                effect_->setOcclusionMapProperty(panel == 3 ? occlusionMap_.get() : nullptr);
                effect_->getDirectionalLight0Property().setEnabledProperty(panel != 3);
                effect_->setAmbientLightColorProperty(panel == 3 ? Vector3(0.9f, 0.9f, 0.9f)
                                                                 : Vector3(0.08f, 0.08f, 0.08f));
                effect_->setWorldProperty(Matrix::CreateTranslation(PanelX[panel], 0.0f, 0.0f));
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, quad_, 0, 2, PbrVertexDeclaration());
                }
            }

            if (FramesDrawn() == 0)
            {
                Check(device);
            }
            Game::Draw(gameTime);
        }

    private:
        // Texel (u, v) of a panel, in pixels.
        Point At(std::size_t panel, float u, float v)
        {
            const Vector3 p = getGraphicsDeviceProperty().getViewportProperty().Project(
                Vector3(PanelX[panel] - 0.7f + 1.4f * u, 0.7f - 1.4f * v, 0.0f), effect_->getProjectionProperty(),
                effect_->getViewProperty(), Matrix::getIdentityProperty());
            return Point(static_cast<int>(p.X), static_cast<int>(p.Y));
        }

        void Check(GraphicsDevice& device)
        {
            const CnaDemos::BackBufferPixels pixels(device);
            const auto green = [&](std::size_t panel, float u, float v) {
                const Point p = At(panel, u, v);
                return static_cast<int>(pixels.At(p.X, p.Y).getGProperty());
            };
            // A bump's upper-left side faces the light, its lower-right side faces away; on the
            // flat panel both points are alike. (Cell 1,1 spans u, v = 0.25 .. 0.5.)
            const int flatLit = green(0, 0.33f, 0.33f), flatShade = green(0, 0.42f, 0.42f);
            const int bumpLit = green(1, 0.33f, 0.33f), bumpShade = green(1, 0.42f, 0.42f);
            Context().Print("flat panel {} / {}, normal-mapped bump {} / {}", flatLit, flatShade, bumpLit, bumpShade);
            Context().Check(std::abs(flatLit - flatShade) < 12, "without a normal map the panel is evenly lit");
            Context().Check(bumpLit > bumpShade + 30, "NormalMap: a bump's side facing the light is brighter");

            // Emissive stripe rows (v in 0 .. 0.125 of each quarter) glow, the others do not.
            const int glow = green(2, 0.5f, 0.06f + 0.25f), dark = green(2, 0.5f, 0.19f + 0.25f);
            Context().Print("emissive stripe {}, between stripes {}", glow, dark);
            Context().Check(glow > dark + 60, "EmissiveMap: the stripes glow");

            // Occlusion grooves (the first 4 of every 16 texels) take less ambient light.
            const int open = green(3, 0.5f + 0.15f, 0.5f + 0.15f), groove = green(3, 0.5f + 0.03f, 0.5f + 0.15f);
            Context().Print("ambient-lit panel {}, in a groove {}", open, groove);
            Context().Check(groove + 40 < open, "OcclusionMap: the grooves are darker in ambient light");
        }

        std::unique_ptr<PbrEffect> effect_;
        std::unique_ptr<Texture2D> normalMap_;
        std::unique_ptr<Texture2D> emissiveMap_;
        std::unique_ptr<Texture2D> occlusionMap_;
        std::vector<PbrVertex> quad_;
    };
}

CNA_DEMOS_GAME(TextureMapsDemo, {
    .id = "graphics.pbr_effect.texture_maps",
    .title = "PBR texture maps",
    .description = "Four PbrEffect panels: none, a NormalMap of bumps (with a vertex tangent), an EmissiveMap of "
                   "glowing stripes and an OcclusionMap of grooves that darkens ambient light only.",
    .apis = {"PbrEffect::NormalMap", "PbrEffect::EmissiveMap", "PbrEffect::EmissiveFactor", "PbrEffect::OcclusionMap",
             "PbrEffect::AmbientLightColor", "VertexElementUsage::Tangent", "VertexElementFormat::Vector4"},
    .tags = {"3d", "pbr", "normal-map", "emissive", "occlusion", "gltf", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Skipped on SOFTWARE, HEADLESS and SDL_RENDERER, where CNA draws PbrEffect as an unlit fallback.",
})
