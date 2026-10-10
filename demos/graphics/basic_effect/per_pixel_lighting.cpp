// SPDX-License-Identifier: MS-PL
// BasicEffect::PreferPerPixelLighting: lighting per pixel instead of per vertex -- crisp highlights on coarse meshes.
#include <algorithm>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DirectionalLight.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class PerPixelLightingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // A deliberately coarse sphere: few vertices for the lighting to be computed at.
            sphere_ = CnaDemos::MakeSphere(1.0f, 9, 13);

            for (int i = 0; i < 2; ++i)
            {
                auto& effect = effects_[i];
                effect = std::make_unique<BasicEffect>(device);
                effect->setWorldProperty(Matrix::CreateTranslation(i == 0 ? -1.4f : 1.4f, 0.0f, 0.0f));
                effect->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 5.0f), Vector3::Zero, Vector3::Up));
                effect->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                    MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
                effect->setLightingEnabledProperty(true);
                effect->setDiffuseColorProperty(Vector3(0.1f, 0.3f, 0.8f));
                effect->setAmbientLightColorProperty(Vector3(0.05f, 0.05f, 0.05f));
                effect->setSpecularColorProperty(Vector3::One);
                effect->setSpecularPowerProperty(64.0f);  // a small, sharp highlight
                DirectionalLight& light = effect->getDirectionalLight0Property();
                light.setEnabledProperty(true);
                light.setDirectionProperty(Vector3::Normalize(Vector3(-0.35f, -0.3f, -1.0f)));
                light.setDiffuseColorProperty(Vector3::One);
                light.setSpecularColorProperty(Vector3::One);
            }
            // The only difference between the two.
            effects_[0]->setPreferPerPixelLightingProperty(false);
            effects_[1]->setPreferPerPixelLightingProperty(true);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);
            for (auto& effect : effects_)
            {
                for (EffectPass& pass : effect->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, sphere_.vertices.data(), 0,
                                                     sphere_.VertexCount(), sphere_.indices.data(), 0,
                                                     sphere_.PrimitiveCount());
                }
            }

            if (FramesDrawn() == 0)
            {
                // The brightest pixel of each sphere: the per-pixel highlight reaches (nearly) white.
                const CnaDemos::BackBufferPixels p(device);
                const auto brightest = [&](int x0, int x1) {
                    int best = 0;
                    for (int y = 100; y < 380; ++y)
                        for (int x = x0; x < x1; ++x)
                            best = std::max(best, static_cast<int>(p.At(x, y).getRProperty()));
                    return best;
                };
                const int perVertex = brightest(150, 380);
                const int perPixel = brightest(420, 650);
                Context().Print("brightest red channel: per vertex {}, per pixel {}", perVertex, perPixel);
                Context().Check(perPixel > 240, "per-pixel lighting renders the full specular highlight");
                Context().Check(perPixel > perVertex, "per-vertex lighting misses part of it on this coarse mesh");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "PreferPerPixelLighting = false", Vector2(60.0f, 400.0f), Color::White);
            spriteBatch_->DrawString(*font_, "= true", Vector2(560.0f, 400.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        CnaDemos::Mesh sphere_;
        std::unique_ptr<BasicEffect> effects_[2];
    };
}

CNA_DEMOS_GAME(PerPixelLightingDemo, {
    .id = "graphics.basic_effect.per_pixel_lighting",
    .title = "Per-pixel lighting",
    .description = "Lights a coarse sphere with a sharp specular highlight per vertex and with "
                   "BasicEffect::PreferPerPixelLighting; per pixel the highlight stays round and bright.",
    .apis = {"BasicEffect::PreferPerPixelLighting", "BasicEffect::SpecularColor", "BasicEffect::SpecularPower",
             "DirectionalLight::SpecularColor"},
    .tags = {"3d", "lighting", "specular", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
