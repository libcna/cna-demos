// SPDX-License-Identifier: MS-PL
// CNAEXT GamePad::SetLightBarEXT: the coloured light of PlayStation controllers.
#include <cmath>
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
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

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
        }

        void UnloadContent() override
        {
            // Leave the controller in a calm state; the platform restores its own colour later.
            GamePad::SetLightBarEXT(PlayerIndex::One, Color::Blue);
        }

        void Update(GameTime& gameTime) override
        {
            // A slow trip around the colour wheel, sent to the controller every frame.
            const float t = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
            colour_ = Color(Vector3(0.5f + 0.5f * std::sin(t), 0.5f + 0.5f * std::sin(t + MathHelper::TwoPi / 3.0f),
                                    0.5f + 0.5f * std::sin(t + 2.0f * MathHelper::TwoPi / 3.0f)));
            hasLightBar_ = GamePad::GetCapabilities(PlayerIndex::One).getHasLightBarEXTProperty();
            GamePad::SetLightBarEXT(PlayerIndex::One, colour_);
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_,
                                     hasLightBar_ ? "Controller one has a light bar: it follows this colour"
                                                  : "Controller one reports no light bar (HasLightBarEXT)",
                                     Vector2(30.0f, 30.0f), hasLightBar_ ? Color::White : Color::Gray);
            spriteBatch_->Draw(*pixel_, Rectangle(250, 150, 300, 60), colour_);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        Color colour_ = Color::Blue;
        bool hasLightBar_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(LightBarDemo, {
    .id = "input.gamepad.light_bar",
    .title = "Controller light bar (CNAEXT)",
    .description = "Cycles the light bar of controller one through the colour wheel with GamePad::SetLightBarEXT, "
                   "after asking GamePadCapabilities::HasLightBarEXT whether there is one.",
    .apis = {"GamePad::SetLightBarEXT", "GamePadCapabilities::HasLightBarEXT"},
    .tags = {"gamepad", "controller", "light-bar", "led", "playstation", "cnaext"},
    .automation = CnaDemos::Automation::SmokeOnly,
    .needs = CnaDemos::Need::GamePad,
    .assets = {"fonts/pixel_font.cnb"},
})
