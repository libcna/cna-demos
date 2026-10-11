// SPDX-License-Identifier: MS-PL
// Guide::BeginShowKeyboardInput / EndShowKeyboardInput: asking the player for a line of text.
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Guide.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/TextInputEXT.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IAsyncResult.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Xna::Framework::Input::TextInputEXT;

namespace
{
    class KeyboardInputDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override
        {
            if (Guide::getHasPendingKeyboardInputEXTProperty())
            {
                Guide::ResetPendingKeyboardInputForTestingEXT();
            }
            delete pending_;
            pending_ = nullptr;
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            auto& ctx = Context();
            ++updates_;

            // 1. A name, pre-filled with a default the player may keep (Enter) or extend.
            if (updates_ == 3)
            {
                pending_ = Guide::BeginShowKeyboardInput(PlayerIndex::One, "Name your hero",
                                                         "Up to 15 letters. Enter confirms, Escape cancels.",
                                                         "Hero", nullptr, std::any{});
                ctx.Check(Guide::getIsVisibleProperty() && Guide::getHasPendingKeyboardInputEXTProperty(),
                          "the Guide shows the keyboard input screen");
            }
            // 2. A secret in password mode: the screen shows one '*' per character.
            if (stage_ == 1 && pending_ == nullptr)
            {
                pending_ = Guide::BeginShowKeyboardInput(PlayerIndex::One, "Door code", "Four digits.", "", nullptr,
                                                         std::any{}, true);
                stage_ = 2;
                typed_ = 0;
            }

            if (ctx.Automated())
            {
                TypeUnattended();
            }

            if (pending_ != nullptr && pending_->getIsCompletedProperty())
            {
                const bool cancelled = Guide::WasKeyboardInputCanceledEXT(pending_);
                const std::string text = Guide::EndShowKeyboardInput(pending_);
                delete pending_;
                pending_ = nullptr;
                ctx.Print("request {}: {}", stage_ == 0 ? 1 : 2, cancelled ? "cancelled" : "\"" + text + "\"");
                if (stage_ == 0)
                {
                    name_ = cancelled ? "(cancelled)" : text;
                    stage_ = 1;
                    if (ctx.Automated())
                    {
                        ctx.Check(!cancelled && text == "Heroine",
                                  "EndShowKeyboardInput returned the default text plus what was typed");
                    }
                }
                else
                {
                    code_ = cancelled ? "(cancelled)" : text;
                    stage_ = 3;
                    if (ctx.Automated())
                    {
                        // Real XNA returns null for a cancelled request; CNA returns "" and says
                        // which it was through WasKeyboardInputCanceledEXT.
                        ctx.Check(cancelled && text.empty(),
                                  "a cancelled request returns empty text and WasKeyboardInputCanceledEXT is true");
                    }
                }
            }
        }

        // Unattended runs type through TextInputEXT, the event a real keystroke raises (CNA's own
        // examples do the same), one character every few frames so the screen shows it happen.
        void TypeUnattended()
        {
            auto& ctx = Context();
            if (pending_ == nullptr || updates_ % 3 != 0)
            {
                return;
            }
            const std::u16string keys = stage_ == 0 ? u"ine\r" : u"1234";
            if (typed_ < keys.size())
            {
                TextInputEXT::INTERNAL_OnTextInput(keys[typed_++]);
            }
            else if (stage_ == 2)
            {
                ctx.Check(Guide::GetPendingKeyboardInputDisplayTextForTestingEXT() == "****",
                          "password mode shows the code as ****");
                Guide::SimulateKeyboardInputCancelEXT(); // as if Escape was pressed
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(36, 48, 40));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Guide::BeginShowKeyboardInput", Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Name: " + (name_.empty() ? std::string("...") : name_),
                                     Vector2(40.0f, 70.0f), Color::LightGreen);
            spriteBatch_->DrawString(*font_, "Door code: " + (code_.empty() ? std::string("...") : code_),
                                     Vector2(40.0f, 100.0f), Color::LightGreen);
            spriteBatch_->End();
            DemoGame::Draw(gameTime); // the Guide overlay draws the input screen over this
        }

    private:
        int updates_ = 0;
        int stage_ = 0; // 0 name pending, 1 name answered, 2 code pending, 3 done
        std::size_t typed_ = 0;
        System::IAsyncResult* pending_ = nullptr;
        std::string name_;
        std::string code_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(KeyboardInputDemo, {
    .id = "gamer_services.guide.keyboard_input",
    .title = "Guide keyboard input",
    .description = "Asks for a hero name with Guide::BeginShowKeyboardInput (pre-filled with a default) and then for "
                   "a code in password mode; EndShowKeyboardInput returns the text, and WasKeyboardInputCanceledEXT "
                   "tells a cancelled request apart. Unattended runs type through TextInputEXT and cancel the second.",
    .apis = {"Guide::BeginShowKeyboardInput", "Guide::EndShowKeyboardInput", "Guide::WasKeyboardInputCanceledEXT",
             "Guide::HasPendingKeyboardInputEXT", "Guide::SimulateKeyboardInputCancelEXT",
             "Guide::GetPendingKeyboardInputDisplayTextForTestingEXT", "Guide::ResetPendingKeyboardInputForTestingEXT",
             "TextInputEXT::INTERNAL_OnTextInput", "IAsyncResult::IsCompleted"},
    .tags = {"gamer-services", "guide", "keyboard", "text", "password", "async", "ui"},
    .frames = 60,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
