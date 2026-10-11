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

            for (const Achievement& achievement : after)
            {
                std::string line = achievement.getIsEarnedProperty() ? "[x] " : "[ ] ";
                if (achievement.getIsEarnedProperty() || achievement.getDisplayBeforeEarnedProperty())
                {
                    line += achievement.getNameProperty() + " (" + std::to_string(achievement.getGamerScoreProperty())
                        + ") - " + achievement.getDescriptionProperty();
                }
                else
                {
                    line += "Secret achievement - " + achievement.getHowToEarnProperty();
                }
                ctx.Print("{}", line);
                lines_.push_back(line);
            }
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
                                         line.starts_with("[x]") ? Color::Gold : Color::Gray);
                y += 32.0f;
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
                   "with AwardAchievement (the Guide shows a toast), and shows that earned achievements keep their "
                   "first date, unknown keys are refused and a hidden one stays secret until earned.",
    .apis = {"SignedInGamer::AwardAchievement", "SignedInGamer::GetAchievements", "Achievement::Key",
             "Achievement::Name", "Achievement::Description", "Achievement::HowToEarn", "Achievement::GamerScore",
             "Achievement::IsEarned", "Achievement::EarnedDateTime", "Achievement::EarnedOnline",
             "Achievement::DisplayBeforeEarned", "AchievementCollection::Count", "GamerServicesNotAvailableException"},
    .tags = {"gamer-services", "achievements", "profile", "offline", "catalog"},
    .frames = 60,
    .assets = {"fonts/pixel_font.cnb"},
    .knownIssue = "CNA-DEMOS-ISSUE-017",
    .ownProcess = true,
})
