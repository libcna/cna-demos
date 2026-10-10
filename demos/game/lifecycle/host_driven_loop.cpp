// SPDX-License-Identifier: MS-PL
// A loop owned by the host: Game::RunFrameEXT (CNA extension), as a browser page needs.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class HostedGame final : public Game
    {
    public:
        HostedGame() : graphics_(this) {}

        int updates = 0;
        int beginRuns = 0;
        int endRuns = 0;

    protected:
        void BeginRun() override { ++beginRuns; }
        void EndRun() override { ++endRuns; }
        void Update(GameTime& gameTime) override
        {
            // The game still decides when it is finished.
            if (++updates == 5)
            {
                Exit();
            }
            Game::Update(gameTime);
        }
        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkOrange);
            Game::Draw(gameTime);
        }

    private:
        GraphicsDeviceManager graphics_;
    };

    void HostDrivenLoop(CnaDemos::DemoContext& ctx)
    {
        HostedGame game;

        // The host calls RunFrameEXT once per animation frame. The first call initializes and
        // begins the run; the call that finds the game exited ends it and returns false.
        int hostFrames = 0;
        while (game.RunFrameEXT())
        {
            ++hostFrames;
        }
        ctx.Print("RunFrameEXT returned true {} time(s); BeginRun {}, EndRun {}", hostFrames, game.beginRuns, game.endRuns);

        ctx.Check(game.beginRuns == 1 && game.endRuns == 1, "the run begins and ends exactly once");
        ctx.Check(game.updates == 5, "the game ran until it called Exit");
        ctx.Check(!game.RunFrameEXT(), "calls after the end return false and do nothing");
        game.Dispose();
    }
}

CNA_DEMOS_CONSOLE(HostDrivenLoop, {
    .id = "game.lifecycle.host_driven_loop",
    .title = "Host-driven game loop",
    .description = "Drives a Game from an external loop with the CNA extension Game::RunFrameEXT, the way a "
                   "browser host calls it once per animation frame.",
    .apis = {"Game::RunFrameEXT", "Game::BeginRun", "Game::EndRun", "Game::Exit"},
    .tags = {"lifecycle", "cnaext", "web", "game-loop"},
})
