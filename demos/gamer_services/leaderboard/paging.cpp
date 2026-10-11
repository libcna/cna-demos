// SPDX-License-Identifier: MS-PL
// LeaderboardReader paging: a board read a page at a time, each row with the columns the game wrote.
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
#include "Microsoft/Xna/Framework/GamerServices/LeaderboardOutcome.hpp"
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
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    // Four local players finish a round: score, result and time, in the order they sign in.
    constexpr const char* Players = "Demo Player,Second Player,Third Player,Fourth Player";
    struct Round
    {
        long long score;
        LeaderboardOutcome result;
        int seconds;
    };
    constexpr Round Rounds[] = {
        {4200, LeaderboardOutcome::Win, 112},
        {3100, LeaderboardOutcome::Tie, 130},
        {2600, LeaderboardOutcome::Loss, 141},
        {1900, LeaderboardOutcome::Loss, 155},
    };
    constexpr int PageSize = 2;

    const char* OutcomeName(LeaderboardOutcome outcome)
    {
        switch (outcome)
        {
            case LeaderboardOutcome::Win: return "Win";
            case LeaderboardOutcome::Loss: return "Loss";
            case LeaderboardOutcome::Tie: return "Tie";
            case LeaderboardOutcome::None: break;
        }
        return "None";
    }

    class PagingDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        PagingDemo() : GamerServicesDemo(Players) {}

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
            if (Gamer::getSignedInGamersProperty()->getCountProperty() < 4)
            {
                // Only when CNA_GAMER_SERVICES_AUTO_SIGN_IN names fewer players than this demo asks.
                if (++waited_ == 30)
                {
                    throw CnaDemos::SkipDemo("four signed-in players are needed (CNA_GAMER_SERVICES_AUTO_SIGN_IN)");
                }
                return;
            }
            done_ = true;
            // Game mode 1 of the best-score board: a board of its own (gamer_services.leaderboard.
            // write_and_read uses mode 0).
            const LeaderboardIdentity board = LeaderboardIdentity::Create(LeaderboardKey::BestScoreLifeTime, 1);
            WriteRound(board);
            ReadPages(board);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 30, 46));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "BestScoreLifeTime, mode 1 -- two rows per page", Vector2(40.0f, 30.0f),
                                     Color::White);
            float y = 80.0f;
            for (const std::string& line : lines_)
            {
                const bool heading = line.starts_with("page");
                spriteBatch_->DrawString(*font_, line, Vector2(heading ? 40.0f : 60.0f, y),
                                         heading ? Color::LightSkyBlue : Color::Gold);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        void WriteRound(const LeaderboardIdentity& board)
        {
            auto& ctx = Context();
            std::unique_ptr<NetworkSession> session(NetworkSession::Create(NetworkSessionType::LocalWithLeaderboards, 4, 4));
            session->StartGame();
            session->Update();
            int i = 0;
            for (LocalNetworkGamer* local : session->getLocalGamersProperty())
            {
                // Besides the rating, a row carries typed columns: the round's outcome and time.
                // (Columns before the rating: offline, CNA saves a row when its rating is set and
                // drops columns written after that, CNA-DEMOS-ISSUE-025.)
                LeaderboardEntry* entry =
                    local->getSignedInGamerProperty()->getLeaderboardWriterProperty().GetLeaderboard(board);
                entry->getColumnsProperty().SetValue("Result", Rounds[i].result);
                entry->getColumnsProperty().SetValue("Time", System::TimeSpan::FromSeconds(Rounds[i].seconds));
                entry->setRatingProperty(Rounds[i].score);
                ++i;
            }
            ctx.Check(i == 4, "four local gamers play the round");
            session->EndGame();
            session->Update();
            session->Dispose();
        }

        // Lists one page: rank, gamer, rating and the columns read back.
        void ListPage(const LeaderboardReader& reader)
        {
            auto& ctx = Context();
            const std::string heading = "page from " + std::to_string(reader.getPageStartProperty()) + " of " +
                                        std::to_string(reader.getTotalLeaderboardSizeProperty()) +
                                        (reader.getCanPageUpProperty() ? ", can page up" : "") +
                                        (reader.getCanPageDownProperty() ? ", can page down" : "");
            ctx.Print("{}", heading);
            lines_.push_back(heading);
            for (const LeaderboardEntry& entry : reader.getEntriesProperty())
            {
                const PropertyDictionary& columns = entry.getColumnsProperty();
                const int seconds = static_cast<int>(columns.GetValueTimeSpan("Time").getTotalSecondsProperty());
                const std::string line = std::to_string(entry.getRankingEXTProperty()) + ". " +
                                         entry.getGamerProperty()->getGamertagProperty() + "  " +
                                         std::to_string(entry.getRatingProperty()) + "  " +
                                         OutcomeName(columns.GetValueOutcome("Result")) + "  " +
                                         std::to_string(seconds / 60) + ":" + (seconds % 60 < 10 ? "0" : "") +
                                         std::to_string(seconds % 60);
                ctx.Print("  {}", line);
                lines_.push_back(line);
            }
        }

        void ReadPages(const LeaderboardIdentity& board)
        {
            auto& ctx = Context();
            // The first page; the reader knows the board's size and whether more pages exist.
            LeaderboardReader reader = LeaderboardReader::Read(board, 0, PageSize);
            ListPage(reader);
            ctx.Check(reader.getTotalLeaderboardSizeProperty() == 4 && reader.getEntriesProperty().getCountProperty() == 2,
                      "the first page holds two of the four rows");
            ctx.Check(reader.getCanPageDownProperty() && !reader.getCanPageUpProperty(),
                      "the first page can page down, not up");
            const auto first = reader.getEntriesProperty();
            ctx.Check(first[0].getColumnsProperty().GetValueOutcome("Result") == LeaderboardOutcome::Win,
                      "the leader's row reads back Result = Win");

            // PageDown moves the same reader to the next rows.
            reader.PageDown();
            ListPage(reader);
            ctx.Check(reader.getPageStartProperty() == 2 && !reader.getCanPageDownProperty() &&
                          reader.getCanPageUpProperty(),
                      "PageDown shows rows 3-4, the last page");
            const auto page = reader.getEntriesProperty();
            const PropertyDictionary& third = page[0].getColumnsProperty();
            ctx.Check(page[0].getGamerProperty()->getGamertagProperty() == "Third Player" &&
                          third.GetValueOutcome("Result") == LeaderboardOutcome::Loss &&
                          third.GetValueTimeSpan("Time") == System::TimeSpan::FromSeconds(141),
                      "Third Player's row reads back Result = Loss and Time = 2:21");

            reader.PageUp();
            const auto again = reader.getEntriesProperty();
            ctx.Check(reader.getPageStartProperty() == 0 &&
                          again[0].getGamerProperty()->getGamertagProperty() == "Demo Player",
                      "PageUp returns to the first page");
        }

        bool done_ = false;
        int waited_ = 0;
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(PagingDemo, {
    .id = "gamer_services.leaderboard.paging",
    .title = "Leaderboard pages and columns",
    .description = "Four local players write a round to a board -- rating plus a LeaderboardOutcome and a TimeSpan "
                   "column -- and LeaderboardReader reads it two rows at a time with PageDown and PageUp, the "
                   "columns read back with GetValueOutcome and GetValueTimeSpan.",
    .apis = {"LeaderboardReader::PageDown", "LeaderboardReader::PageUp", "LeaderboardReader::CanPageDown",
             "LeaderboardReader::CanPageUp", "LeaderboardReader::PageStart", "LeaderboardReader::TotalLeaderboardSize",
             "LeaderboardReader::Read", "LeaderboardOutcome", "PropertyDictionary::GetValueOutcome",
             "PropertyDictionary::GetValueTimeSpan", "PropertyDictionary::SetValue", "LeaderboardEntry::Columns",
             "LeaderboardEntry::RankingEXT", "LeaderboardReader::Entries"},
    .tags = {"gamer-services", "leaderboard", "high-score", "paging", "offline"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
