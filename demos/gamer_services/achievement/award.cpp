// SPDX-License-Identifier: MS-PL
// SignedInGamer::AwardAchievement and GetAchievements, offline, from the title's catalog.
#include <any>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Achievement.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AchievementCollection.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerServicesNotAvailableException.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IAsyncResult.hpp"
#include "System/IndexOutOfRangeException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Achievement* Find(const AchievementCollection& achievements, const std::string& key)
    {
        for (const Achievement& achievement : achievements)
        {
            if (achievement.getKeyProperty() == key)
            {
                return &achievement;
            }
        }
        return nullptr;
    }

    class AwardDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            SignedInGamer* player = (*Gamer::getSignedInGamersProperty())[PlayerIndex::One];
            if (done_ || player == nullptr)
            {
                return;
            }
            done_ = true;
            auto& ctx = Context();

            // Offline, the achievements are those of the title's catalog,
            // GamerServices/Achievements.json next to the executable; what the player earned is
            // kept on this machine, so a second run finds them earned already.
            AchievementCollection before = player->GetAchievements();
            ctx.Check(before.getCountProperty() == 3, "GetAchievements lists the catalog's three achievements");
            const bool hadFirstSteps = Find(before, "first_steps")->getIsEarnedProperty();
            ctx.Print("first_steps earned before this run: {}", hadFirstSteps);

            player->AwardAchievement("first_steps");
            int awardCallbacks = 0;
            bool awardStateMatched = false;
            std::unique_ptr<System::IAsyncResult> awardRequest(player->BeginAwardAchievement(
                "explorer",
                [&awardCallbacks, &awardStateMatched](System::IAsyncResult& result) {
                    ++awardCallbacks;
                    awardStateMatched = std::any_cast<std::string>(result.getAsyncStateProperty()) == "explorer";
                },
                std::string("explorer")));
            ctx.Check(awardCallbacks == 1 && awardStateMatched && awardRequest->getIsCompletedProperty() &&
                          awardRequest->getCompletedSynchronouslyProperty(),
                      "BeginAwardAchievement completes offline with one callback and exact caller state");
            player->EndAwardAchievement(awardRequest.get());

            // A key the catalog does not define is refused, as a CNA service would.
            bool refused = false;
            try
            {
                player->AwardAchievement("not_in_the_catalog");
            }
            catch (const GamerServicesNotAvailableException&)
            {
                refused = true;
            }
            ctx.Check(refused, "an unknown key throws GamerServicesNotAvailableException");

            int readCallbacks = 0;
            bool readStateMatched = false;
            std::unique_ptr<System::IAsyncResult> readRequest(player->BeginGetAchievements(
                [&readCallbacks, &readStateMatched](System::IAsyncResult& result) {
                    ++readCallbacks;
                    readStateMatched = std::any_cast<int>(result.getAsyncStateProperty()) == 3;
                },
                3));
            ctx.Check(readCallbacks == 1 && readStateMatched && readRequest->getIsCompletedProperty() &&
                          readRequest->getCompletedSynchronouslyProperty(),
                      "BeginGetAchievements completes offline with one callback and exact caller state");
            AchievementCollection after = player->EndGetAchievements(readRequest.get());
            const Achievement* firstSteps = Find(after, "first_steps");
            const Achievement* hidden = Find(after, "hidden_door");
            ctx.Check(firstSteps->getIsEarnedProperty() && Find(after, "explorer")->getIsEarnedProperty(),
                      "the awarded achievements are earned");
            ctx.Check(!firstSteps->getEarnedOnlineProperty(), "earned offline (EarnedOnline is false)");
            const auto earnedAt = firstSteps->getEarnedDateTimeProperty().getTicksProperty();
            player->AwardAchievement("first_steps");
            ctx.Check(Find(player->GetAchievements(), "first_steps")->getEarnedDateTimeProperty().getTicksProperty() ==
                          earnedAt,
                      "awarding an earned achievement again keeps its first date");
            ctx.Check(!hidden->getIsEarnedProperty() && !hidden->getDisplayBeforeEarnedProperty(),
                      "hidden_door stays unearned, and its catalog entry hides it until earned");

            Achievement pictureAchievement = *firstSteps;
            std::unique_ptr<System::IO::Stream> picture(pictureAchievement.GetPicture());
            const std::array<SharpRuntime::bytecs, 8> pngSignature{0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
            std::array<SharpRuntime::bytecs, pngSignature.size()> header{};
            const bool startsAtZero = picture->getPositionProperty() == 0;
            const bool exactLength = picture->getLengthProperty() == 70;
            const bool readHeader = picture->Read(header.data(), 0, static_cast<int>(header.size())) ==
                static_cast<int>(header.size());
            std::unique_ptr<System::IO::Stream> secondPicture(pictureAchievement.GetPicture());
            ctx.Check(startsAtZero && exactLength && readHeader && header == pngSignature &&
                          picture.get() != secondPicture.get() && secondPicture->getPositionProperty() == 0 &&
                          secondPicture->getLengthProperty() == picture->getLengthProperty(),
                      "GetPicture returns independent caller-owned PNG streams at position zero");

            bool missingPictureRefused = false;
            try
            {
                Achievement missingPictureAchievement = *hidden;
                delete missingPictureAchievement.GetPicture();
            }
            catch (const GamerServicesNotAvailableException&)
            {
                missingPictureRefused = true;
            }
            ctx.Check(missingPictureRefused,
                      "GetPicture throws GamerServicesNotAvailableException when the catalog has no picture");

            bool integerBoundsRefused = false;
            bool keyBoundsRefused = false;
            try
            {
                (void)after[after.getCountProperty()];
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                integerBoundsRefused = true;
            }
            try
            {
                (void)after["missing"];
            }
            catch (const System::IndexOutOfRangeException&)
            {
                keyBoundsRefused = true;
            }
            ctx.Check(after[0].getKeyProperty() == "first_steps" &&
                          after["explorer"].getKeyProperty() == "explorer" &&
                          integerBoundsRefused && keyBoundsRefused,
                      "integer and key Item lookups preserve catalog order and exact boundary exceptions");

            const Achievement first = after[0];
            const Achievement explorer = after[1];
            const Achievement hiddenCopy = after[2];
            std::vector<Achievement> shortCopy(4, explorer);
            bool shortCopyRefused = false;
            try
            {
                after.CopyTo(shortCopy, 2);
            }
            catch (const System::ArgumentException&)
            {
                shortCopyRefused = true;
            }
            bool shortCopyUntouched = true;
            for (const Achievement& achievement : shortCopy)
            {
                shortCopyUntouched = shortCopyUntouched && achievement.getKeyProperty() == "explorer";
            }
            std::vector<Achievement> copied(5, explorer);
            after.CopyTo(copied, 1);
            ctx.Check(shortCopyRefused && shortCopyUntouched && copied.front().getKeyProperty() == "explorer" &&
                          copied.back().getKeyProperty() == "explorer" &&
                          copied[1].getKeyProperty() == "first_steps" &&
                          copied[3].getKeyProperty() == "hidden_door" && after.Contains(explorer) &&
                          after.IndexOf(hiddenCopy) == 2,
                      "CopyTo is atomic when short and exact at an offset; Contains and IndexOf use values");

            AchievementCollection scratch = after;
            scratch.Insert(1, hiddenCopy);
            const bool inserted = scratch.getCountProperty() == 4 &&
                scratch[1].getKeyProperty() == "hidden_door" && scratch[2].getKeyProperty() == "explorer";
            scratch.RemoveAt(1);
            scratch.Add(first);
            const bool appended = scratch.getCountProperty() == 4 && scratch[3].getKeyProperty() == "first_steps";
            const bool removed = scratch.Remove(first) && scratch.getCountProperty() == 3 &&
                scratch[2].getKeyProperty() == "first_steps";
            scratch.Clear();
            ctx.Check(scratch.getIsReadOnlyProperty() && inserted && appended && removed &&
                          scratch.getCountProperty() == 0 && !scratch.getIsDisposedProperty(),
                      "the read-only flag coexists with exact Insert, RemoveAt, Add, Remove and Clear behavior");

            for (const Achievement& achievement : after)
            {
                const bool shown = achievement.getIsEarnedProperty() || achievement.getDisplayBeforeEarnedProperty();
                std::string line = achievement.getIsEarnedProperty() ? "[x] " : "[ ] ";
                line += shown ? achievement.getNameProperty() + " (" + std::to_string(achievement.getGamerScoreProperty()) + ")"
                              : std::string("Secret achievement");
                const std::string detail = shown ? achievement.getDescriptionProperty() : achievement.getHowToEarnProperty();
                ctx.Print("{} - {}", line, detail);
                lines_.push_back(line);
                lines_.push_back("    " + detail);
            }

            after.Dispose();
            after.Dispose();
            ctx.Check(after.getIsDisposedProperty() && after.getCountProperty() == 0 &&
                          !before.getIsDisposedProperty() && before.getCountProperty() == 3,
                      "collection disposal is idempotent, clears its copy, and leaves another snapshot live");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(28, 28, 40));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "SignedInGamer::GetAchievements", Vector2(30.0f, 30.0f), Color::White);
            float y = 80.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(30.0f, y),
                                         line.starts_with("[x]") ? Color::Gold
                                         : line.starts_with("[ ]") ? Color::LightGray
                                                                    : Color::Gray);
                y += line.starts_with("    ") ? 34.0f : 22.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime); // the Guide overlay shows "Achievement unlocked" toasts
        }

    private:
        bool done_ = false;
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(AwardDemo, {
    .id = "gamer_services.achievement.award",
    .title = "Achievements",
    .description = "Lists the title's offline achievement catalog with SignedInGamer::GetAchievements, reads one "
                   "catalog picture, awards two with AwardAchievement, and exercises populated collection behavior "
                   "while earned achievements keep their first date and a hidden one stays secret until earned.",
    .apis = {"SignedInGamer::AwardAchievement", "SignedInGamer::BeginAwardAchievement",
             "SignedInGamer::EndAwardAchievement", "SignedInGamer::GetAchievements",
             "SignedInGamer::BeginGetAchievements", "SignedInGamer::EndGetAchievements", "Achievement::Key",
             "Achievement::Name", "Achievement::Description", "Achievement::HowToEarn", "Achievement::GamerScore",
             "Achievement::IsEarned", "Achievement::EarnedDateTime", "Achievement::EarnedOnline",
             "Achievement::DisplayBeforeEarned", "Achievement::GetPicture", "AchievementCollection::Count",
             "AchievementCollection::Item",
             "AchievementCollection::Dispose", "AchievementCollection::IsDisposed", "AchievementCollection::IndexOf",
             "AchievementCollection::Insert", "AchievementCollection::RemoveAt", "AchievementCollection::Add",
             "AchievementCollection::Remove", "AchievementCollection::Clear", "AchievementCollection::Contains",
             "AchievementCollection::CopyTo", "AchievementCollection::IsReadOnly",
             "GamerServicesNotAvailableException"},
    .tags = {"gamer-services", "achievements", "profile", "offline", "catalog"},
    .frames = 60,
    .assets = {"fonts/pixel_font.cnb"},
    .knownIssue = "CNA-DEMOS-ISSUE-017",
    .ownProcess = true,
})
