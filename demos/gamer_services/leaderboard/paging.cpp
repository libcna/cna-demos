// SPDX-License-Identifier: MS-PL
// LeaderboardReader paging: a board read a page at a time, each row with the columns the game wrote.
#include <any>
#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
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
#include "System/DateTime.hpp"
#include "System/ArgumentException.hpp"
#include "System/Collections/Generic/KeyValuePair.hpp"
#include "System/IO/Stream.hpp"
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
        {1900, LeaderboardOutcome::None, 155},
    };
    constexpr std::array<LeaderboardKey, 4> BoardKeys = {
        LeaderboardKey::BestScoreLifeTime, LeaderboardKey::BestScoreRecent,
        LeaderboardKey::BestTimeLifeTime, LeaderboardKey::BestTimeRecent};
    constexpr std::array<const char*, 4> BoardNames = {
        "BestScoreLifeTime", "BestScoreRecent", "BestTimeLifeTime", "BestTimeRecent"};
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
            bool allBoardKeysExact = true;
            for (std::size_t i = 0; i < BoardKeys.size(); ++i)
            {
                const LeaderboardIdentity identity = LeaderboardIdentity::Create(BoardKeys[i], static_cast<int>(i + 2));
                allBoardKeysExact = allBoardKeysExact && identity.getKeyProperty() == BoardNames[i] &&
                    identity.getGameModeProperty() == static_cast<int>(i + 2);
            }
            Context().Check(allBoardKeysExact,
                            "all four stock leaderboard keys create their exact independent board identities");
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
            bool writerColumnsExact = true;
            bool writerStreamsExact = true;
            for (LocalNetworkGamer* local : session->getLocalGamersProperty())
            {
                // Besides the rating, a row carries typed columns: the round's outcome and time.
                // (Columns before the rating: offline, CNA saves a row when its rating is set and
                // drops columns written after that, CNA-DEMOS-ISSUE-025.)
                LeaderboardEntry* entry =
                    local->getSignedInGamerProperty()->getLeaderboardWriterProperty().GetLeaderboard(board);
                PropertyDictionary& columns = entry->getColumnsProperty();
                columns.SetValue("Result", Rounds[i].result);
                columns.SetValue("Time", System::TimeSpan::FromSeconds(Rounds[i].seconds));
                columns.SetValue("Round", i + 1);
                columns.SetValue("ScoreCopy", Rounds[i].score);
                columns.SetValue("Accuracy", 0.75f - static_cast<float>(i) * 0.05f);
                columns.SetValue("Multiplier", 1.5 + static_cast<double>(i));
                columns.SetValue("Recorded", System::DateTime(2026, 10, 14));
                columns.SetValue("Note", std::string("round ") + std::to_string(i + 1));

                try
                {
                    System::IO::Stream* replay = columns.GetValueStream("Replay");
                    std::array<SharpRuntime::bytecs, 3> replayBytes{
                        static_cast<SharpRuntime::bytecs>(i + 1), 0x43, 0x4e};
                    replay->Write(replayBytes.data(), 0, static_cast<int>(replayBytes.size()));
                    replay->setPositionProperty(0);
                    std::array<SharpRuntime::bytecs, 3> replayRead{};
                    writerStreamsExact = writerStreamsExact &&
                        replay->Read(replayRead.data(), 0, static_cast<int>(replayRead.size())) ==
                            static_cast<int>(replayRead.size()) &&
                        replayRead == replayBytes;
                    replay->setPositionProperty(0);
                }
                catch (const std::out_of_range&)
                {
                    writerStreamsExact = false;
                }
                // A repeated run restores Replay before these scalar writes. Check the
                // same complete shape after stream creation on both first and later runs.
                std::any round;
                writerColumnsExact = writerColumnsExact && columns.getCountProperty() == 9 &&
                    columns.ContainsKey("Recorded") && columns.ContainsKey("Replay") && !columns.ContainsKey("Missing") &&
                    columns.TryGetValue("Round", round) && std::any_cast<int>(round) == i + 1;
                entry->setRatingProperty(Rounds[i].score);
                ++i;
            }
            ctx.Check(i == 4, "four local gamers play the round");
            ctx.Check(writerColumnsExact,
                      "writer dictionaries expose exact Count, ContainsKey, TryGetValue and Item values");
            writerStreamsExact_ = writerStreamsExact;
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
            // Offline leaderboard work completes synchronously, but the public Begin/End contract
            // still invokes its callback once and preserves caller state.
            int readCallbacks = 0;
            bool readStateMatched = false;
            std::unique_ptr<System::IAsyncResult> readResult(LeaderboardReader::BeginRead(
                board, 0, PageSize,
                [&readCallbacks, &readStateMatched](System::IAsyncResult& result) {
                    ++readCallbacks;
                    readStateMatched = std::any_cast<std::string>(result.getAsyncStateProperty()) == "initial page";
                },
                std::string("initial page")));
            ctx.Check(readCallbacks == 1 && readStateMatched && readResult->getIsCompletedProperty() &&
                          readResult->getCompletedSynchronouslyProperty(),
                      "BeginRead completes offline, invokes one callback, and preserves caller state");
            LeaderboardReader reader = LeaderboardReader::EndRead(readResult.get());
            ctx.Check(reader.getLeaderboardIdentityProperty().getKeyProperty() == board.getKeyProperty() &&
                          reader.getLeaderboardIdentityProperty().getGameModeProperty() == board.getGameModeProperty(),
                      "EndRead retains the requested leaderboard key and game mode");

            // The first page knows the board's size and whether more pages exist.
            ListPage(reader);
            ctx.Check(reader.getTotalLeaderboardSizeProperty() == 4 && reader.getEntriesProperty().getCountProperty() == 2,
                      "the first page holds two of the four rows");
            ctx.Check(reader.getCanPageDownProperty() && !reader.getCanPageUpProperty(),
                      "the first page can page down, not up");
            const auto first = reader.getEntriesProperty();
            ctx.Check(first[0].getColumnsProperty().GetValueOutcome("Result") == LeaderboardOutcome::Win &&
                          first[1].getColumnsProperty().GetValueOutcome("Result") == LeaderboardOutcome::Tie,
                      "the first page reads back Win and Tie outcomes");
            const PropertyDictionary& firstColumns = first[0].getColumnsProperty();
            ctx.Check(firstColumns.GetValueInt32("Round") == 1 &&
                          firstColumns.GetValueInt64("ScoreCopy") == 4200 &&
                          firstColumns.GetValueSingle("Accuracy") == 0.75f &&
                          firstColumns.GetValueDouble("Multiplier") == 1.5 &&
                          firstColumns.GetValueDateTime("Recorded") == System::DateTime(2026, 10, 14) &&
                          firstColumns.GetValueString("Note") == "round 1",
                      "all six additional typed columns survive the offline store");

            std::any foundRound;
            std::any missingSentinel = std::string("unchanged");
            int rangedColumns = 0;
            for (const auto& [key, value] : firstColumns)
            {
                (void)key;
                (void)value;
                ++rangedColumns;
            }
            ctx.Check(firstColumns.ContainsKey("Round") && !firstColumns.ContainsKey("Missing") &&
                          firstColumns.TryGetValue("Round", foundRound) && std::any_cast<int>(foundRound) == 1 &&
                          !firstColumns.TryGetValue("Missing", missingSentinel) &&
                          std::any_cast<std::string>(missingSentinel) == "unchanged" &&
                          std::any_cast<int>(firstColumns["Round"]) == 1 &&
                          rangedColumns == 8 + (firstColumns.ContainsKey("Replay") ? 1 : 0),
                      "read dictionaries preserve lookup outputs, Item access and C++ range shape");

            bool replayPersisted = false;
            if (firstColumns.ContainsKey("Replay"))
            {
                System::IO::Stream* replay = firstColumns.GetValueStream("Replay");
                std::array<SharpRuntime::bytecs, 3> bytes{};
                replayPersisted = replay != nullptr && replay->Read(bytes.data(), 0, static_cast<int>(bytes.size())) == 3 &&
                    bytes == std::array<SharpRuntime::bytecs, 3>{1, 0x43, 0x4e};
            }
            ctx.Check(writerStreamsExact_ && replayPersisted,
                      "an offline writer creates, reads and persists a stream column (CNA-DEMOS-ISSUE-085)");

            PropertyDictionary scratch = firstColumns;
            const std::vector<std::string> keys = scratch.Keys();
            const std::vector<std::any> values = scratch.Values();
            ctx.Check(keys.size() == 9 && values.size() == 9 && keys.front() == "Accuracy" && keys.back() == "Time" &&
                          keys[4] == "Replay" && std::any_cast<System::IO::Stream*>(values[4]) ==
                              firstColumns.GetValueStream("Replay") &&
                          std::any_cast<float>(values.front()) == 0.75f &&
                          std::any_cast<System::TimeSpan>(values.back()) == System::TimeSpan::FromSeconds(112),
                      "Keys and Values are matching sorted snapshots of all nine persisted columns, including Replay");

            const std::pair<std::string, std::any> sentinel{"sentinel", std::any(-1)};
            std::vector<std::pair<std::string, std::any>> shortCopy(9, sentinel);
            bool shortCopyRefused = false;
            try
            {
                scratch.CopyTo(shortCopy, 1);
            }
            catch (const System::ArgumentException&)
            {
                shortCopyRefused = true;
            }
            bool shortCopyUntouched = true;
            for (const auto& pair : shortCopy)
            {
                shortCopyUntouched = shortCopyUntouched && pair.first == "sentinel" &&
                    std::any_cast<int>(pair.second) == -1;
            }
            std::vector<std::pair<std::string, std::any>> copied(11, sentinel);
            scratch.CopyTo(copied, 1);
            ctx.Check(shortCopyRefused && shortCopyUntouched && copied.front().first == "sentinel" &&
                          copied.back().first == "sentinel" && copied[1].first == "Accuracy" &&
                          copied[5].first == "Replay" && std::any_cast<System::IO::Stream*>(copied[5].second) ==
                              firstColumns.GetValueStream("Replay") && copied[9].first == "Time",
                      "CopyTo refuses a short offset atomically and preserves outer sentinels on success");

            using PropertyPair = System::Collections::Generic::KeyValuePair<std::string, std::any>;
            const PropertyPair temporary("Temporary", std::any(99));
            scratch.Add(temporary);
            bool duplicateRefused = false;
            try
            {
                scratch.Add("Temporary", std::any(100));
            }
            catch (const System::ArgumentException&)
            {
                duplicateRefused = true;
            }
            const bool pairMatched = scratch.Contains(temporary) &&
                !scratch.Contains(PropertyPair("Temporary", std::any(98)));
            const bool wrongPairPreserved = !scratch.Remove(PropertyPair("Temporary", std::any(98))) &&
                scratch.ContainsKey("Temporary");
            const bool matchingPairRemoved = scratch.Remove(temporary) && !scratch.ContainsKey("Temporary");
            scratch.Add("ByKey", std::any(7));
            const bool keyRemoved = scratch.Remove("ByKey") && !scratch.Remove("ByKey");
            scratch.Add("ClearMe", std::any(1));
            scratch.Clear();
            ctx.Check(scratch.getIsReadOnlyProperty() && duplicateRefused && pairMatched && wrongPairPreserved &&
                          matchingPairRemoved && keyRemoved && scratch.getCountProperty() == 0,
                      "the read-only flag coexists with exact Add, Contains, Remove and Clear collection behavior");

            // BeginPageDown/EndPageDown move the same reader to the next rows.
            int downCallbacks = 0;
            bool downStateMatched = false;
            std::unique_ptr<System::IAsyncResult> downResult(reader.BeginPageDown(
                [&downCallbacks, &downStateMatched](System::IAsyncResult& result) {
                    ++downCallbacks;
                    downStateMatched = std::any_cast<int>(result.getAsyncStateProperty()) == 2;
                },
                2));
            ctx.Check(downCallbacks == 1 && downStateMatched && downResult->getIsCompletedProperty() &&
                          downResult->getCompletedSynchronouslyProperty(),
                      "BeginPageDown completes offline with one callback and its caller state");
            reader.EndPageDown(downResult.get());
            ListPage(reader);
            ctx.Check(reader.getPageStartProperty() == 2 && !reader.getCanPageDownProperty() &&
                          reader.getCanPageUpProperty(),
                      "PageDown shows rows 3-4, the last page");
            const auto page = reader.getEntriesProperty();
            const PropertyDictionary& third = page[0].getColumnsProperty();
            ctx.Check(page[0].getGamerProperty()->getGamertagProperty() == "Third Player" &&
                          third.GetValueOutcome("Result") == LeaderboardOutcome::Loss &&
                          third.GetValueTimeSpan("Time") == System::TimeSpan::FromSeconds(141) &&
                          page[1].getColumnsProperty().GetValueOutcome("Result") == LeaderboardOutcome::None,
                      "the last page reads back Loss and the explicit None outcome");

            int upCallbacks = 0;
            bool upStateMatched = false;
            std::unique_ptr<System::IAsyncResult> upResult(reader.BeginPageUp(
                [&upCallbacks, &upStateMatched](System::IAsyncResult& result) {
                    ++upCallbacks;
                    upStateMatched = std::any_cast<std::string>(result.getAsyncStateProperty()) == "first page";
                },
                std::string("first page")));
            ctx.Check(upCallbacks == 1 && upStateMatched && upResult->getIsCompletedProperty() &&
                          upResult->getCompletedSynchronouslyProperty(),
                      "BeginPageUp completes offline with one callback and its caller state");
            reader.EndPageUp(upResult.get());
            const auto again = reader.getEntriesProperty();
            ctx.Check(reader.getPageStartProperty() == 0 &&
                          again[0].getGamerProperty()->getGamertagProperty() == "Demo Player",
                      "PageUp returns to the first page");

            reader.PageDown();
            const bool synchronousDown = reader.getPageStartProperty() == 2;
            reader.PageUp();
            ctx.Check(synchronousDown && reader.getPageStartProperty() == 0,
                      "the synchronous PageDown and PageUp pair traverses the same two pages");

            reader.Dispose();
            reader.Dispose();
            ctx.Check(reader.getIsDisposedProperty(), "the reader reports disposed after idempotent cleanup");
        }

        bool done_ = false;
        bool writerStreamsExact_ = false;
        int waited_ = 0;
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(PagingDemo, {
    .id = "gamer_services.leaderboard.paging",
    .title = "Leaderboard pages and columns",
    .description = "Four local players write typed and stream columns to a board, exercise dictionary lookup and "
                   "range behavior, and read it two rows at a time through the offline Begin/End contracts, "
                   "including persisted stream contents.",
    .apis = {"LeaderboardReader::PageDown", "LeaderboardReader::PageUp", "LeaderboardReader::BeginPageDown",
             "LeaderboardReader::EndPageDown", "LeaderboardReader::BeginPageUp", "LeaderboardReader::EndPageUp",
             "LeaderboardReader::BeginRead", "LeaderboardReader::EndRead", "LeaderboardReader::CanPageDown",
             "LeaderboardReader::CanPageUp", "LeaderboardReader::PageStart", "LeaderboardReader::TotalLeaderboardSize",
             "LeaderboardReader::LeaderboardIdentity", "LeaderboardReader::Dispose",
             "LeaderboardReader::IsDisposed", "LeaderboardIdentity::Create", "LeaderboardIdentity::Key",
             "LeaderboardIdentity::GameMode", "LeaderboardKey", "LeaderboardKey::BestScoreLifeTime",
             "LeaderboardKey::BestScoreRecent", "LeaderboardKey::BestTimeLifeTime",
             "LeaderboardKey::BestTimeRecent", "LeaderboardOutcome", "LeaderboardOutcome::None",
             "LeaderboardOutcome::Win", "LeaderboardOutcome::Loss", "LeaderboardOutcome::Tie",
             "PropertyDictionary::Count", "PropertyDictionary::Item",
             "PropertyDictionary::ContainsKey", "PropertyDictionary::TryGetValue",
             "PropertyDictionary::GetValueDateTime", "PropertyDictionary::GetValueDouble",
             "PropertyDictionary::GetValueInt32", "PropertyDictionary::GetValueInt64",
             "PropertyDictionary::GetValueOutcome", "PropertyDictionary::GetValueSingle",
             "PropertyDictionary::GetValueStream", "PropertyDictionary::GetValueString",
             "PropertyDictionary::GetValueTimeSpan", "PropertyDictionary::SetValue",
             "PropertyDictionary::Add", "PropertyDictionary::Clear", "PropertyDictionary::Contains",
             "PropertyDictionary::CopyTo", "PropertyDictionary::IsReadOnly", "PropertyDictionary::Remove",
             "PropertyDictionary::Keys", "PropertyDictionary::Values",
             "LeaderboardEntry::Columns", "LeaderboardEntry::RankingEXT", "LeaderboardReader::Entries"},
    .tags = {"gamer-services", "leaderboard", "high-score", "paging", "offline"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
