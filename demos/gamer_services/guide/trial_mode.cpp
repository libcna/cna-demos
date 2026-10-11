// SPDX-License-Identifier: MS-PL
// Guide::IsTrialMode and SimulateTrialMode: testing a game's trial version.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Guide.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class TrialModeDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void Initialize() override
        {
            DemoGame::Initialize();
            // Like XNA, the Guide reports trial mode until gamer services have run once.
            Context().Check(Guide::getIsTrialModeProperty(), "IsTrialMode is true before the first Update");
        }

        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        // SimulateTrialMode is process-wide; leave it off.
        void UnloadContent() override { Guide::setSimulateTrialModeProperty(false); }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime); // the dispatcher updates IsTrialMode here
            auto& ctx = Context();
            ++updates_;
            if (updates_ == 1)
            {
                ctx.Check(!Guide::getIsTrialModeProperty(), "a CNA title is fully licensed: IsTrialMode is false");
            }
            else if (updates_ == 10)
            {
                // While developing, SimulateTrialMode lets the trial version be played.
                Guide::setSimulateTrialModeProperty(true);
                ctx.Check(!Guide::getIsTrialModeProperty(), "IsTrialMode changes only at the next dispatcher update");
            }
            else if (updates_ == 11)
            {
                ctx.Check(Guide::getIsTrialModeProperty(), "with SimulateTrialMode the game runs as a trial");
            }
            else if (updates_ == 40)
            {
                Guide::setSimulateTrialModeProperty(false);
            }
            else if (updates_ == 41)
            {
                ctx.Check(!Guide::getIsTrialModeProperty(), "turning the simulation off restores the full version");
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            const bool trial = Guide::getIsTrialModeProperty();
            getGraphicsDeviceProperty().Clear(trial ? Color(70, 40, 30) : Color(30, 50, 70));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Guide::IsTrialMode", Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, trial ? "TRIAL VERSION - level 1 only" : "Full version - all levels",
                                     Vector2(40.0f, 80.0f), trial ? Color::Orange : Color::LightGreen);
            spriteBatch_->DrawString(*font_,
                                     std::string("SimulateTrialMode: ") +
                                         (Guide::getSimulateTrialModeProperty() ? "true" : "false"),
                                     Vector2(40.0f, 120.0f), Color::LightGray);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        int updates_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(TrialModeDemo, {
    .id = "gamer_services.guide.trial_mode",
    .title = "Trial mode",
    .description = "Shows Guide::IsTrialMode (true until gamer services first run, then false: CNA titles are fully "
                   "licensed) and switches Guide::SimulateTrialMode on and off, which the dispatcher picks up at its "
                   "next update; the screen shows the trial or full version.",
    .apis = {"Guide::IsTrialMode", "Guide::SimulateTrialMode", "GamerServicesDispatcher::Update"},
    .tags = {"gamer-services", "guide", "trial", "licence", "testing"},
    .frames = 60,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
