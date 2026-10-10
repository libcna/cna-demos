// SPDX-License-Identifier: MS-PL
// Fixed time step: IsFixedTimeStep, TargetElapsedTime and what GameTime reports.
#include <cstdint>

#include "CnaDemos/Demo.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class FixedStepDemo final : public CnaDemos::DemoGame
    {
    public:
        FixedStepDemo()
        {
            // 30 updates per second. With a fixed time step (the default), Update always sees
            // exactly this ElapsedGameTime, however fast the machine is.
            setIsFixedTimeStepProperty(true);
            setTargetElapsedTimeProperty(System::TimeSpan::FromTicks(333333));
        }

    protected:
        void Update(GameTime& gameTime) override
        {
            const auto elapsed = gameTime.getElapsedGameTimeProperty().getTicksProperty();
            const auto total = gameTime.getTotalGameTimeProperty().getTicksProperty();
            if (updates_ < 4)
            {
                Context().Print("update {}: ElapsedGameTime {} ticks, TotalGameTime {} ticks", updates_, elapsed, total);
            }
            if (updates_ == 0)
            {
                // XNA quirk kept by CNA: the very first Update has a zero elapsed time.
                Context().Check(elapsed == 0, "the first update reports zero elapsed time");
            }
            else
            {
                allFixed_ = allFixed_ && elapsed == 333333;
                // TotalGameTime is the game time at the START of this step (XNA's clock; it
                // advances once Update returns): update n sees (n - 1) steps.
                totalMatches_ = totalMatches_ && total == static_cast<std::int64_t>(updates_ - 1) * 333333;
            }
            ++updates_;
            Game::Update(gameTime);
        }

        void EndRun() override
        {
            Context().Check(getIsFixedTimeStepProperty(), "IsFixedTimeStep is on");
            Context().Check(getTargetElapsedTimeProperty().getTicksProperty() == 333333, "TargetElapsedTime is 1/30 s");
            Context().Check(allFixed_, "every later update advances by exactly TargetElapsedTime");
            Context().Check(totalMatches_, "TotalGameTime is the time before the current step");
        }

    private:
        int updates_ = 0;
        bool allFixed_ = true;
        bool totalMatches_ = true;
    };
}

CNA_DEMOS_GAME(FixedStepDemo, {
    .id = "game.time_step.fixed",
    .title = "Fixed time step",
    .description = "Runs the game at a fixed 30 updates per second via TargetElapsedTime and shows XNA's clock: "
                   "a zero first step, then exactly one step per Update, with TotalGameTime measured before the step.",
    .apis = {"Game::IsFixedTimeStep", "Game::TargetElapsedTime", "GameTime::ElapsedGameTime", "GameTime::TotalGameTime"},
    .tags = {"time", "fixed-step", "determinism"},
    .frames = 20,
})
