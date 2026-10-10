// SPDX-License-Identifier: MS-PL
// Variable time step: one Update per Draw, with the real elapsed time.
#include "CnaDemos/Demo.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class VariableStepDemo final : public CnaDemos::DemoGame
    {
    public:
        VariableStepDemo()
        {
            // Update and Draw once per frame, as fast as the platform presents; ElapsedGameTime
            // is whatever time really passed. Movement must then scale by it.
            setIsFixedTimeStepProperty(false);
        }

    protected:
        void Update(GameTime& gameTime) override
        {
            ++updates_;
            const double ms = gameTime.getElapsedGameTimeProperty().getTotalMillisecondsProperty();
            if (updates_ > 1)
            {
                anyNonZero_ = anyNonZero_ || ms > 0.0;
            }
            if (updates_ <= 4)
            {
                Context().Print("update {}: elapsed {:.3f} ms", updates_, ms);
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
            Context().Check(!getIsFixedTimeStepProperty(), "IsFixedTimeStep is off");
            Context().Check(updates_ == draws_, "with a variable step, Update and Draw alternate one to one");
            Context().Check(anyNonZero_, "the elapsed time reflects real time");
        }

    private:
        int updates_ = 0;
        int draws_ = 0;
        bool anyNonZero_ = false;
    };
}

CNA_DEMOS_GAME(VariableStepDemo, {
    .id = "game.time_step.variable",
    .title = "Variable time step",
    .description = "Turns IsFixedTimeStep off so Update and Draw run once per frame with the real elapsed time.",
    .apis = {"Game::IsFixedTimeStep", "GameTime::ElapsedGameTime"},
    .tags = {"time", "variable-step"},
    .frames = 20,
})
