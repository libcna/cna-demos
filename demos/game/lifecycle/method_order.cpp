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
        ~MethodOrderDemo() override
        {
            // Run() has returned and UnloadContent has run by the time the game is destroyed.
            Context().Print("calls: {}", Joined());
            const std::vector<std::string> expectedStart = {"Initialize", "LoadContent", "BeginRun", "Update", "Draw"};
            Context().Check(std::vector<std::string>(calls_.begin(), calls_.begin() + 5) == expectedStart,
                            "Initialize -> LoadContent -> BeginRun -> Update -> Draw");
            // As in XNA, UnloadContent is not part of Run: it runs when the game is disposed (the
            // runner calls Dispose after Run, like C#'s "using (var game = new Game1()) game.Run();").
            Context().Check(calls_.size() >= 2 && calls_[calls_.size() - 2] == "EndRun" && calls_.back() == "UnloadContent",
                            "Run ends with EndRun; Dispose then calls UnloadContent");
        }

    protected:
        void Initialize() override
        {
            calls_.push_back("Initialize");
            // Game::Initialize initializes the components and then calls LoadContent.
            DemoGame::Initialize();
        }
        void LoadContent() override { calls_.push_back("LoadContent"); }
        void BeginRun() override { calls_.push_back("BeginRun"); }
        void Update(GameTime& gameTime) override
        {
            Record("Update");
            Game::Update(gameTime);
        }
        void Draw(const GameTime& gameTime) override
        {
            Record("Draw");
            Game::Draw(gameTime);
        }
        void EndRun() override { calls_.push_back("EndRun"); }
        void UnloadContent() override { calls_.push_back("UnloadContent"); }

    private:
        // Only the first few frames are recorded, to keep the output short.
        void Record(const char* name)
        {
            if (++frameCalls_ <= 4)
            {
                calls_.push_back(name);
            }
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
        int frameCalls_ = 0;
    };
}

CNA_DEMOS_GAME(MethodOrderDemo, {
    .id = "game.lifecycle.method_order",
    .title = "Game method order",
    .description = "Records when Game calls Initialize, LoadContent, BeginRun, Update, Draw, EndRun and "
                   "UnloadContent (the last one on Dispose), and checks the order of a complete run.",
    .apis = {"Game::Initialize", "Game::LoadContent", "Game::BeginRun", "Game::Update", "Game::Draw", "Game::EndRun",
             "Game::UnloadContent", "Game::Run", "Game::Dispose"},
    .tags = {"lifecycle", "game-loop"},
    .frames = 5,
})
