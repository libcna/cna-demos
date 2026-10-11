// SPDX-License-Identifier: MS-PL
// Gamer::GetProfile and BeginGetProfile: a gamer's public profile, offline.
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/GamerServicesNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Achievement.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AchievementCollection.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerProfile.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IAsyncResult.hpp"
#include "System/IO/Stream.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class GetProfileDemo final : public CnaDemos::GamerServicesDemo
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

            // The asynchronous form, as a game uses it for gamers other than its own players: the
            // callback receives the result; the caller owns the IAsyncResult and the profile.
            std::unique_ptr<GamerProfile> profile;
            std::unique_ptr<System::IAsyncResult> request(player->BeginGetProfile(
                [&profile, player](System::IAsyncResult& result) { profile.reset(player->EndGetProfile(&result)); },
                std::any{}));
            ctx.Check(profile != nullptr, "BeginGetProfile's callback received the profile (offline: at once)");

            // Offline, the totals come from this title's achievements on this machine.
            int score = 0;
            int earned = 0;
            for (const Achievement& achievement : player->GetAchievements())
            {
                if (achievement.getIsEarnedProperty())
                {
                    score += achievement.getGamerScoreProperty();
                    ++earned;
                }
            }
            ctx.Check(profile->getGamerScoreProperty() == score && profile->getTotalAchievementsProperty() == earned,
                      "GamerScore and TotalAchievements add up this title's earned achievements");
            ctx.Check(profile->getTitlesPlayedProperty() == (earned > 0 ? 1 : 0),
                      "TitlesPlayed counts this title once something was earned in it");
            const std::unique_ptr<System::IO::Stream> picture(profile->GetGamerPicture());
            ctx.Check(picture == nullptr, "a local profile has no gamer picture");

            // The synchronous form blocks until the profile is there.
            const std::unique_ptr<GamerProfile> again(player->GetProfile());
            ctx.Check(again->getGamerScoreProperty() == profile->getGamerScoreProperty(), "GetProfile agrees");

            // Looking up an arbitrary gamertag needs a directory to search: offline there is none.
            bool unsupported = false;
            try
            {
                delete Gamer::GetFromGamertag("Somebody");
            }
            catch (const System::NotSupportedException&)
            {
                unsupported = true;
            }
            ctx.Check(unsupported, "Gamer::GetFromGamertag throws NotSupportedException without a service");

            lines_ = {
                "Gamertag:           " + player->getGamertagProperty(),
                "GamerScore:         " + std::to_string(profile->getGamerScoreProperty()),
                "TotalAchievements:  " + std::to_string(profile->getTotalAchievementsProperty()),
                "TitlesPlayed:       " + std::to_string(profile->getTitlesPlayedProperty()),
                "GamerZone:          " + std::string(CnaDemos::Name(profile->getGamerZoneProperty())),
                "Reputation:         " + std::to_string(profile->getReputationProperty()),
                "Motto:              \"" + profile->getMottoProperty() + "\"",
                "Region:             " + profile->getRegionProperty().getNameProperty(),
            };
            for (const std::string& line : lines_)
            {
                ctx.Print("{}", line);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(26, 40, 44));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Gamer::GetProfile", Vector2(40.0f, 30.0f), Color::White);
            float y = 80.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::LightCyan);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        bool done_ = false;
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(GetProfileDemo, {
    .id = "gamer_services.gamer_profile.get_profile",
    .title = "Gamer profile",
    .description = "Reads the signed-in gamer's GamerProfile with BeginGetProfile/EndGetProfile and GetProfile: "
                   "offline, GamerScore and TotalAchievements add up the achievements earned in this title; "
                   "Gamer::GetFromGamertag is not supported without a service.",
    .apis = {"Gamer::GetProfile", "Gamer::BeginGetProfile", "Gamer::EndGetProfile", "GamerProfile::GamerScore",
             "GamerProfile::TotalAchievements", "GamerProfile::TitlesPlayed", "GamerProfile::GamerZone",
             "GamerProfile::Reputation", "GamerProfile::Motto", "GamerProfile::Region",
             "GamerProfile::GetGamerPicture", "Gamer::GetFromGamertag"},
    .tags = {"gamer-services", "profile", "gamerscore", "async", "offline"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
