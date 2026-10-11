// SPDX-License-Identifier: MS-PL
// IGraphicsDeviceService: how a component that only knows Game.Services reaches the GraphicsDevice.
#include <cmath>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"
#include "Microsoft/Xna/Framework/GameServiceContainer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IGraphicsDeviceService.hpp"
#include "Microsoft/Xna/Framework/IGraphicsDeviceManager.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A component written without access to the game's GraphicsDeviceManager: it asks the service
    // container for whoever provides IGraphicsDeviceService -- the pattern ContentManager follows.
    class FlashComponent final : public GameComponent
    {
    public:
        explicit FlashComponent(Game& game)
            : GameComponent(game)
        {
        }

        void Initialize() override
        {
            service_ = getGameProperty().getServicesProperty().GetService<IGraphicsDeviceService>();
            GameComponent::Initialize();
        }

        IGraphicsDeviceService* service_ = nullptr;
    };

    class GraphicsDeviceServiceDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            flash_ = std::make_shared<FlashComponent>(*this);
            getComponentsProperty().Add(flash_);
            DemoGame::Initialize();

            auto& ctx = Context();
            // The GraphicsDeviceManager registered itself under both interfaces when it was created.
            GameServiceContainer& services = getServicesProperty();
            IGraphicsDeviceService* deviceService = services.GetService<IGraphicsDeviceService>();
            IGraphicsDeviceManager* deviceManager = services.GetService<IGraphicsDeviceManager>();
            ctx.Check(deviceService != nullptr && deviceManager != nullptr,
                      "Game.Services provides IGraphicsDeviceService and IGraphicsDeviceManager");
            ctx.Check(flash_->service_ == deviceService, "the component found the same service");
            ctx.Check(deviceService != nullptr && deviceService->getGraphicsDeviceProperty() == &getGraphicsDeviceProperty(),
                      "its GraphicsDevice is the game's device");
            if (deviceService != nullptr)
            {
                // The service's events tell a component when the device comes and goes.
                deviceService->getDeviceResetEvent().Add([this](System::Object*, const System::EventArgs&) { ++resets_; });
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            // Drawing through the device the component found.
            GraphicsDevice* device = flash_->service_ ? flash_->service_->getGraphicsDeviceProperty() : nullptr;
            if (device != nullptr)
            {
                const float t = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
                device->Clear(Color::Lerp(Color(30, 40, 90), Color(90, 150, 220), 0.5f + 0.5f * std::sin(t * 3.0f)));
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::shared_ptr<FlashComponent> flash_;
        int resets_ = 0;
    };
}

CNA_DEMOS_GAME(GraphicsDeviceServiceDemo, {
    .id = "game.services.graphics_device_service",
    .title = "The graphics device service",
    .description = "A component that only knows Game.Services looks up IGraphicsDeviceService -- which the "
                   "GraphicsDeviceManager registered, together with IGraphicsDeviceManager -- and clears the screen "
                   "through the GraphicsDevice it provides, as ContentManager finds its device.",
    .apis = {"IGraphicsDeviceService", "IGraphicsDeviceService::GraphicsDevice", "IGraphicsDeviceService::DeviceReset",
             "IGraphicsDeviceManager", "GameServiceContainer::GetService", "GameComponent::Initialize"},
    .tags = {"services", "graphics-device", "components", "architecture"},
    .frames = 10,
})
