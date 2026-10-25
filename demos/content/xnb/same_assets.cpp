// SPDX-License-Identifier: MS-PL
// The same sources compiled to CNB and to XNA's XNB: a SpriteFont, a SoundEffect and a Curve.
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
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

            // This is an original MonoGame-produced file, not output from CNA's own writer. Its
            // container flag proves the body is LZX compressed; loading it then exercises CNA's
            // decompressor and Texture2DReader as one ordinary ContentManager workflow.
            const auto lzxFile = TitleContainer::OpenStream("assets/xnb/monogame/Explosion.xnb");
            std::array<std::uint8_t, 10> header{};
            const int headerBytes = lzxFile->Read(header.data(), 0, static_cast<int>(header.size()));
            ctx.Check(headerBytes == static_cast<int>(header.size()) &&
                          header[0] == 'X' && header[1] == 'N' && header[2] == 'B' &&
                          header[3] == 'w' && header[4] == 5u && (header[5] & 0x80u) != 0,
                      "the external MonoGame fixture is a Windows v5 LZX-compressed XNB");

            lzxTexture_.emplace(content.Load<Texture2D>("xnb/monogame/Explosion"));
            std::vector<Color> pixels(64u * 64u);
            lzxTexture_->GetData(pixels.data(), static_cast<int>(pixels.size()));
            ctx.Check(lzxTexture_->getWidthProperty() == 64 &&
                          lzxTexture_->getHeightProperty() == 64 &&
                          lzxTexture_->getLevelCountProperty() == 1 && pixels.size() == 4096u,
                      "ContentManager decompresses and loads the external LZX Texture2D payload");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 36, 48));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*cnbFont_, "CNB font: The quick brown fox", Vector2(40.0f, 60.0f), Color::White);
            spriteBatch_->DrawString(*xnbFont_, "XNB font: The quick brown fox", Vector2(40.0f, 100.0f), Color::LightGreen);
            spriteBatch_->Draw(*lzxTexture_, Rectangle(40, 150, 128, 128), Color::White);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> cnbFont_;
        std::optional<SpriteFont> xnbFont_;
        std::optional<Texture2D> lzxTexture_;
    };
}

CNA_DEMOS_GAME(SameAssetsDemo, {
    .id = "content.xnb.same_assets",
    .title = "CNB and XNB side by side",
    .description = "Loads equivalent CNB/XNB fonts, sounds and curves, then verifies and renders a real externally "
                   "produced MonoGame LZX-compressed XNB texture through CNA's ordinary ContentManager path.",
    .apis = {"ContentManager::Load", "SpriteFont::MeasureString", "SpriteFont::LineSpacing", "SoundEffect::Duration",
             "Curve::Evaluate", "Curve::Keys", "TitleContainer::OpenStream", "Texture2D::GetData"},
    .tags = {"content", "xnb", "cnb", "lzx", "monogame", "font", "sound", "curve", "interoperability"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "xnb/fonts/pixel_font.xnb", "sounds/chime.cnb", "xnb/sounds/chime.xnb",
               "curves/bounce.cnb", "xnb/curves/bounce.xnb", "xnb/monogame/Explosion.xnb"},
})
