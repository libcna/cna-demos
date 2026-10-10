// SPDX-License-Identifier: MS-PL
// Ending the game: Game::Exit, the Exiting event and Game::OnExiting.
#include "CnaDemos/Demo.hpp"
#include "System/EventArgs.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class ExitDemo final : public CnaDemos::DemoGame
    {
    public:
        ExitDemo()
        {
            // Subscribers to Exiting are told once, as the loop ends.
            Exiting += [this](System::Object*, const System::EventArgs&) { ++exitingEvents_; };
        }

        ~ExitDemo() override
        {
            Context().Print("updates: {}, OnExiting calls: {}, Exiting events: {}", updates_, onExitingCalls_, exitingEvents_);
            Context().Check(updates_ == 3, "no Update runs after the one that called Exit");
            Context().Check(onExitingCalls_ == 1 && exitingEvents_ == 1, "OnExiting and the Exiting event happen once");
        }

    protected:
        void Update(GameTime& gameTime) override
        {
            // A game decides to quit (menu choice, end of level...) and calls Exit. The current
            // frame finishes; the loop then stops.
            if (++updates_ == 3)
            {
                Exit();
            }
            Game::Update(gameTime);
        }

        void OnExiting(System::Object* sender, const System::EventArgs& args) override
        {
            ++onExitingCalls_;
            // The base implementation raises the Exiting event.
            Game::OnExiting(sender, args);
        }

    private:
        int updates_ = 0;
        int onExitingCalls_ = 0;
        int exitingEvents_ = 0;
    };
}

CNA_DEMOS_GAME(ExitDemo, {
    .id = "game.lifecycle.exit",
    .title = "Exit the game",
    .description = "Calls Game::Exit from Update after three frames and observes OnExiting and the Exiting event; "
                   "no further Update runs.",
    .apis = {"Game::Exit", "Game::Exiting", "Game::OnExiting"},
    .tags = {"lifecycle", "quit", "event"},
    .frames = 60,
})
