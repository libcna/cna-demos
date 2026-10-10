// SPDX-License-Identifier: MS-PL
// GraphicsDeviceManager's device lifetime events: DeviceCreated, DeviceResetting, DeviceReset.
#include "CnaDemos/Demo.hpp"
#include "System/EventArgs.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class DeviceEventsDemo final : public CnaDemos::DemoGame
    {
    public:
        DeviceEventsDemo()
        {
            Graphics().getDeviceCreatedEvent() += [this](System::Object*, const System::EventArgs&) { ++created_; };
            Graphics().getDeviceResettingEvent() += [this](System::Object*, const System::EventArgs&) { ++resetting_; };
            Graphics().getDeviceResetEvent() += [this](System::Object*, const System::EventArgs&) { ++reset_; };
        }

    protected:
        void LoadContent() override
        {
            Context().Check(created_ == 1, "DeviceCreated fired once, before LoadContent");
        }

        void Update(GameTime& gameTime) override
        {
            // Changing presentation settings resets the device; resources that live on the GPU
            // are recreated around DeviceResetting/DeviceReset in XNA.
            if (++updates_ == 2)
            {
                Graphics().setPreferredBackBufferWidthProperty(720);
                Graphics().setPreferredBackBufferHeightProperty(400);
                Graphics().ApplyChanges();
            }
            Game::Update(gameTime);
        }

        void EndRun() override
        {
            Context().Print("DeviceCreated {}, DeviceResetting {}, DeviceReset {}", created_, resetting_, reset_);
            Context().Check(resetting_ >= 1 && reset_ >= 1, "ApplyChanges with new settings reset the device");
            Context().Check(resetting_ == reset_, "every DeviceResetting is followed by DeviceReset");
        }

    private:
        int created_ = 0;
        int resetting_ = 0;
        int reset_ = 0;
        int updates_ = 0;
    };
}

CNA_DEMOS_GAME(DeviceEventsDemo, {
    .id = "game.graphics_device_manager.device_events",
    .title = "Device lifetime events",
    .description = "Counts GraphicsDeviceManager's DeviceCreated, DeviceResetting and DeviceReset events while a "
                   "settings change is applied with ApplyChanges.",
    .apis = {"GraphicsDeviceManager::DeviceCreated", "GraphicsDeviceManager::DeviceResetting",
             "GraphicsDeviceManager::DeviceReset", "GraphicsDeviceManager::ApplyChanges"},
    .tags = {"device", "event", "reset"},
    .frames = 5,
})
