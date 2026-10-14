// SPDX-License-Identifier: MS-PL
// SignedInGamer::AwardAchievement and GetAchievements, offline, from the title's catalog.
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
            player->AwardAchievement("explorer");

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

            AchievementCollection after = player->GetAchievements();
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
    .description = "Lists the title's offline achievement catalog with SignedInGamer::GetAchievements, awards two "
                   "with AwardAchievement, and exercises populated item, copy, mutation and disposal behavior while "
                   "earned achievements keep their first date and a hidden one stays secret until earned.",
    .apis = {"SignedInGamer::AwardAchievement", "SignedInGamer::GetAchievements", "Achievement::Key",
             "Achievement::Name", "Achievement::Description", "Achievement::HowToEarn", "Achievement::GamerScore",
             "Achievement::IsEarned", "Achievement::EarnedDateTime", "Achievement::EarnedOnline",
             "Achievement::DisplayBeforeEarned", "AchievementCollection::Count", "AchievementCollection::Item",
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
