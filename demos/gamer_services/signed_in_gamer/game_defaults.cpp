// SPDX-License-Identifier: MS-PL
// SignedInGamer::GameDefaults: the player's preferred settings, for a game's initial options.
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
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
    constexpr const char* Players = "Easy Player,Normal Player,Hard Player";

    struct ExpectedDefaults
    {
        const char* player;
        GameDifficulty difficulty;
        ControllerSensitivity sensitivity;
        RacingCameraAngle camera;
    };

    constexpr std::array<ExpectedDefaults, 3> Expected = {{
        {"Easy Player", GameDifficulty::Easy, ControllerSensitivity::Low, RacingCameraAngle::Back},
        {"Normal Player", GameDifficulty::Normal, ControllerSensitivity::Medium, RacingCameraAngle::Front},
        {"Hard Player", GameDifficulty::Hard, ControllerSensitivity::High, RacingCameraAngle::Inside},
    }};

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
    public:
        GameDefaultsDemo() : GamerServicesDemo(Players)
        {
            const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
            profilesDirectory_ = std::filesystem::temp_directory_path() /
                ("cna-demos-game-defaults-" + std::to_string(suffix));
            std::filesystem::create_directories(profilesDirectory_);
            std::ofstream profiles(profilesDirectory_ / "profiles.json", std::ios::binary);
            profiles << R"JSON({"version":1,"profiles":[
{"gamertag":"Easy Player","gameDefaults":{"gameDifficulty":"Easy","controllerSensitivity":"Low","racingCameraAngle":"Back"}},
{"gamertag":"Normal Player","gameDefaults":{"gameDifficulty":"Normal","controllerSensitivity":"Medium","racingCameraAngle":"Front"}},
{"gamertag":"Hard Player","gameDefaults":{"gameDifficulty":"Hard","controllerSensitivity":"High","racingCameraAngle":"Inside"}}
]})JSON";
            profiles.close();
            CnaDemos::SetEnvironment("CNA_GAMER_SERVICES_PROFILES_DIR", profilesDirectory_.string());
            CnaDemos::SetEnvironment("CNA_GAMER_SERVICES_AUTO_SIGN_IN", Players);
        }

        ~GameDefaultsDemo() override
        {
            std::error_code ignored;
            std::filesystem::remove_all(profilesDirectory_, ignored);
        }

    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            const auto players = Gamer::getSignedInGamersProperty();
            if (!lines_.empty())
            {
                return;
            }
            if (players->getCountProperty() < static_cast<int>(Expected.size()))
                return;

            bool allProfilesExact = true;
            for (std::size_t i = 0; i < Expected.size(); ++i)
            {
                SignedInGamer* player = (*players)[static_cast<PlayerIndex>(i)];
                const GameDefaults& defaults = player->getGameDefaultsProperty();
                allProfilesExact = allProfilesExact && player->getGamertagProperty() == Expected[i].player &&
                    defaults.getGameDifficultyProperty() == Expected[i].difficulty &&
                    defaults.getControllerSensitivityProperty() == Expected[i].sensitivity &&
                    defaults.getRacingCameraAngleProperty() == Expected[i].camera;
                lines_.push_back(player->getGamertagProperty() + ": " +
                    std::string(CnaDemos::Name(defaults.getGameDifficultyProperty())) + ", " +
                    std::string(CnaDemos::Name(defaults.getControllerSensitivityProperty())) + ", " +
                    std::string(CnaDemos::Name(defaults.getRacingCameraAngleProperty())));
            }
            Context().Check(allProfilesExact,
                            "three stored profiles preserve every difficulty, sensitivity and camera value");

            // A game starts a new player with these as its options instead of its own defaults:
            // the difficulty, inverted look, auto-aim... the player chose once, in the profile.
            const GameDefaults& defaults = (*players)[PlayerIndex::One]->getGameDefaultsProperty();
            lines_.insert(lines_.end(), {
                "InvertYAxis:             " + YesNo(defaults.getInvertYAxisProperty()),
                "AutoAim:                 " + YesNo(defaults.getAutoAimProperty()),
                "AutoCenter:              " + YesNo(defaults.getAutoCenterProperty()),
                "MoveWithRightThumbStick: " + YesNo(defaults.getMoveWithRightThumbStickProperty()),
                "PrimaryColor:            " + ColorText(defaults.getPrimaryColorProperty()),
                "SecondaryColor:          " + ColorText(defaults.getSecondaryColorProperty()),
                "ManualTransmission:      " + YesNo(defaults.getManualTransmissionProperty()),
                "AccelerateWithButtons:   " + YesNo(defaults.getAccelerateWithButtonsProperty()),
                "BrakeWithButtons:        " + YesNo(defaults.getBrakeWithButtonsProperty()),
            });
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
        std::filesystem::path profilesDirectory_;
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
             "GameDefaults::AccelerateWithButtons", "GameDefaults::BrakeWithButtons",
             "GameDifficulty", "GameDifficulty::Easy", "GameDifficulty::Normal", "GameDifficulty::Hard",
             "ControllerSensitivity", "ControllerSensitivity::Low", "ControllerSensitivity::Medium",
             "ControllerSensitivity::High", "RacingCameraAngle", "RacingCameraAngle::Back",
             "RacingCameraAngle::Front", "RacingCameraAngle::Inside"},
    .tags = {"gamer-services", "profile", "options", "settings", "difficulty"},
    .frames = 20,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
