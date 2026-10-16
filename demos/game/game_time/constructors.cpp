// SPDX-License-Identifier: MS-PL
// GameTime constructors: building GameTime values for tests and tools that call Update themselves.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using System::TimeSpan;

namespace
{
    void Constructors(CnaDemos::DemoContext& ctx)
    {
        // A unit test of game logic steps it with hand-made times instead of a running Game.
        const GameTime zero;
        ctx.Check(zero.getTotalGameTimeProperty() == TimeSpan::Zero && zero.getElapsedGameTimeProperty() == TimeSpan::Zero &&
                      !zero.getIsRunningSlowlyProperty(),
                  "GameTime() is all zero and not running slowly");

        const TimeSpan step = TimeSpan::FromTicks(166667); // one 60 Hz frame, as XNA's default step
        const GameTime frame(TimeSpan::FromSeconds(10.0), step);
        ctx.Check(frame.getTotalGameTimeProperty() == TimeSpan::FromSeconds(10.0) &&
                      frame.getElapsedGameTimeProperty() == step && !frame.getIsRunningSlowlyProperty(),
                  "GameTime(total, elapsed) keeps both times");

        const GameTime boundary(TimeSpan::MinValue, TimeSpan::MaxValue);
        ctx.Check(boundary.getTotalGameTimeProperty() == TimeSpan::MinValue &&
                      boundary.getElapsedGameTimeProperty() == TimeSpan::MaxValue &&
                      !boundary.getIsRunningSlowlyProperty(),
                  "negative and extreme TimeSpan values are retained without validation or clamping");

        const GameTime slow(TimeSpan::FromSeconds(10.0), step, true);
        ctx.Check(slow.getTotalGameTimeProperty() == TimeSpan::FromSeconds(10.0) &&
                      slow.getElapsedGameTimeProperty() == step && slow.getIsRunningSlowlyProperty(),
                  "the third argument sets IsRunningSlowly without changing either time");

        // Stepping a simple simulation by hand, ten frames of 1/60 s.
        double position = 0.0;
        TimeSpan total = TimeSpan::Zero;
        for (int i = 0; i < 10; ++i)
        {
            total = total + step;
            const GameTime time(total, step);
            position += 3.0 * time.getElapsedGameTimeProperty().getTotalSecondsProperty(); // 3 units per second
        }
        ctx.Print("after 10 steps: total {:.4f} s, position {:.4f}", total.getTotalSecondsProperty(), position);
        ctx.CheckNear(position, 0.5, 1e-4, "ten 60 Hz steps at 3 units/s move about half a unit");
    }
}

CNA_DEMOS_CONSOLE(Constructors, {
    .id = "game.game_time.constructors",
    .title = "Making GameTime values",
    .description = "Builds GameTime values with all three constructors, including negative/extreme times and the "
                   "slow-running flag, then steps a small simulation by hand.",
    .apis = {"GameTime::GameTime", "GameTime::TotalGameTime", "GameTime::ElapsedGameTime", "GameTime::IsRunningSlowly"},
    .tags = {"time", "testing", "simulation"},
})
