// SPDX-License-Identifier: MS-PL
// LeaderboardWriter and LeaderboardReader: high scores of a LocalWithLeaderboards game, offline.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/LeaderboardEntry.hpp"
#include "Microsoft/Xna/Framework/GamerServices/LeaderboardIdentity.hpp"
#include "Microsoft/Xna/Framework/GamerServices/LeaderboardKey.hpp"
#include "Microsoft/Xna/Framework/GamerServices/LeaderboardReader.hpp"
#include "Microsoft/Xna/Framework/GamerServices/LeaderboardWriter.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Net/LocalNetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionState.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    // Two local players, each with a final score this round.
    constexpr const char* Players = "Demo Player,Second Player";
    constexpr long long Scores[] = {1500, 900};

    class LeaderboardDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        LeaderboardDemo() : GamerServicesDemo(Players) {}

    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (done_)
            {
                return;
            }
            if (Gamer::getSignedInGamersProperty()->getCountProperty() < 2)
            {
                // Only when CNA_GAMER_SERVICES_AUTO_SIGN_IN names fewer players than this demo asks.
                if (++waited_ == 30)
                {
                    throw CnaDemos::SkipDemo("two signed-in players are needed (CNA_GAMER_SERVICES_AUTO_SIGN_IN)");
                }
                return;
            }
            done_ = true;
            auto& ctx = Context();
            const LeaderboardIdentity board = LeaderboardIdentity::Create(LeaderboardKey::BestScoreLifeTime, 0);

            // As on XNA, leaderboards are written during gameplay of a session that has them:
            // LocalWithLeaderboards is the split-screen kind (no network).
            std::unique_ptr<NetworkSession> session(NetworkSession::Create(NetworkSessionType::LocalWithLeaderboards, 2, 2));
            session->StartGame();
            session->Update();
            ctx.Check(session->getSessionStateProperty() == NetworkSessionState::Playing, "the game is being played");

            int i = 0;
            for (LocalNetworkGamer* local : session->getLocalGamersProperty())
            {
                SignedInGamer* gamer = local->getSignedInGamerProperty();
                LeaderboardEntry* entry = gamer->getLeaderboardWriterProperty().GetLeaderboard(board);
                // A best-score board keeps the player's best: write only an improvement.
                if (Scores[i] > entry->getRatingProperty())
                {
                    entry->getColumnsProperty().SetValue("Level", 3);
                    entry->setRatingProperty(Scores[i]);
                }
                ctx.Check(entry->getRatingProperty() >= Scores[i], gamer->getGamertagProperty() + "'s entry holds the score");
                ++i;
            }
            session->EndGame();
            session->Update();
            session->Dispose();

            // Reading needs no session. Offline, the board holds this machine's profiles.
            LeaderboardReader top = LeaderboardReader::Read(board, 0, 10);
            ctx.Print("{} entries", top.getTotalLeaderboardSizeProperty());
            ctx.Check(top.getTotalLeaderboardSizeProperty() >= 2 && top.getPageStartProperty() == 0,
                      "the first page lists both players");
            long long previous = -1;
            bool sorted = true;
            for (const LeaderboardEntry& entry : top.getEntriesProperty())
            {
                sorted = sorted && (previous < 0 || entry.getRatingProperty() <= previous);
                previous = entry.getRatingProperty();
                const std::string line = std::to_string(entry.getRankingEXTProperty()) + ". " +
                    entry.getGamerProperty()->getGamertagProperty() + "  " + std::to_string(entry.getRatingProperty());
                ctx.Print("{}", line);
                lines_.push_back(line);
            }
            ctx.Check(sorted, "entries come best first");

            // A page centred on one gamer: "where am I?".
            SignedInGamer* second = (*Gamer::getSignedInGamersProperty())[PlayerIndex::Two];
            LeaderboardReader around = LeaderboardReader::Read(board, second, 5);
            bool found = false;
            for (const LeaderboardEntry& entry : around.getEntriesProperty())
            {
                found = found || entry.getGamerProperty()->getGamertagProperty() == second->getGamertagProperty();
            }
            ctx.Check(found, "a page read around Second Player contains Second Player");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 30, 46));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "BestScoreLifeTime (offline)", Vector2(40.0f, 30.0f), Color::White);
            float y = 80.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::Gold);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        bool done_ = false;
        int waited_ = 0;
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(LeaderboardDemo, {
    .id = "gamer_services.leaderboard.write_and_read",
    .title = "Leaderboards",
    .description = "Two local players finish a LocalWithLeaderboards game and write their scores through "
                   "LeaderboardWriter (an improvement only, plus a column); LeaderboardReader then reads the "
                   "board best first, and a page around one player.",
    .apis = {"LeaderboardWriter::GetLeaderboard", "Gamer::LeaderboardWriter", "LeaderboardEntry::Rating",
             "LeaderboardEntry::Columns", "LeaderboardEntry::Gamer", "LeaderboardIdentity::Create", "LeaderboardKey",
             "LeaderboardReader::Read", "LeaderboardReader::Entries", "LeaderboardReader::TotalLeaderboardSize",
             "LeaderboardReader::PageStart", "PropertyDictionary::SetValue", "NetworkSession::Create",
             "NetworkSession::StartGame", "NetworkSession::EndGame", "NetworkSessionType"},
    .tags = {"gamer-services", "leaderboard", "high-score", "multiplayer", "offline"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
