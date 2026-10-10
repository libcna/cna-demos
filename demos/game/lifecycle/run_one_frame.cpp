// SPDX-License-Identifier: MS-PL
// Driving a game one frame at a time with Game::RunOneFrame instead of Game::Run.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    // A plain Game (not a DemoGame): this demo owns the loop itself.
    class SteppedGame final : public Game
    {
    public:
        SteppedGame() : graphics_(this)
        {
            // With the default fixed time step, one tick may run several Updates to catch up
            // with real time. A variable step makes it exactly one Update per tick.
            setIsFixedTimeStepProperty(false);
        }

        int initializeCalls = 0;
        int updates = 0;
        int draws = 0;

    protected:
        void Initialize() override
        {
            ++initializeCalls;
            Game::Initialize();
        }
        void Update(GameTime& gameTime) override
        {
            ++updates;
            Game::Update(gameTime);
        }
        void Draw(const GameTime& gameTime) override
        {
            ++draws;
            getGraphicsDeviceProperty().Clear(Color::SteelBlue);
            Game::Draw(gameTime);
        }

    private:
        GraphicsDeviceManager graphics_;
    };

    void RunOneFrameDemo(CnaDemos::DemoContext& ctx)
    {
        SteppedGame game;

        // Each call runs exactly one tick; the first one also initializes the game. This is the
        // XNA way to step a game from a debugger or a test harness.
        for (int frame = 1; frame <= 3; ++frame)
        {
            game.RunOneFrame();
            ctx.Print("after RunOneFrame #{}: {} update(s), {} draw(s)", frame, game.updates, game.draws);
        }

        ctx.Check(game.initializeCalls == 1, "the game is initialized once, by the first call");
        ctx.Check(game.updates == 3, "each call runs one Update (variable time step)");
        ctx.Check(game.draws == 3, "and one Draw");
        game.Dispose();
    }
}

CNA_DEMOS_CONSOLE(RunOneFrameDemo, {
    .id = "game.lifecycle.run_one_frame",
    .title = "Step a game frame by frame",
    .description = "Creates a Game and advances it one tick at a time with Game::RunOneFrame instead of Run; with "
                   "a variable time step each tick is one Update and one Draw.",
    .apis = {"Game::RunOneFrame", "Game::IsFixedTimeStep", "Game::Dispose"},
    .tags = {"lifecycle", "debugging", "game-loop"},
})
