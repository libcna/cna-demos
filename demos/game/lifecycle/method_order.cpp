// SPDX-License-Identifier: MS-PL
// The order in which Game calls its overridable methods.
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class MethodOrderDemo final : public CnaDemos::DemoGame
    {
    public:
        MethodOrderDemo()
        {
            Disposed += [this](System::Object*, const System::EventArgs&) { ++disposed_; };
        }

        ~MethodOrderDemo() override
        {
            // Run() has returned and UnloadContent has run by the time the game is destroyed.
            Context().Print("calls: {}", Joined());
            Context().Check(HasTwoOrderedDrawCycles(),
                            "each draw cycle follows one or more Updates with BeginDraw -> Draw -> EndDraw");
            // As in XNA, UnloadContent is not part of Run: it runs when the game is disposed (the
            // runner calls Dispose after Run, like C#'s "using (var game = new Game1()) game.Run();").
            Context().Check(calls_.size() >= 2 && calls_[calls_.size() - 2] == "EndRun" && calls_.back() == "UnloadContent",
                            "Run ends with EndRun; Dispose then calls UnloadContent");
            Context().Check(disposed_ == 1, "Dispose raises the Game.Disposed event once");
        }

    protected:
        void Initialize() override
        {
            calls_.push_back("Initialize");
            // Game::Initialize initializes the components and then calls LoadContent.
            DemoGame::Initialize();
        }
        void LoadContent() override
        {
            calls_.push_back("LoadContent");
            Context().Check(&getGraphicsDeviceProperty() == Graphics().getGraphicsDeviceProperty(),
                            "Game.GraphicsDevice is the device supplied by its graphics manager");
        }
        void BeginRun() override { calls_.push_back("BeginRun"); }
        void Update(GameTime& gameTime) override
        {
            if (drawCycles_ < 2) calls_.push_back("Update");
            Game::Update(gameTime);
        }
        bool BeginDraw() override
        {
            if (drawCycles_ < 2) calls_.push_back("BeginDraw");
            return Game::BeginDraw();
        }
        void Draw(const GameTime& gameTime) override
        {
            if (drawCycles_ < 2) calls_.push_back("Draw");
            Game::Draw(gameTime);
        }
        void EndDraw() override
        {
            if (drawCycles_ < 2) calls_.push_back("EndDraw");
            ++drawCycles_;
            DemoGame::EndDraw();
        }
        void EndRun() override { calls_.push_back("EndRun"); }
        void UnloadContent() override { calls_.push_back("UnloadContent"); }

    private:
        [[nodiscard]] bool HasTwoOrderedDrawCycles() const
        {
            std::size_t at = 0;
            const auto take = [this, &at](const std::string& expected) {
                return at < calls_.size() && calls_[at++] == expected;
            };
            if (!take("Initialize") || !take("LoadContent") || !take("BeginRun"))
                return false;
            for (int cycle = 0; cycle < 2; ++cycle)
            {
                int updates = 0;
                while (at < calls_.size() && calls_[at] == "Update")
                {
                    ++at;
                    ++updates;
                }
                if (updates == 0 || !take("BeginDraw") || !take("Draw") || !take("EndDraw"))
                    return false;
            }
            return true;
        }

        std::string Joined() const
        {
            std::string text;
            for (const auto& call : calls_)
            {
                text += (text.empty() ? "" : " -> ") + call;
            }
            return text;
        }

        std::vector<std::string> calls_;
        int drawCycles_ = 0;
        int disposed_ = 0;
    };
}

CNA_DEMOS_GAME(MethodOrderDemo, {
    .id = "game.lifecycle.method_order",
    .title = "Game method order",
    .description = "Records initialization, two complete fixed-step update/draw cycles (including catch-up "
                   "Updates), run shutdown, content unloading and the Disposed event to check a complete Game run.",
    .apis = {"Game::Initialize", "Game::LoadContent", "Game::BeginRun", "Game::Update", "Game::BeginDraw",
             "Game::Draw", "Game::EndDraw", "Game::EndRun", "Game::UnloadContent", "Game::GraphicsDevice",
             "Game::Run", "Game::Dispose", "Game::Disposed"},
    .tags = {"lifecycle", "game-loop"},
    .frames = 5,
})
