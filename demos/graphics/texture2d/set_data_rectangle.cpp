// SPDX-License-Identifier: MS-PL
// Texture2D::SetData with a rectangle: update part of a texture, like painting on a canvas.
#include <algorithm>
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
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

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

            device.getTexturesProperty()(0, nullptr);
            const Rectangle validArea(0, 0, Brush, Brush);
            const Rectangle outside(CanvasSize - 2, CanvasSize - 2, Brush, Brush);
            const Rectangle edgeProbe(CanvasSize - 1, CanvasSize - 1, 1, 1);
            const Rectangle originProbe(0, 0, 1, 1);
            const int pixelCount = Brush * Brush;
            const std::vector<Color> replacement(pixelCount, Color::Black);

            bool invalidWriteRefused = false;
            bool invalidReadRefused = false;
            std::vector<Color> invalidRead(pixelCount + 2, Color::Magenta);
            try
            {
                canvas_->SetData(0, &outside, replacement.data(), 0, pixelCount);
            }
            catch (const System::ArgumentException&)
            {
                invalidWriteRefused = true;
            }
            try
            {
                canvas_->GetData(0, &outside, invalidRead.data(), 1, pixelCount);
            }
            catch (const System::ArgumentException&)
            {
                invalidReadRefused = true;
            }
            Color preservedPixel;
            canvas_->GetData(0, &edgeProbe, &preservedPixel, 0, 1);
            Context().Check(invalidWriteRefused && invalidReadRefused &&
                                std::all_of(invalidRead.begin(), invalidRead.end(),
                                            [](const Color& value) { return value == Color::Magenta; }) &&
                                preservedPixel == Color::White,
                            "an out-of-bounds rectangle is refused before changing texture or caller data");

            bool shortWriteRefused = false;
            bool shortReadRefused = false;
            std::vector<Color> shortRead(pixelCount + 2, Color::Transparent);
            try
            {
                canvas_->SetData(0, &validArea, replacement.data(), 0, pixelCount - 1);
            }
            catch (const System::ArgumentException&)
            {
                shortWriteRefused = true;
            }
            try
            {
                canvas_->GetData(0, &validArea, shortRead.data(), 1, pixelCount - 1);
            }
            catch (const System::ArgumentException&)
            {
                shortReadRefused = true;
            }
            canvas_->GetData(0, &originProbe, &preservedPixel, 0, 1);
            Context().Check(shortWriteRefused && shortReadRefused &&
                                std::all_of(shortRead.begin(), shortRead.end(),
                                            [](const Color& value) { return value == Color::Transparent; }) &&
                                preservedPixel == Color::White,
                            "a short logical count is refused before changing texture or caller data");

            Texture2D disposed(device, 1, 1);
            Color disposedValue = Color::Gold;
            disposed.SetData(&disposedValue, 1);
            disposed.Dispose();
            bool disposedWriteRefused = false;
            bool disposedReadRefused = false;
            Color disposedOutput = Color::Magenta;
            try
            {
                disposed.SetData(&disposedValue, 1);
            }
            catch (const System::ObjectDisposedException&)
            {
                disposedWriteRefused = true;
            }
            try
            {
                disposed.GetData(&disposedOutput, 1);
            }
            catch (const System::ObjectDisposedException&)
            {
                disposedReadRefused = true;
            }
            Context().Check(disposedWriteRefused && disposedReadRefused &&
                                disposedOutput == Color::Magenta,
                            "a disposed texture refuses uploads and readback without changing caller data");
        }

        void Update(GameTime& gameTime) override
        {
            // Keep the texture unbound before every partial update.
            getGraphicsDeviceProperty().getTexturesProperty()(0, nullptr);

            // One brush stroke per update at a random (seeded) spot. Only those 16 pixels are sent
            // to the texture -- the rest of the canvas is left as it is.
            std::uniform_int_distribution<int> position(0, (CanvasSize - Brush) / Brush);
            const Rectangle area(position(random_) * Brush, position(random_) * Brush, Brush, Brush);
            const std::array<Color, 4> palette = {Color::Crimson, Color::DodgerBlue, Color::Gold, Color::SeaGreen};
            const int pixelCount = Brush * Brush;
            // startIndex selects a slice of the CPU array, independently of the texture rectangle.
            std::vector<Color> block(pixelCount + 2, Color::Magenta);
            std::fill(block.begin() + 1, block.end() - 1, palette[static_cast<std::size_t>(strokes_) % palette.size()]);
            canvas_->SetData(0, &area, block.data(), 1, pixelCount);

            if (strokes_++ == 0)
            {
                // GetData takes the same rectangle form: read back the stroke and a pixel beside it.
                std::vector<Color> stroke(pixelCount + 4, Color::Transparent);
                canvas_->GetData(0, &area, stroke.data(), 2, pixelCount);
                Context().Check(std::equal(block.begin() + 1, block.end() - 1, stroke.begin() + 2),
                                "the CPU source slice fills the texture rectangle and the destination slice");
                Context().Check(stroke[0] == Color::Transparent && stroke[1] == Color::Transparent &&
                                    stroke[stroke.size() - 2] == Color::Transparent && stroke.back() == Color::Transparent,
                                "GetData leaves destination elements outside its slice untouched");
                Context().Print("SetData source startIndex=1; GetData destination startIndex=2; {} pixels", pixelCount);
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
    .description = "Paints 4x4 blocks onto a canvas texture with nonzero CPU array offsets, preserves surrounding "
                   "elements, and atomically rejects invalid rectangles, short logical buffers and disposed resources. "
                   "Unsets the texture from GraphicsDevice::Textures before each update as XNA requires.",
    .apis = {"Texture2D::SetData", "Texture2D::GetData", "GraphicsDevice::Textures", "TextureCollection::Item"},
    .tags = {"texture", "canvas", "paint", "pixels"},
    .frames = 60,
    .restrictions = "CNA-DEMOS-ISSUE-003: CNA's SpriteBatch does not set Textures[0], so forgetting to unset it "
                    "does not throw on CNA as it does on XNA.",
})
