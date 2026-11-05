// SPDX-License-Identifier: MS-PL
// Initialize with RunFrameEXT, then tick with Game::RunOneFrame instead of Game::Run.
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

        // The host-driven extension initializes and begins the run before its first tick.
        ctx.Check(game.RunFrameEXT(), "the initial host frame keeps the game running");
        ctx.Check(game.initializeCalls == 1 && game.updates == 1 && game.draws == 1,
                  "RunFrameEXT initializes once and performs the first Update and Draw");

        // XNA's RunOneFrame only ticks: it never initializes or begins a run.
        for (int frame = 1; frame <= 3; ++frame)
        {
            game.RunOneFrame();
            ctx.Print("after RunOneFrame #{}: {} update(s), {} draw(s)", frame, game.updates, game.draws);
        }

        // Tick is the public single-step primitive used by RunOneFrame.
        game.Tick();
        ctx.Print("after direct Tick: {} update(s), {} draw(s)", game.updates, game.draws);

        ctx.Check(game.initializeCalls == 1, "RunOneFrame and Tick do not initialize the game again");
        ctx.Check(game.updates == 5, "the host frame, three RunOneFrame calls and direct Tick each run one Update");
        ctx.Check(game.draws == 5, "and one Draw");
        game.Exit();
        ctx.Check(!game.RunFrameEXT(), "the host-driven extension ends the run after Exit");
        int disposed = 0;
        game.Disposed += [&disposed](System::Object*, const System::EventArgs&) { ++disposed; };
        game.Dispose();
        ctx.Check(disposed == 1, "Dispose raises Game.Disposed once");
    }
}

CNA_DEMOS_CONSOLE(RunOneFrameDemo, {
    .id = "game.lifecycle.run_one_frame",
    .title = "Step a game frame by frame",
    .description = "Initializes a Game with RunFrameEXT, then advances it with RunOneFrame and direct Tick; "
                   "with a variable time step each call performs one Update and Draw, and Dispose raises Disposed.",
    .apis = {"Game::RunFrameEXT", "Game::RunOneFrame", "Game::Tick", "Game::IsFixedTimeStep", "Game::GraphicsDevice",
             "Game::Dispose", "Game::Disposed"},
    .tags = {"lifecycle", "debugging", "game-loop"},
})
