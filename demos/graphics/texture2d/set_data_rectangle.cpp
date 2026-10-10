// SPDX-License-Identifier: MS-PL
// Texture2D::SetData with a rectangle: update part of a texture, like painting on a canvas.
#include <array>
#include <cstddef>
#include <memory>
#include <random>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCollection.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int CanvasSize = 64;
    constexpr int Brush = 4;

    class SetDataRectangleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            random_.seed(Context().Seed());

            canvas_ = std::make_unique<Texture2D>(device, CanvasSize, CanvasSize);
            const std::vector<Color> paper(static_cast<std::size_t>(CanvasSize) * CanvasSize, Color::White);
            canvas_->SetData(paper.data(), static_cast<int>(paper.size()));

            // XNA refuses SetData on a texture that is set on the device. SpriteBatch leaves the last
            // texture it drew in Textures[0], so a texture painted every frame must be unset first.
            device.getTexturesProperty()(0, canvas_.get());
            bool refused = false;
            try
            {
                canvas_->SetData(paper.data(), static_cast<int>(paper.size()));
            }
            catch (const System::InvalidOperationException& e)
            {
                refused = true;
                Context().Print("SetData while set on the device: {}", e.what());
            }
            Context().Check(refused, "SetData on a texture set in GraphicsDevice::Textures throws");
        }

        void Update(GameTime& gameTime) override
        {
            // Textures[0] = null in XNA (see LoadContent).
            getGraphicsDeviceProperty().getTexturesProperty()(0, nullptr);

            // One brush stroke per update at a random (seeded) spot. Only those 16 pixels are sent
            // to the texture -- the rest of the canvas is left as it is.
            std::uniform_int_distribution<int> position(0, (CanvasSize - Brush) / Brush);
            const Rectangle area(position(random_) * Brush, position(random_) * Brush, Brush, Brush);
            const std::array<Color, 4> palette = {Color::Crimson, Color::DodgerBlue, Color::Gold, Color::SeaGreen};
            const std::vector<Color> block(Brush * Brush, palette[static_cast<std::size_t>(strokes_) % palette.size()]);
            canvas_->SetData(0, &area, block.data(), 0, static_cast<int>(block.size()));

            if (strokes_++ == 0)
            {
                // GetData takes the same rectangle form: read back the stroke and a pixel beside it.
                std::vector<Color> stroke(block.size());
                canvas_->GetData(0, &area, stroke.data(), 0, static_cast<int>(stroke.size()));
                Context().Check(stroke == block, "the rectangle now holds the brush colour");
                const Rectangle beside(area.X == 0 ? Brush : area.X - 1, area.Y, 1, 1);
                Color pixel;
                canvas_->GetData(0, &beside, &pixel, 0, 1);
                Context().Check(pixel == Color::White, "the pixel next to it is untouched");
            }
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DimGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*canvas_, Rectangle(208, 48, 384, 384), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> canvas_;
        std::minstd_rand random_;
        int strokes_ = 0;
    };
}

CNA_DEMOS_GAME(SetDataRectangleDemo, {
    .id = "graphics.texture2d.set_data_rectangle",
    .title = "Update part of a texture",
    .description = "Paints 4x4 blocks onto a canvas texture with the rectangle overload of Texture2D::SetData, one "
                   "block per update, after unsetting it from GraphicsDevice::Textures as XNA requires.",
    .apis = {"Texture2D::SetData", "Texture2D::GetData", "GraphicsDevice::Textures", "TextureCollection::Item"},
    .tags = {"texture", "canvas", "paint", "pixels"},
    .frames = 60,
    .restrictions = "CNA-DEMOS-ISSUE-003: CNA's SpriteBatch does not set Textures[0], so forgetting to unset it "
                    "does not throw on CNA as it does on XNA.",
})
