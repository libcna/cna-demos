// SPDX-License-Identifier: MS-PL
// Game::ResetElapsedTime: after a deliberate pause (loading a level), skip the catch-up updates.
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class ResetElapsedTimeDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Update(GameTime& gameTime) override
        {
            ++updates_;
            ++updatesThisFrame_;
            if (updates_ == afterLoad_)
            {
                // The first Update after the load: the time it took is not game time.
                firstAfterLoad_ = gameTime;
            }
            if (updates_ == 10)
            {
                // "Loading the next level": 200 ms of work, about twelve fixed steps. Without
                // the reset, the loop would make up for it with a burst of catch-up Updates
                // and report IsRunningSlowly; the player would see the game jump ahead.
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                ResetElapsedTime();
                afterLoad_ = updates_ + 1;
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            if (updates_ >= afterLoad_ && afterLoad_ > 0 && !checked_)
            {
                checked_ = true;
                Context().Print("updates in the frame after the load: {}, IsRunningSlowly {}", updatesThisFrame_,
                                firstAfterLoad_.getIsRunningSlowlyProperty());
                Context().Check(updatesThisFrame_ <= 2, "no burst of catch-up updates followed the load");
                Context().Check(!firstAfterLoad_.getIsRunningSlowlyProperty(), "the game is not reported as running slowly");
                Context().Check(firstAfterLoad_.getElapsedGameTimeProperty() == TargetElapsedTimeValue(),
                                "the next Update advances by one ordinary step");
            }
            updatesThisFrame_ = 0;
            Game::Draw(gameTime);
        }

        void EndRun() override
        {
            Context().Check(checked_, "the frame after the load was seen");
            DemoGame::EndRun();
        }

    private:
        System::TimeSpan TargetElapsedTimeValue() const { return getTargetElapsedTimeProperty(); }

        int updates_ = 0;
        int updatesThisFrame_ = 0;
        int afterLoad_ = 0;
        bool checked_ = false;
        GameTime firstAfterLoad_;
    };
}

CNA_DEMOS_GAME(ResetElapsedTimeDemo, {
    .id = "game.time_step.reset_elapsed_time",
    .title = "Skipping time after a load",
    .description = "Pauses one Update for 200 ms as if loading a level and calls Game::ResetElapsedTime: the next "
                   "frame runs one ordinary fixed step instead of a burst of catch-up Updates, and IsRunningSlowly "
                   "stays false.",
    .apis = {"Game::ResetElapsedTime", "GameTime::ElapsedGameTime", "GameTime::IsRunningSlowly",
             "Game::TargetElapsedTime"},
    .tags = {"time", "fixed-step", "loading"},
    .frames = 20,
    .restrictions = "Uses the wall clock (a deliberate 200 ms pause).",
    .knownIssue = "CNA-DEMOS-ISSUE-019",
})
