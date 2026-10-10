// SPDX-License-Identifier: MS-PL
// GraphicsDevice::GetBackBufferData: read rendered pixels back from the back buffer.
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class GetBackBufferDataDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            spriteBatch_->Begin();
            spriteBatch_->Draw(*pixel_, Rectangle(100, 100, 200, 100), Color::Orange);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
                ReadBack(device);

            Game::Draw(gameTime);
        }

    private:
        void ReadBack(GraphicsDevice& device)
        {
            // The whole back buffer: elementCount must equal width * height exactly.
            const int width = device.getPresentationParametersProperty().getBackBufferWidthProperty();
            const int height = device.getPresentationParametersProperty().getBackBufferHeightProperty();
            std::vector<Color> all(static_cast<std::size_t>(width) * height);
            device.GetBackBufferData(all.data(), static_cast<int>(all.size()));
            const Color inside = all[static_cast<std::size_t>(150) * width + 150];
            const Color outside = all[static_cast<std::size_t>(50) * width + 50];
            Context().Print("whole buffer: pixel (150, 150) = {}, pixel (50, 50) = {}", inside.ToString(),
                            outside.ToString());
            Context().Check(inside == Color::Orange, "a pixel inside the rectangle is orange");
            Context().Check(outside == Color::Black, "a pixel outside it is the clear colour");

            // Only a region: rows of the rectangle are packed one after another into the array.
            const Rectangle region(290, 190, 20, 20);
            std::vector<Color> part(static_cast<std::size_t>(region.Width) * region.Height);
            device.GetBackBufferData(&region, part.data(), 0, static_cast<int>(part.size()));
            Context().Print("region {{X:290 Y:190 Width:20 Height:20}}: first {} last {}", part.front().ToString(),
                            part.back().ToString());
            Context().Check(part.front() == Color::Orange, "the region's top-left pixel is inside the rectangle");
            Context().Check(part.back() == Color::Black, "the region's bottom-right pixel is outside it");
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
    };
}

CNA_DEMOS_GAME(GetBackBufferDataDemo, {
    .id = "graphics.device.get_back_buffer_data",
    .title = "Read back the back buffer",
    .description = "Draws a rectangle and reads the pixels back with GraphicsDevice::GetBackBufferData, once for the "
                   "whole back buffer and once for a small region.",
    .apis = {"GraphicsDevice::GetBackBufferData"},
    .tags = {"readback", "screenshot", "pixels"},
    .frames = 3,
    .deterministic = true,
    .restrictions = "HiDef profile only: XNA refuses GetBackBufferData under Reach.",
})
