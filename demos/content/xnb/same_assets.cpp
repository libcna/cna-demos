// SPDX-License-Identifier: MS-PL
// The same sources compiled to CNB and to XNA's XNB: a SpriteFont, a SoundEffect and a Curve.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Xna::Framework::Audio::SoundEffect;

namespace
{
    class SameAssetsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& content = getContentProperty();
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());

            // content/ compiles to CNB, content_xnb/ to XNB (as an XNA content project wrote it).
            // The game loads either by name with the same ContentManager::Load<T>.
            cnbFont_.emplace(content.Load<SpriteFont>("fonts/pixel_font"));
            xnbFont_.emplace(content.Load<SpriteFont>("xnb/fonts/pixel_font"));
            const std::string sample = "The quick brown fox";
            ctx.Check(cnbFont_->MeasureString(sample) == xnbFont_->MeasureString(sample) &&
                          cnbFont_->getLineSpacingProperty() == xnbFont_->getLineSpacingProperty(),
                      "both SpriteFonts measure text identically");

            SoundEffect cnbSound = content.Load<SoundEffect>("sounds/chime");
            SoundEffect xnbSound = content.Load<SoundEffect>("xnb/sounds/chime");
            ctx.Print("chime: {} ms (CNB), {} ms (XNB)", cnbSound.getDurationProperty().getTotalMillisecondsProperty(),
                      xnbSound.getDurationProperty().getTotalMillisecondsProperty());
            ctx.Check(cnbSound.getDurationProperty() == xnbSound.getDurationProperty(), "both SoundEffects last as long");

            const Curve cnbCurve = content.Load<Curve>("curves/bounce");
            const Curve xnbCurve = content.Load<Curve>("xnb/curves/bounce");
            bool same = cnbCurve.getKeysProperty().getCountProperty() == xnbCurve.getKeysProperty().getCountProperty();
            for (float t = -0.5f; t <= 2.5f; t += 0.25f)
            {
                same = same && cnbCurve.Evaluate(t) == xnbCurve.Evaluate(t);
            }
            ctx.Check(same, "both Curves have the same keys and evaluate the same, loops included");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 36, 48));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*cnbFont_, "CNB font: The quick brown fox", Vector2(40.0f, 60.0f), Color::White);
            spriteBatch_->DrawString(*xnbFont_, "XNB font: The quick brown fox", Vector2(40.0f, 100.0f), Color::LightGreen);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> cnbFont_;
        std::optional<SpriteFont> xnbFont_;
    };
}

CNA_DEMOS_GAME(SameAssetsDemo, {
    .id = "content.xnb.same_assets",
    .title = "CNB and XNB side by side",
    .description = "Loads a SpriteFont, a SoundEffect and a Curve compiled from the same sources into CNA's CNB format "
                   "and into XNA's XNB format, and shows they behave identically: same text measurements, same "
                   "duration, same curve values.",
    .apis = {"ContentManager::Load", "SpriteFont::MeasureString", "SpriteFont::LineSpacing", "SoundEffect::Duration",
             "Curve::Evaluate", "Curve::Keys"},
    .tags = {"content", "xnb", "cnb", "font", "sound", "curve", "interoperability"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "xnb/fonts/pixel_font.xnb", "sounds/chime.cnb", "xnb/sounds/chime.xnb",
               "curves/bounce.cnb", "xnb/curves/bounce.xnb"},
})
