// SPDX-License-Identifier: MS-PL
// SignedInGamer::Privileges: what the player may do (online play, communication, purchases).
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/GamerServicesNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerPrivilegeException.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerPrivileges.hpp"
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
    std::string YesNo(bool value) { return value ? "yes" : "no"; }

    class PrivilegesDemo final : public CnaDemos::GamerServicesDemo
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
            if (!lines_.empty() || player == nullptr)
            {
                return;
            }
            // A game checks these before offering online play, voice or text chat, user-made
            // levels or the store -- parental controls and account policy decide them.
            const GamerPrivileges& privileges = player->getPrivilegesProperty();
            lines_ = {
                "AllowOnlineSessions:     " + YesNo(privileges.getAllowOnlineSessionsProperty()),
                "AllowCommunication:      " + std::string(CnaDemos::Name(privileges.getAllowCommunicationProperty())),
                "AllowProfileViewing:     " + std::string(CnaDemos::Name(privileges.getAllowProfileViewingProperty())),
                "AllowUserCreatedContent: " +
                    std::string(CnaDemos::Name(privileges.getAllowUserCreatedContentProperty())),
                "AllowPurchaseContent:    " + YesNo(privileges.getAllowPurchaseContentProperty()),
                "AllowPremiumContent:     " + YesNo(privileges.getAllowPremiumContentProperty()),
                "AllowTradeContent:       " + YesNo(privileges.getAllowTradeContentProperty()),
            };
            for (const std::string& line : lines_)
            {
                Context().Print("{}", line);
            }
            // A local profile is not signed in to a service: no online sessions, no purchases.
            Context().Check(!privileges.getAllowOnlineSessionsProperty() && !privileges.getAllowPurchaseContentProperty(),
                            "a local profile may not play online sessions or purchase content");
            Context().Check(privileges.getAllowCommunicationProperty() == GamerPrivilegeSetting::Everyone &&
                                privileges.getAllowProfileViewingProperty() == GamerPrivilegeSetting::Everyone &&
                                privileges.getAllowUserCreatedContentProperty() == GamerPrivilegeSetting::Everyone,
                            "a local profile exposes unrestricted offline communication, profile and content policy");
            Context().Check(player->getPartySizeProperty() == 0,
                            "a local profile that has joined no service party reports party size zero");

            bool friendLookupRefused = false;
            bool friendListRefused = false;
            try
            {
                (void) player->IsFriend(player);
            }
            catch (const GamerPrivilegeException&)
            {
                friendLookupRefused = true;
            }
            try
            {
                (void) player->GetFriends();
            }
            catch (const GamerPrivilegeException&)
            {
                friendListRefused = true;
            }
            Context().Check(friendLookupRefused && friendListRefused,
                            "friend lookup and enumeration require an online account");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(48, 36, 30));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "SignedInGamer::Privileges", Vector2(40.0f, 30.0f), Color::White);
            float y = 80.0f;
            for (const std::string& line : lines_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(40.0f, y), Color::NavajoWhite);
                y += 30.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::vector<std::string> lines_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(PrivilegesDemo, {
    .id = "gamer_services.signed_in_gamer.privileges",
    .title = "Gamer privileges",
    .description = "Lists the signed-in gamer's GamerPrivileges -- online sessions, communication, profile viewing, "
                   "user-created content and purchases -- then shows that a local profile has no party and cannot "
                   "query online friends.",
    .apis = {"SignedInGamer::Privileges", "GamerPrivileges::AllowOnlineSessions", "GamerPrivileges::AllowCommunication",
             "GamerPrivileges::AllowProfileViewing", "GamerPrivileges::AllowUserCreatedContent",
             "GamerPrivileges::AllowPurchaseContent", "GamerPrivileges::AllowPremiumContent",
             "GamerPrivileges::AllowTradeContent", "SignedInGamer::PartySize", "SignedInGamer::IsFriend",
             "SignedInGamer::GetFriends", "GamerPrivilegeException", "GamerPrivilegeSetting",
             "GamerPrivilegeSetting::Everyone"},
    .tags = {"gamer-services", "profile", "privileges", "parental-controls", "online"},
    .frames = 20,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
