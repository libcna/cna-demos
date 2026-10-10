// SPDX-License-Identifier: MS-PL
// GraphicsDevice::ScissorRectangle with RasterizerState::ScissorTestEnable: clip drawing to a rectangle.
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/CullMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Rectangle ClipArea(200, 120, 400, 240);

    class ScissorRectangleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");

            // The scissor test is part of the rasterizer state; none of the built-in states enable it.
            scissorState_ = std::make_unique<RasterizerState>();
            scissorState_->setCullModeProperty(CullMode::None);
            scissorState_->setScissorTestEnableProperty(true);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);

            // Pixels outside this rectangle (in back-buffer pixels) are discarded while the
            // scissor test is on.
            device.setScissorRectangleProperty(ClipArea);

            // The texture is drawn over the whole window, but only the scissor rectangle shows it.
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointWrap, nullptr,
                                scissorState_.get());
            spriteBatch_->Draw(checker_, Rectangle(0, 0, 800, 480), Rectangle(0, 0, 400, 240), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
                CheckPixels(device);

            Game::Draw(gameTime);
        }

    private:
        void CheckPixels(GraphicsDevice& device)
        {
            const int width = device.getPresentationParametersProperty().getBackBufferWidthProperty();
            const int height = device.getPresentationParametersProperty().getBackBufferHeightProperty();
            std::vector<Color> pixels(static_cast<std::size_t>(width) * height);
            device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
            const auto at = [&](int x, int y) { return pixels[static_cast<std::size_t>(y) * width + x]; };

            Context().Check(at(ClipArea.X - 1, ClipArea.Y + 10) == Color::DarkSlateGray,
                            "the pixel just left of the scissor rectangle keeps the clear colour");
            Context().Check(at(ClipArea.X + 10, ClipArea.Y - 1) == Color::DarkSlateGray,
                            "the pixel just above the scissor rectangle keeps the clear colour");
            Context().Check(at(ClipArea.getRightProperty() - 1, ClipArea.getBottomProperty() - 1) != Color::DarkSlateGray,
                            "the last pixel inside the scissor rectangle is drawn");
            Context().Check(at(ClipArea.getRightProperty(), ClipArea.getBottomProperty()) == Color::DarkSlateGray,
                            "the pixel just past its bottom-right corner keeps the clear colour");
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        std::unique_ptr<RasterizerState> scissorState_;
    };
}

CNA_DEMOS_GAME(ScissorRectangleDemo, {
    .id = "graphics.device.scissor_rectangle",
    .title = "Scissor rectangle",
    .description = "Draws a window-sized sprite through a RasterizerState with ScissorTestEnable and a "
                   "GraphicsDevice::ScissorRectangle, so only the rectangle is drawn.",
    .apis = {"GraphicsDevice::ScissorRectangle", "RasterizerState::ScissorTestEnable", "RasterizerState::RasterizerState",
             "SpriteBatch::Begin"},
    .tags = {"scissor", "clipping", "rasterizer", "2d", "ui"},
    .frames = 5,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
