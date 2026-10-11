// SPDX-License-Identifier: MS-PL
// GraphicsDeviceManager's device lifetime events: DeviceCreated, DeviceResetting, DeviceReset --
// and the GraphicsDevice's own DeviceResetting/DeviceReset, for code that only holds the device.
#include <algorithm>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
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
            Graphics().getDeviceResettingEvent() += [this](System::Object*, const System::EventArgs&) {
                ++resetting_;
                order_.push_back("manager.DeviceResetting");
            };
            Graphics().getDeviceResetEvent() += [this](System::Object*, const System::EventArgs&) {
                ++reset_;
                order_.push_back("manager.DeviceReset");
            };
        }

    protected:
        void LoadContent() override
        {
            Context().Check(created_ == 1, "DeviceCreated fired once, before LoadContent");
            // A component that was handed only the GraphicsDevice listens on the device itself.
            Graphics::GraphicsDevice& device = getGraphicsDeviceProperty();
            device.DeviceResetting += [this](System::Object*, const System::EventArgs&) {
                order_.push_back("device.DeviceResetting");
            };
            device.DeviceReset += [this](System::Object*, const System::EventArgs&) {
                order_.push_back("device.DeviceReset");
            };
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
            std::string order;
            for (const std::string& name : order_)
            {
                order += " " + name;
            }
            Context().Print("order:{}", order);
            Context().Check(resetting_ >= 1 && reset_ >= 1, "ApplyChanges with new settings reset the device");
            Context().Check(resetting_ == reset_, "every DeviceResetting is followed by DeviceReset");
            const auto resetting = std::find(order_.begin(), order_.end(), "device.DeviceResetting");
            const auto reset = std::find(order_.begin(), order_.end(), "device.DeviceReset");
            Context().Check(resetting != order_.end() && reset != order_.end() && resetting < reset,
                            "the device raises its own DeviceResetting, then DeviceReset");
            DemoGame::EndRun();
        }

    private:
        int created_ = 0;
        int resetting_ = 0;
        int reset_ = 0;
        int updates_ = 0;
        std::vector<std::string> order_;
    };
}

CNA_DEMOS_GAME(DeviceEventsDemo, {
    .id = "game.graphics_device_manager.device_events",
    .title = "Device lifetime events",
    .description = "Counts GraphicsDeviceManager's DeviceCreated, DeviceResetting and DeviceReset events while a "
                   "settings change is applied with ApplyChanges, and the order in which the manager and the "
                   "GraphicsDevice itself raise DeviceResetting and DeviceReset.",
    .apis = {"GraphicsDeviceManager::DeviceCreated", "GraphicsDeviceManager::DeviceResetting",
             "GraphicsDeviceManager::DeviceReset", "GraphicsDeviceManager::ApplyChanges",
             "GraphicsDevice::DeviceResetting", "GraphicsDevice::DeviceReset",
             "GraphicsDeviceManager::PreferredBackBufferWidth", "GraphicsDeviceManager::PreferredBackBufferHeight"},
    .tags = {"device", "event", "reset"},
    .frames = 5,
})
