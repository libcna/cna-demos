// SPDX-License-Identifier: MS-PL
// Skipping a frame's Draw with Game::SuppressDraw.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class SuppressDrawDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Update(GameTime& gameTime) override
        {
            // Nothing visible changed (e.g. a paused, static screen): skip drawing this frame.
            if (++updates_ % 2 == 0)
            {
                SuppressDraw();
                ++suppressed_;
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            ++draws_;
            getGraphicsDeviceProperty().Clear(Color::Teal);
            Game::Draw(gameTime);
        }

        void EndRun() override
        {
            Context().Print("{} updates, {} suppressed, {} draws", updates_, suppressed_, draws_);
            Context().Check(draws_ <= updates_ - suppressed_ + 1, "every suppressed frame skipped its Draw");
            Context().Check(suppressed_ > 0, "some frames were suppressed");
        }

    private:
        int updates_ = 0;
        int suppressed_ = 0;
        int draws_ = 0;
    };
}

CNA_DEMOS_GAME(SuppressDrawDemo, {
    .id = "game.time_step.suppress_draw",
    .title = "Suppress a frame's Draw",
    .description = "Calls Game::SuppressDraw on every other Update so those frames skip Draw.",
    .apis = {"Game::SuppressDraw"},
    .tags = {"game-loop", "performance"},
    .frames = 10,
})
