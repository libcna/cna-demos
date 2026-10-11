// SPDX-License-Identifier: MS-PL
// SignedInGamer::GameDefaults: the player's preferred settings, for a game's initial options.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/GamerServicesNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GameDefaults.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::string YesNo(bool value) { return value ? "yes" : "no"; }

    std::string ColorText(const std::optional<Color>& color)
    {
        if (!color)
        {
            return "(none)";
        }
        return std::to_string(color->getRProperty()) + ", " + std::to_string(color->getGProperty()) + ", " +
            std::to_string(color->getBProperty());
    }

    class GameDefaultsDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            SignedInGamer* player = (*Gamer::getSignedInGamersProperty())[PlayerIndex::One];
            if (!lines_.empty() || player == nullptr)
            {
                return;
            }
            // A game starts a new player with these as its options instead of its own defaults:
            // the difficulty, inverted look, auto-aim... the player chose once, in the profile.
            const GameDefaults& defaults = player->getGameDefaultsProperty();
            lines_ = {
                "GameDifficulty:          " + std::string(CnaDemos::Name(defaults.getGameDifficultyProperty())),
                "ControllerSensitivity:   " + std::string(CnaDemos::Name(defaults.getControllerSensitivityProperty())),
                "InvertYAxis:             " + YesNo(defaults.getInvertYAxisProperty()),
                "AutoAim:                 " + YesNo(defaults.getAutoAimProperty()),
                "AutoCenter:              " + YesNo(defaults.getAutoCenterProperty()),
                "MoveWithRightThumbStick: " + YesNo(defaults.getMoveWithRightThumbStickProperty()),
                "PrimaryColor:            " + ColorText(defaults.getPrimaryColorProperty()),
                "SecondaryColor:          " + ColorText(defaults.getSecondaryColorProperty()),
                "ManualTransmission:      " + YesNo(defaults.getManualTransmissionProperty()),
                "RacingCameraAngle:       " + std::string(CnaDemos::Name(defaults.getRacingCameraAngleProperty())),
                "AccelerateWithButtons:   " + YesNo(defaults.getAccelerateWithButtonsProperty()),
                "BrakeWithButtons:        " + YesNo(defaults.getBrakeWithButtonsProperty()),
            };
            for (const std::string& line : lines_)
            {
                Context().Print("{}", line);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(32, 40, 32));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "SignedInGamer::GameDefaults", Vector2(40.0f, 20.0f), Color::White);
            float y = 60.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::PaleGreen);
                y += 28.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(GameDefaultsDemo, {
    .id = "gamer_services.signed_in_gamer.game_defaults",
    .title = "Player's game defaults",
    .description = "Shows the signed-in gamer's GameDefaults -- preferred difficulty, controller sensitivity, "
                   "inverted look, auto-aim, colours and driving settings -- that a game uses as the starting "
                   "options for that player.",
    .apis = {"SignedInGamer::GameDefaults", "GameDefaults::GameDifficulty", "GameDefaults::ControllerSensitivity",
             "GameDefaults::InvertYAxis", "GameDefaults::AutoAim", "GameDefaults::AutoCenter",
             "GameDefaults::MoveWithRightThumbStick", "GameDefaults::PrimaryColor", "GameDefaults::SecondaryColor",
             "GameDefaults::ManualTransmission", "GameDefaults::RacingCameraAngle",
             "GameDefaults::AccelerateWithButtons", "GameDefaults::BrakeWithButtons"},
    .tags = {"gamer-services", "profile", "options", "settings", "difficulty"},
    .frames = 20,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
