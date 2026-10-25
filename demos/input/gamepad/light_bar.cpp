// SPDX-License-Identifier: MS-PL
// CNAEXT GamePad::SetLightBarEXT: the coloured light of PlayStation controllers.
#include <array>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadCapabilities.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class LightBarDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
            hasLightBar_ = GamePad::GetCapabilities(PlayerIndex::One).getHasLightBarEXTProperty();
            Context().Check(hasLightBar_ ==
                                GamePad::GetCapabilities(PlayerIndex::One).getHasLightBarEXTProperty(),
                            "light-bar capability inspection is stable and read-only");
            previousKeyboard_ = Keyboard::GetState();
        }

        void Update(GameTime& gameTime) override
        {
            const KeyboardState keyboard = Keyboard::GetState();
            if (!Context().Automated() && keyboard.IsKeyDown(Keys::Space) &&
                previousKeyboard_.IsKeyUp(Keys::Space))
            {
                static const std::array<Color, 4> colors = {
                    Color::Red, Color::Lime, Color::Blue, Color::White};
                colour_ = colors[colorIndex_++ % colors.size()];
                GamePad::SetLightBarEXT(PlayerIndex::One, colour_);
                wroteLightBar_ = true;
            }
            previousKeyboard_ = keyboard;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_,
                                     hasLightBar_ ? "Space: explicitly set the next light-bar colour once"
                                                  : "Controller one reports no light bar (HasLightBarEXT)",
                                     Vector2(30.0f, 30.0f), hasLightBar_ ? Color::White : Color::Gray);
            spriteBatch_->Draw(*pixel_, Rectangle(250, 150, 300, 60), colour_);
            spriteBatch_->DrawString(*font_,
                                     wroteLightBar_ ? "Changed by request; the API cannot read/restore the old colour"
                                                    : "No light-bar write has been issued",
                                     Vector2(30.0f, 240.0f), Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        Color colour_ = Color::Blue;
        bool hasLightBar_ = false;
        bool wroteLightBar_ = false;
        std::size_t colorIndex_ = 0;
        KeyboardState previousKeyboard_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(LightBarDemo, {
    .id = "input.gamepad.light_bar",
    .title = "Controller light bar (CNAEXT)",
    .description = "Inspects HasLightBarEXT without side effects. Each fresh Space press in an attended run writes "
                   "one new colour with SetLightBarEXT; the public API has no light-bar getter, so the demo neither "
                   "claims restoration nor writes an invented replacement value during shutdown.",
    .apis = {"GamePad::SetLightBarEXT", "GamePadCapabilities::HasLightBarEXT"},
    .tags = {"gamepad", "controller", "light-bar", "led", "playstation", "cnaext", "explicit-consent"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = {"Unattended runs never write the light bar. CNA exposes no getter, so an explicitly requested "
                     "colour cannot be restored reliably; shutdown deliberately performs no guessed write."},
})
