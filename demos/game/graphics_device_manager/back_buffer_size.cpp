// SPDX-License-Identifier: MS-PL
// Choosing the back-buffer size: PreferredBackBufferWidth/Height and ApplyChanges.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class BackBufferSizeDemo final : public CnaDemos::DemoGame
    {
    public:
        BackBufferSizeDemo()
        {
            // Set before the device exists (in the constructor), these choose the initial size.
            Graphics().setPreferredBackBufferWidthProperty(640);
            Graphics().setPreferredBackBufferHeightProperty(360);
        }

    protected:
        void LoadContent() override
        {
            const Viewport viewport = getGraphicsDeviceProperty().getViewportProperty();
            Context().Print("initial back buffer: {}x{}", viewport.getWidthProperty(), viewport.getHeightProperty());
            Context().Check(viewport.getWidthProperty() == 640 && viewport.getHeightProperty() == 360,
                            "the device was created at the preferred size");
        }

        void Update(GameTime& gameTime) override
        {
            // Later changes take effect when ApplyChanges is called.
            if (++updates_ == 3)
            {
                Graphics().setPreferredBackBufferWidthProperty(800);
                Graphics().setPreferredBackBufferHeightProperty(600);
                Graphics().ApplyChanges();

                const auto& parameters = getGraphicsDeviceProperty().getPresentationParametersProperty();
                Context().Print("after ApplyChanges: {}x{}", parameters.getBackBufferWidthProperty(),
                                parameters.getBackBufferHeightProperty());
                Context().Check(parameters.getBackBufferWidthProperty() == 800
                                    && parameters.getBackBufferHeightProperty() == 600,
                                "ApplyChanges resized the back buffer");
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(updates_ < 3 ? Color::Navy : Color::DarkGreen);
            Game::Draw(gameTime);
        }

    private:
        int updates_ = 0;
    };
}

CNA_DEMOS_GAME(BackBufferSizeDemo, {
    .id = "game.graphics_device_manager.back_buffer_size",
    .title = "Back-buffer size",
    .description = "Creates the device at 640x360 with PreferredBackBufferWidth/Height, then resizes it to 800x600 "
                   "at run time with GraphicsDeviceManager::ApplyChanges.",
    .apis = {"GraphicsDeviceManager::PreferredBackBufferWidth", "GraphicsDeviceManager::PreferredBackBufferHeight",
             "GraphicsDeviceManager::ApplyChanges", "PresentationParameters::BackBufferWidth",
             "PresentationParameters::BackBufferHeight", "GraphicsDevice::Viewport",
             "GraphicsDevice::PresentationParameters"},
    .tags = {"resolution", "window", "device"},
    .frames = 6,
})
