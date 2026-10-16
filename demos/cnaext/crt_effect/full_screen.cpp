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
            Context().Check(crt_->getScanlineIntensity() == 0.3f && crt_->getCurvature() == 0.08f &&
                                crt_->getVignetteIntensity() == 0.25f && crt_->getMaskIntensity() == 0.35f &&
                                crt_->getMaskType() == CNA::Graphics::CRTMaskType::ApertureGrille,
                            "all five CRT parameters start at their documented moderate defaults");
            crt_->setScanlineIntensity(-1.0f);
            crt_->setCurvature(2.0f);
            crt_->setVignetteIntensity(-1.0f);
            crt_->setMaskIntensity(2.0f);
            Context().Check(crt_->getScanlineIntensity() == 0.0f && crt_->getCurvature() == 1.0f &&
                                crt_->getVignetteIntensity() == 0.0f && crt_->getMaskIntensity() == 1.0f,
                            "numeric parameters clamp independently to their zero and one bounds");
            bool everyMaskReadsBack = true;
            for (const auto mask : {CNA::Graphics::CRTMaskType::None, CNA::Graphics::CRTMaskType::ShadowMask,
                                    CNA::Graphics::CRTMaskType::ApertureGrille})
            {
                crt_->setMaskType(mask);
                everyMaskReadsBack = everyMaskReadsBack && crt_->getMaskType() == mask;
            }
            Context().Check(everyMaskReadsBack, "all three sub-pixel mask choices round-trip");
            // Every part is tunable; these are on the strong side so the screenshot shows them.
            crt_->setScanlineIntensity(0.6f);
            crt_->setMaskType(CNA::Graphics::CRTMaskType::ApertureGrille);
            crt_->setMaskIntensity(0.4f);
            crt_->setCurvature(0.15f);
            crt_->setVignetteIntensity(0.5f);
            Context().Check(crt_->getMaskType() == CNA::Graphics::CRTMaskType::ApertureGrille &&
                                crt_->getScanlineIntensity() == 0.6f && crt_->getMaskIntensity() == 0.4f &&
                                crt_->getCurvature() == 0.15f && crt_->getVignetteIntensity() == 0.5f,
                            "the complete authored CRT configuration reads back");

            std::unique_ptr<Effect> cloneOwner(crt_->Clone());
            auto* clone = dynamic_cast<CNA::Graphics::CRTEffect*>(cloneOwner.get());
            const bool cloneRetained = clone && clone != crt_.get() && clone->GetTypeName() == "CNA.Graphics.CRTEffect" &&
                clone->getScanlineIntensity() == 0.6f && clone->getMaskIntensity() == 0.4f &&
                clone->getCurvature() == 0.15f && clone->getVignetteIntensity() == 0.5f &&
                clone->getMaskType() == CNA::Graphics::CRTMaskType::ApertureGrille;
            if (clone)
            {
                clone->setScanlineIntensity(0.9f);
                clone->setMaskType(CNA::Graphics::CRTMaskType::None);
            }
            Context().Check(cloneRetained && crt_->getScanlineIntensity() == 0.6f &&
                                crt_->getMaskType() == CNA::Graphics::CRTMaskType::ApertureGrille,
                            "Clone returns an independent CRTEffect with the complete current configuration");
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
             "CNA::Graphics::CRTEffect::getScanlineIntensity", "CNA::Graphics::CRTEffect::getCurvature",
             "CNA::Graphics::CRTEffect::setMaskType", "CNA::Graphics::CRTEffect::setMaskIntensity",
             "CNA::Graphics::CRTEffect::getMaskType", "CNA::Graphics::CRTEffect::getMaskIntensity",
             "CNA::Graphics::CRTEffect::setCurvature", "CNA::Graphics::CRTEffect::setVignetteIntensity",
             "CNA::Graphics::CRTEffect::getVignetteIntensity", "CNA::Graphics::CRTEffect::Clone",
             "CNA::Graphics::CRTEffect::GetTypeName",
             "CNA::Graphics::CRTMaskType", "SpriteBatch::Begin"},
    .tags = {"cnaext", "post-process", "crt", "retro", "shader", "render-target"},
    .needs = CnaDemos::Need::CustomEffects,
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb", "textures/gradient.png"},
    .restrictions = "Shader renderers only (GLSL or SPIR-V package); SOFTWARE and other 2D renderers skip it.",
})
