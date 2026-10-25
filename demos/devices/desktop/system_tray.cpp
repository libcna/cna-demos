// SPDX-License-Identifier: MS-PL
// Native notification-area icon, explicitly created and always destroyed.
#include <atomic>
#include <memory>
#include <optional>
#include <string>

#include "CNA/Devices/SystemTray.hpp"
#include "CNA/Platform/PlatformException.hpp"
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
    class SystemTrayDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            supported_ = CNA::Devices::SystemTray::getIsSupportedProperty();
            pendingClicks_ = std::make_shared<std::atomic<int>>(0);
            tWasDown_ = Keyboard::GetState().IsKeyDown(Keys::T);
        }

        void UnloadContent() override
        {
            DestroyTray();
            pendingClicks_.reset();
        }

        void Update(GameTime& gameTime) override
        {
            const int clicks = pendingClicks_->exchange(0);
            for (int i = 0; i < clicks && tray_; ++i)
            {
                ++clickCount_;
                checked_ = !tray_->GetEntryChecked(entry_);
                tray_->SetEntryChecked(entry_, checked_);
                tray_->SetEntryLabel(entry_, "Enabled (clicked " + std::to_string(clickCount_) + ")");
            }

            const bool tDown = Keyboard::GetState().IsKeyDown(Keys::T);
            if (!Context().Automated() && tDown && !tWasDown_)
            {
                if (tray_)
                    DestroyTray();
                else
                    CreateTray();
            }
            tWasDown_ = tDown;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Press T to create/destroy a real notification-area icon.",
                                     Vector2(24.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Its checkable menu callback is applied on the game thread.",
                                     Vector2(24.0f, 65.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, std::string("Supported: ") + (supported_ ? "yes" : "no") +
                                         "  active icon: " + (tray_ ? "yes" : "no") +
                                         "  clicks: " + std::to_string(clickCount_),
                                     Vector2(24.0f, 120.0f), Color::Gold);
            spriteBatch_->DrawString(*font_, status_, Vector2(24.0f, 158.0f), Color::LightGreen);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        void CreateTray()
        {
            if (!supported_)
            {
                status_ = "This platform has no native tray service.";
                return;
            }
            try
            {
                tray_.emplace("CNA Demos manual tray example");
                const std::weak_ptr<std::atomic<int>> pending = pendingClicks_;
                entry_ = tray_->AddEntry("Enabled", true, true, true, [pending] {
                    if (const auto clicks = pending.lock())
                        clicks->fetch_add(1);
                });
                checked_ = true;
                status_ = "Tray icon created; open its menu and click Enabled.";
            }
            catch (const CNA::Platform::PlatformException& exception)
            {
                tray_.reset();
                status_ = std::string("Tray host unavailable: ") + exception.what();
            }
        }

        void DestroyTray()
        {
            tray_.reset();
            if (pendingClicks_)
                pendingClicks_->store(0);
            status_ = "Tray icon destroyed and callback state released.";
        }

        bool supported_ = false;
        bool tWasDown_ = false;
        bool checked_ = true;
        int clickCount_ = 0;
        std::size_t entry_ = 0;
        std::shared_ptr<std::atomic<int>> pendingClicks_;
        std::string status_ = "No icon exists until T is pressed.";
        std::optional<CNA::Devices::SystemTray> tray_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(SystemTrayDemo, {
    .id = "devices.desktop.system_tray",
    .title = "Native system tray icon",
    .description = "Press T to create or destroy a real notification-area icon with a checkable menu entry; clicks "
                   "are marshalled to Update and the icon is unconditionally destroyed on exit.",
    .apis = {"CNA::Devices::SystemTray::IsSupported", "CNA::Devices::SystemTray::SystemTray",
             "CNA::Devices::SystemTray::AddEntry", "CNA::Devices::SystemTray::SetEntryLabel",
             "CNA::Devices::SystemTray::SetEntryChecked", "CNA::Devices::SystemTray::GetEntryChecked"},
    .tags = {"devices", "desktop", "tray", "notification-area", "menu", "callback", "manual", "cnaext"},
    .automation = CnaDemos::Automation::Manual,
    .needs = CnaDemos::Need::Window | CnaDemos::Need::Keyboard,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Manual desktop workflow; T is required to create the icon and unsupported/missing tray hosts are reported.",
})
