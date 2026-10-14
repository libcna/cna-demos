// SPDX-License-Identifier: MS-PL
// CNAEXT PbrEffect: glTF metallic-roughness materials -- a grid of spheres from rough to smooth, plastic to metal.
#include <algorithm>
#include <array>
#include <format>
#include <string>
#include <memory>
#include <optional>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DirectionalLight.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr std::array<float, 4> Roughness = {0.15f, 0.4f, 0.7f, 1.0f};

    class MetallicRoughnessDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // CNA documents PbrEffect as an unlit fallback on these renderers, with no capability
            // query for it (CNA-DEMOS-ISSUE-006).
            const std::string_view renderer = device.GetGraphicsRendererName();
            if (renderer == "SOFTWARE" || renderer == "HEADLESS")
                Context().Skip(std::string(renderer) + " draws PbrEffect as an unlit fallback");

            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            sphere_ = CnaDemos::MakeSphere(0.42f);
            effect_ = std::make_unique<PbrEffect>(device);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
            effect_->setDiffuseColorProperty(Vector3(0.9f, 0.55f, 0.2f));  // the glTF base colour
            effect_->setAmbientLightColorProperty(Vector3(0.05f, 0.05f, 0.05f));
            effect_->setLightingEnabledProperty(true);
            DirectionalLight& light = effect_->getDirectionalLight0Property();
            light.setEnabledProperty(true);
            light.setDirectionProperty(Vector3::Normalize(Vector3(-0.5f, -0.6f, -1.0f)));
            light.setDiffuseColorProperty(Vector3(3.0f, 3.0f, 3.0f));
            light.setSpecularColorProperty(Vector3(3.0f, 3.0f, 3.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(30, 30, 36));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            // Rows: metallic 0 (plastic-like) and 1 (metal). Columns: roughness from smooth to rough.
            for (int row = 0; row < 2; ++row)
            {
                effect_->setMetallicFactorProperty(static_cast<float>(row));
                for (std::size_t column = 0; column < Roughness.size(); ++column)
                {
                    effect_->setRoughnessFactorProperty(Roughness[column]);
                    effect_->setWorldProperty(Matrix::CreateTranslation(-1.65f + 1.1f * static_cast<float>(column),
                                                                        row == 0 ? 0.6f : -0.6f, 0.0f));
                    for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                    {
                        pass.Apply();
                        device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, sphere_.vertices.data(), 0,
                                                         sphere_.VertexCount(), sphere_.indices.data(), 0,
                                                         sphere_.PrimitiveCount());
                    }
                }
            }

            if (FramesDrawn() == 0)
            {
                // The brightest pixel of the smoothest and the roughest dielectric sphere: a smooth
                // surface concentrates the light into a small, intense highlight.
                const CnaDemos::BackBufferPixels p(device);
                const auto peak = [&](int cx, int cy) {
                    int best = 0;
                    for (int y = cy - 45; y < cy + 45; ++y)
                        for (int x = cx - 45; x < cx + 45; ++x)
                            best = std::max(best, static_cast<int>(p.At(x, y).getGProperty()));
                    return best;
                };
                const int smooth = peak(220, 170), rough = peak(580, 170);
                Context().Print("highlight peak (green channel): roughness 0.15 -> {}, roughness 1.0 -> {}", smooth, rough);
                Context().Check(smooth > rough, "the smoother surface has the brighter highlight");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "metallic 0", Vector2(20.0f, 160.0f), Color::White);
            spriteBatch_->DrawString(*font_, "metallic 1", Vector2(20.0f, 300.0f), Color::White);
            spriteBatch_->DrawString(*font_, "roughness", Vector2(20.0f, 420.0f), Color::White);
            for (std::size_t column = 0; column < Roughness.size(); ++column)
            {
                // Centre each value under its column of spheres.
                const Vector3 at = device.getViewportProperty().Project(
                    Vector3(-1.65f + 1.1f * static_cast<float>(column), 0.0f, 0.0f), effect_->getProjectionProperty(),
                    effect_->getViewProperty(), Matrix::getIdentityProperty());
                const std::string text = std::format("{:.2f}", Roughness[column]);
                spriteBatch_->DrawString(*font_, text, Vector2(at.X - font_->MeasureString(text).X / 2.0f, 420.0f),
                                         Color::White);
            }
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        CnaDemos::Mesh sphere_;
        std::unique_ptr<PbrEffect> effect_;
    };
}

CNA_DEMOS_GAME(MetallicRoughnessDemo, {
    .id = "graphics.pbr_effect.metallic_roughness",
    .title = "PBR metallic and roughness",
    .description = "Draws a grid of spheres with CNA's PbrEffect (glTF metallic-roughness): metallic 0 and 1 in "
                   "rows, roughness 0.15 to 1.0 in columns.",
    .apis = {"PbrEffect::PbrEffect", "PbrEffect::MetallicFactor", "PbrEffect::RoughnessFactor",
             "PbrEffect::DiffuseColor", "PbrEffect::DirectionalLight0"},
    .tags = {"3d", "pbr", "material", "gltf", "effect", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Skipped on SOFTWARE and HEADLESS, where CNA draws PbrEffect as an unlit fallback.",
})
