// SPDX-License-Identifier: MS-PL
// Falling behind a fixed time step: GameTime::IsRunningSlowly and catch-up updates.
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class RunningSlowlyDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Update(GameTime& gameTime) override
        {
            ++updates_;
            if (gameTime.getIsRunningSlowlyProperty())
            {
                ++slowUpdates_;
            }
            // Simulate a hitch: 150 ms of work against a 16.7 ms budget (60 Hz). The loop then owes
            // about nine fixed steps; after falling that far behind it flags IsRunningSlowly.
            if (updates_ == 5)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            ++draws_;
            Game::Draw(gameTime);
        }

        void EndRun() override
        {
            Context().Print("{} updates, {} draws, {} update(s) flagged IsRunningSlowly", updates_, draws_, slowUpdates_);
            // To keep game time correct, the loop runs extra Updates without drawing in between.
            Context().Check(updates_ > draws_, "the game caught up with extra updates");
            Context().Check(slowUpdates_ > 0, "catch-up updates report IsRunningSlowly");
        }

    private:
        int updates_ = 0;
        int draws_ = 0;
        int slowUpdates_ = 0;
    };
}

CNA_DEMOS_GAME(RunningSlowlyDemo, {
    .id = "game.time_step.running_slowly",
    .title = "Running slowly",
    .description = "Stalls one Update for about nine frames' worth of time and shows the fixed-step loop catching "
                   "up with extra Updates, after which GameTime::IsRunningSlowly is set until it recovers.",
    .apis = {"GameTime::IsRunningSlowly", "Game::IsFixedTimeStep"},
    .tags = {"time", "fixed-step", "performance"},
    .frames = 15,
    .restrictions = "Uses the wall clock (a deliberate 150 ms stall), so the exact counts vary between runs.",
})
