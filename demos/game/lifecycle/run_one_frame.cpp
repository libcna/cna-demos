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

        // Tick is the public single-step primitive used by RunOneFrame after initialization.
        game.Tick();
        ctx.Print("after direct Tick: {} update(s), {} draw(s)", game.updates, game.draws);

        ctx.Check(game.initializeCalls == 1, "the game is initialized once, by the first call");
        ctx.Check(game.updates == 4, "each RunOneFrame and direct Tick runs one Update (variable time step)");
        ctx.Check(game.draws == 4, "and one Draw");
        int disposed = 0;
        game.Disposed += [&disposed](System::Object*, const System::EventArgs&) { ++disposed; };
        game.Dispose();
        ctx.Check(disposed == 1, "Dispose raises Game.Disposed once");
    }
}

CNA_DEMOS_CONSOLE(RunOneFrameDemo, {
    .id = "game.lifecycle.run_one_frame",
    .title = "Step a game frame by frame",
    .description = "Creates a Game and advances it with RunOneFrame and direct Tick; with a variable time step each "
                   "call performs one Update and Draw, and Dispose raises Disposed.",
    .apis = {"Game::RunOneFrame", "Game::Tick", "Game::IsFixedTimeStep", "Game::GraphicsDevice",
             "Game::Dispose", "Game::Disposed"},
    .tags = {"lifecycle", "debugging", "game-loop"},
})
