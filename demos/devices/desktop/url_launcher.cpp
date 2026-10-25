// SPDX-License-Identifier: MS-PL
// External URL launch, guarded by an explicit key press.
#include <exception>
#include <memory>
#include <optional>
#include <string>

#include "CNA/Devices/UrlLauncher.hpp"
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
    class UrlLauncherDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            uWasDown_ = Keyboard::GetState().IsKeyDown(Keys::U);
        }

        void Update(GameTime& gameTime) override
        {
            const bool uDown = Keyboard::GetState().IsKeyDown(Keys::U);
            if (!Context().Automated() && uDown && !uWasDown_)
            {
                try
                {
                    opened_ = CNA::Devices::UrlLauncher::Open("https://github.com/libcna/cna");
                    status_ = *opened_ ? "The system accepted the URL launch request."
                                      : "No external handler accepted the URL.";
                }
                catch (const std::exception& exception)
                {
                    opened_ = false;
                    status_ = std::string("URL launch failed: ") + exception.what();
                }
            }
            uWasDown_ = uDown;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Press U to open https://github.com/libcna/cna", Vector2(32.0f, 40.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "A true result means a handler launched, not that loading succeeded.",
                                     Vector2(32.0f, 80.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, status_, Vector2(32.0f, 140.0f),
                                     opened_.value_or(false) ? Color::LightGreen : Color::Gold);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        bool uWasDown_ = false;
        std::optional<bool> opened_;
        std::string status_ = "No external application has been launched.";
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(UrlLauncherDemo, {
    .id = "devices.desktop.url_launcher",
    .title = "External URL launcher",
    .description = "Press U to hand an HTTPS URL to the system's real default external handler and report whether "
                   "the launch request was accepted.",
    .apis = {"CNA::Devices::UrlLauncher::Open"},
    .tags = {"devices", "desktop", "url", "browser", "external", "manual", "cnaext"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual: launches an external application only after U is pressed; never launched unattended.",
})
