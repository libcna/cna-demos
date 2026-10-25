// SPDX-License-Identifier: MS-PL
// Native modal message box, triggered only by an explicit key press.
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CNA/Devices/MessageBox.hpp"
#include "CNA/Devices/MessageBoxType.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    class MessageBoxDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            supported_ = CNA::Devices::MessageBox::getIsSupportedProperty();
        }

        void Update(GameTime& gameTime) override
        {
            const bool mDown = Keyboard::GetState().IsKeyDown(Keys::M);
            if (mDown && !mWasDown_)
            {
                if (!supported_)
                    result_ = "Native message boxes are unsupported on this host.";
                else
                {
                    try
                    {
                        const int choice = CNA::Devices::MessageBox::Show(
                            CNA::Devices::MessageBoxType::Information, "CNA Demos",
                            "This native dialog blocks the game thread until you answer.", {"Continue", "Cancel"});
                        result_ = "Dialog returned button index " + std::to_string(choice) + ".";
                    }
                    catch (const std::exception& exception)
                    {
                        result_ = std::string("Dialog failed: ") + exception.what();
                    }
                }
            }
            mWasDown_ = mDown;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Press M to show a native modal MessageBox.", Vector2(32.0f, 40.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "The game thread pauses until the dialog closes.",
                                     Vector2(32.0f, 80.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, std::string("Supported: ") + (supported_ ? "yes" : "no"),
                                     Vector2(32.0f, 140.0f), Color::Gold);
            spriteBatch_->DrawString(*font_, result_, Vector2(32.0f, 180.0f), Color::LightGreen);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        bool supported_ = false;
        bool mWasDown_ = false;
        std::string result_ = "No dialog has been requested.";
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(MessageBoxDemo, {
    .id = "devices.desktop.message_box",
    .title = "Native message box",
    .description = "Press M to show a real native two-button MessageBox, observe its synchronous blocking behavior "
                   "and record the selected button index or a platform failure.",
    .apis = {"CNA::Devices::MessageBox::IsSupported", "CNA::Devices::MessageBox::Show",
             "CNA::Devices::MessageBoxType"},
    .tags = {"devices", "desktop", "dialog", "message-box", "manual", "cnaext"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual: opens a modal OS window only after M is pressed; never launched by unattended runs.",
})
