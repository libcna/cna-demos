// SPDX-License-Identifier: MS-PL
// CNAEXT CRTEffect: the finished frame shown on an emulated CRT (scanlines, mask, curvature).
#include <cmath>
#include <memory>
#include <optional>

#include "CNA/Graphics/CRTEffect.hpp"
#include "CNA/Graphics/CRTMaskType.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class CrtDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            gradient_ = getContentProperty().Load<Texture2D>("textures/gradient");
            const auto& pp = device.getPresentationParametersProperty();
            scene_ = std::make_unique<RenderTarget2D>(device, pp.getBackBufferWidthProperty(), pp.getBackBufferHeightProperty());
            crt_ = std::make_unique<CNA::Graphics::CRTEffect>(device);
            // Every part is tunable; these are on the strong side so the screenshot shows them.
            crt_->setScanlineIntensity(0.6f);
            crt_->setMaskType(CNA::Graphics::CRTMaskType::ApertureGrille);
            crt_->setMaskIntensity(0.4f);
            crt_->setCurvature(0.15f);
            crt_->setVignetteIntensity(0.5f);
            Context().Check(crt_->getMaskType() == CNA::Graphics::CRTMaskType::ApertureGrille &&
                                crt_->getScanlineIntensity() == 0.6f,
                            "the settings read back");
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            time_ += static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // 1. The game, into a render target (no effect).
            device.SetRenderTarget(scene_.get());
            device.Clear(Color(16, 40, 72));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::LinearClamp, nullptr, nullptr);
            spriteBatch_->Draw(gradient_, Rectangle(80, 80, 280, 280), Color::White);
            const float bob = 20.0f * std::sin(time_ * 3.0f);
            spriteBatch_->DrawString(*font_, "INSERT COIN", Vector2(420.0f, 200.0f + bob), Color::Yellow, 0.0f,
                                     Vector2::Zero, 2.0f, SpriteEffects::None, 0.0f);
            spriteBatch_->End();

            // 2. The whole frame as one full-screen quad through the effect: curvature and
            // vignette are measured across that single quad.
            device.SetRenderTarget(nullptr);
            device.Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Immediate, &BlendState::Opaque, &SamplerState::LinearClamp, nullptr,
                                nullptr, crt_.get());
            spriteBatch_->Draw(*scene_, Rectangle(0, 0, scene_->getWidthProperty(), scene_->getHeightProperty()),
                               Color::White);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        float time_ = 0.0f;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D gradient_;
        std::unique_ptr<RenderTarget2D> scene_;
        std::unique_ptr<CNA::Graphics::CRTEffect> crt_;
    };
}

CNA_DEMOS_GAME(CrtDemo, {
    .id = "cnaext.crt_effect.full_screen",
    .title = "CRT monitor effect (CNAEXT)",
    .description = "Draws a scene into a RenderTarget2D, then shows the whole frame through CNA's CRTEffect as one "
                   "full-screen SpriteBatch quad: scanlines, an aperture-grille mask, screen curvature and a vignette.",
    .apis = {"CNA::Graphics::CRTEffect::CRTEffect", "CNA::Graphics::CRTEffect::setScanlineIntensity",
             "CNA::Graphics::CRTEffect::setMaskType", "CNA::Graphics::CRTEffect::setMaskIntensity",
             "CNA::Graphics::CRTEffect::setCurvature", "CNA::Graphics::CRTEffect::setVignetteIntensity",
             "CNA::Graphics::CRTMaskType", "SpriteBatch::Begin"},
    .tags = {"cnaext", "post-process", "crt", "retro", "shader", "render-target"},
    .needs = CnaDemos::Need::CustomEffects,
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb", "textures/gradient.png"},
    .restrictions = "Shader renderers only (GLSL or SPIR-V package); SOFTWARE and other 2D renderers skip it.",
})
