// SPDX-License-Identifier: MS-PL
// Guide::BeginShowMessageBox / EndShowMessageBox: a system message box, answered asynchronously.
#include <any>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Guide.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GuideAlreadyVisibleException.hpp"
#include "Microsoft/Xna/Framework/GamerServices/MessageBoxIcon.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IAsyncResult.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr const char* Buttons[] = {"Save", "Quit", "Cancel"};

    class MessageBoxDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override
        {
            // Leave no box behind for whatever runs next in this process.
            if (Guide::getHasPendingMessageBoxEXTProperty())
            {
                Guide::ResetPendingMessageBoxForTestingEXT();
            }
            delete pending_;
            pending_ = nullptr;
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            auto& ctx = Context();
            ++updates_;

            if (updates_ == 3)
            {
                // The answer arrives through the callback; the game keeps running meanwhile.
                // The caller owns the returned IAsyncResult and deletes it after EndShowMessageBox.
                pending_ = Guide::BeginShowMessageBox(
                    PlayerIndex::One, "Leave the level?", "Progress since the last checkpoint is lost.",
                    {Buttons[0], Buttons[1], Buttons[2]}, 0, MessageBoxIcon::Warning,
                    [this](System::IAsyncResult& result) {
                        choice_ = Guide::EndShowMessageBox(&result);
                        state_ = std::any_cast<std::string>(result.getAsyncStateProperty());
                        answered_ = true;
                    },
                    std::string("leave-level"));
                ctx.Check(Guide::getIsVisibleProperty(), "the Guide is visible while the box waits for an answer");

                // One Guide screen at a time.
                bool refused = false;
                try
                {
                    delete Guide::BeginShowMessageBox("Second box", "Not shown.", {"OK"}, 0, MessageBoxIcon::None,
                                                      nullptr, std::any{});
                }
                catch (const GuideAlreadyVisibleException&)
                {
                    refused = true;
                }
                ctx.Check(refused, "a second message box is refused with GuideAlreadyVisibleException");
            }

            // Unattended, "click" Quit; interactively a person answers with the mouse, the
            // keyboard (arrows, Enter, Escape) or a controller (D-pad, A, B).
            if (ctx.Automated() && updates_ == 30 && Guide::getHasPendingMessageBoxEXTProperty())
            {
                Guide::SimulateMessageBoxClickEXT(1);
            }

            if (answered_ && pending_ != nullptr)
            {
                delete pending_;
                pending_ = nullptr;
                ctx.Print("answer: {}", choice_ ? Buttons[*choice_] : "cancelled (no button)");
                ctx.Check(!Guide::getIsVisibleProperty(), "the Guide is hidden again once answered");
                ctx.Check(state_ == "leave-level", "the callback received the state object passed to Begin");
                if (ctx.Automated())
                {
                    ctx.Check(choice_ == 1, "EndShowMessageBox returned the index of the button chosen (Quit)");
                }
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(40, 44, 60));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Guide::BeginShowMessageBox", Vector2(40.0f, 30.0f), Color::White);
            std::string status = "Guide visible: ";
            status += Guide::getIsVisibleProperty() ? "yes -- the game keeps drawing underneath" : "no";
            spriteBatch_->DrawString(*font_, status, Vector2(40.0f, 70.0f), Color::LightGray);
            if (answered_)
            {
                const std::string answer = std::string("Answer: ") + (choice_ ? Buttons[*choice_] : "cancelled");
                spriteBatch_->DrawString(*font_, answer, Vector2(40.0f, 110.0f), Color::LightGreen);
            }
            spriteBatch_->End();
            // The GamerServicesComponent's Guide overlay draws the box over this, after Draw.
            DemoGame::Draw(gameTime);
        }

    private:
        int updates_ = 0;
        System::IAsyncResult* pending_ = nullptr;
        bool answered_ = false;
        std::optional<int> choice_;
        std::string state_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(MessageBoxDemo, {
    .id = "gamer_services.guide.message_box",
    .title = "Guide message box",
    .description = "Shows a three-button warning box with Guide::BeginShowMessageBox while the game keeps running; "
                   "the callback calls EndShowMessageBox for the chosen button (unattended runs click Quit with "
                   "SimulateMessageBoxClickEXT), and a second box is refused while one is up.",
    .apis = {"Guide::BeginShowMessageBox", "Guide::EndShowMessageBox", "Guide::IsVisible", "MessageBoxIcon",
             "GuideAlreadyVisibleException", "Guide::HasPendingMessageBoxEXT", "Guide::SimulateMessageBoxClickEXT",
             "Guide::ResetPendingMessageBoxForTestingEXT", "IAsyncResult::AsyncState"},
    .tags = {"gamer-services", "guide", "dialog", "message-box", "async", "ui"},
    .frames = 45,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
