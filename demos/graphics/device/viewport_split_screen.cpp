// SPDX-License-Identifier: MS-PL
// Split screen with GraphicsDevice::Viewport: each player draws into its own half of the window.
#include <array>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ViewportSplitScreenDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const Viewport full = device.getViewportProperty();
            const int half = full.getWidthProperty() / 2;

            const std::array<Viewport, 2> players = {
                Viewport(0, 0, half, full.getHeightProperty()),
                Viewport(half, 0, full.getWidthProperty() - half, full.getHeightProperty()),
            };
            const std::array<Color, 2> backgrounds = {Color::DarkSlateBlue, Color::DarkOliveGreen};

            if (FramesDrawn() == 0)
                CheckClearIgnoresViewport(device, players[0], half);

            // XNA 4.0's Clear always clears the whole render target, whatever the viewport, so
            // clear once and let each player paint its own background.
            device.Clear(Color::Black);

            for (std::size_t i = 0; i < players.size(); ++i)
            {
                // From here on, drawing is positioned in and clipped to this player's half.
                device.setViewportProperty(players[i]);

                // SpriteBatch builds its projection from the current viewport, so (0, 0) is the
                // top-left corner of this player's half and the same coordinates work for both.
                spriteBatch_->Begin();
                spriteBatch_->Draw(*pixel_,
                                   Rectangle(0, 0, players[i].getWidthProperty(), players[i].getHeightProperty()),
                                   backgrounds[i]);
                spriteBatch_->Draw(arrow_, Vector2(40.0f, 40.0f), Color::White);
                // A bar that runs past the viewport's right edge is clipped there.
                spriteBatch_->Draw(*pixel_, Rectangle(300, 300, 300, 40), Color::Gold);
                spriteBatch_->End();
            }

            // Restore the full-window viewport for anything drawn afterwards.
            device.setViewportProperty(full);

            if (FramesDrawn() == 0)
                CheckPixels(device, half);

            Game::Draw(gameTime);
        }

    private:
        static std::vector<Color> ReadBackBuffer(GraphicsDevice& device)
        {
            const auto& parameters = device.getPresentationParametersProperty();
            std::vector<Color> pixels(static_cast<std::size_t>(parameters.getBackBufferWidthProperty()) *
                                      parameters.getBackBufferHeightProperty());
            device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
            return pixels;
        }

        void CheckClearIgnoresViewport(GraphicsDevice& device, const Viewport& leftHalf, int half)
        {
            const Viewport previous = device.getViewportProperty();
            device.setViewportProperty(leftHalf);
            device.Clear(Color::Magenta);
            device.setViewportProperty(previous);

            const std::vector<Color> pixels = ReadBackBuffer(device);
            Context().Check(pixels[static_cast<std::size_t>(half) + 10] == Color::Magenta,
                            "Clear with the left-half viewport set also cleared the right half");
        }

        void CheckPixels(GraphicsDevice& device, int half)
        {
            const int width = device.getPresentationParametersProperty().getBackBufferWidthProperty();
            const std::vector<Color> pixels = ReadBackBuffer(device);
            const auto at = [&](int x, int y) { return pixels[static_cast<std::size_t>(y) * width + x]; };

            Context().Check(at(10, 10) == Color::DarkSlateBlue, "the left half has the left player's background");
            Context().Check(at(half + 10, 10) == Color::DarkOliveGreen,
                            "the right half has the right player's background");
            Context().Check(at(350, 320) == Color::Gold, "the left player's bar is drawn inside its viewport");
            Context().Check(at(half + 50, 320) == Color::DarkOliveGreen,
                            "the left player's bar is clipped at the viewport edge");
            Context().Check(at(half + 350, 320) == Color::Gold,
                            "the right player's bar starts at the same viewport-relative x");
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D arrow_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(ViewportSplitScreenDemo, {
    .id = "graphics.device.viewport_split_screen",
    .title = "Split screen with viewports",
    .description = "Sets GraphicsDevice::Viewport to each half of the window in turn: SpriteBatch draws in viewport "
                   "coordinates, clipped to the half, while Clear still clears the whole back buffer (XNA 4.0).",
    .apis = {"GraphicsDevice::Viewport", "Viewport::Viewport", "GraphicsDevice::Clear", "SpriteBatch::Begin"},
    .tags = {"viewport", "split-screen", "multiplayer", "2d"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
})
