// SPDX-License-Identifier: MS-PL
// Texture2D::SaveAsPng and SaveAsJpeg: encode a texture as an image file, here into memory.
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/IO/MemoryStream.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::vector<Color> Pixels(const Texture2D& texture)
    {
        std::vector<Color> pixels(static_cast<std::size_t>(texture.getWidthProperty()) * texture.getHeightProperty());
        texture.GetData(pixels.data(), static_cast<int>(pixels.size()));
        return pixels;
    }

    class SaveAsImageDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            source_ = getContentProperty().Load<Texture2D>("textures/gradient");
            const int width = source_.getWidthProperty();
            const int height = source_.getHeightProperty();

            // PNG is lossless: decoding the bytes gives back exactly the same pixels.
            System::IO::MemoryStream png;
            source_.SaveAsPng(&png, width, height);
            const auto pngBytes = png.ToArray();
            Context().Print("SaveAsPng:  {} bytes", pngBytes.size());
            Context().Check(pngBytes.size() > 8 && pngBytes[1] == 'P' && pngBytes[2] == 'N' && pngBytes[3] == 'G',
                            "the stream holds a PNG file");
            png.setPositionProperty(0);
            fromPng_ = Texture2D::FromStream(device, png);
            Context().Check(Pixels(fromPng_) == Pixels(source_), "the PNG round trip is pixel-exact");

            // JPEG is lossy: smaller for photographs, but the pixels come back only approximately.
            System::IO::MemoryStream jpeg;
            source_.SaveAsJpeg(&jpeg, width, height);
            const auto jpegBytes = jpeg.ToArray();
            Context().Print("SaveAsJpeg: {} bytes", jpegBytes.size());
            Context().Check(jpegBytes.size() > 2 && jpegBytes[0] == 0xFF && jpegBytes[1] == 0xD8,
                            "the stream holds a JPEG file");
            jpeg.setPositionProperty(0);
            fromJpeg_ = Texture2D::FromStream(device, jpeg);

            const std::vector<Color> original = Pixels(source_);
            const std::vector<Color> decoded = Pixels(fromJpeg_);
            if (Context().Check(fromJpeg_.getWidthProperty() == width && fromJpeg_.getHeightProperty() == height &&
                                decoded.size() == original.size(), "the JPEG output has the requested source dimensions"))
            {
                double error = 0.0;
                for (std::size_t i = 0; i < original.size(); ++i)
                    error += std::abs(original[i].getRProperty() - decoded[i].getRProperty()) +
                             std::abs(original[i].getGProperty() - decoded[i].getGProperty()) +
                             std::abs(original[i].getBProperty() - decoded[i].getBProperty());
                error /= static_cast<double>(original.size()) * 3.0;
                Context().Print("JPEG mean channel error: {:.2f} of 255", error);
                Context().Check(error < 4.0, "the JPEG round trip stays close to the original");
            }

            // Width and height select the encoded image size, not just a stream buffer shape.
            // Request a non-square thumbnail from the square source, then decode to inspect it.
            System::IO::MemoryStream smallPng, smallJpeg;
            source_.SaveAsPng(&smallPng, 64, 32);
            source_.SaveAsJpeg(&smallJpeg, 64, 32);
            smallPng.setPositionProperty(0);
            smallJpeg.setPositionProperty(0);
            resizedPng_ = Texture2D::FromStream(device, smallPng);
            resizedJpeg_ = Texture2D::FromStream(device, smallJpeg);
            Context().Check(resizedPng_.getWidthProperty() == 64 && resizedPng_.getHeightProperty() == 32,
                            "the PNG stream encodes the requested 64x32 output dimensions");
            Context().Check(resizedJpeg_.getWidthProperty() == 64 && resizedJpeg_.getHeightProperty() == 32,
                            "the JPEG stream encodes the requested 64x32 output dimensions");
            Context().Check(source_.getWidthProperty() == width && source_.getHeightProperty() == height &&
                            Pixels(source_) == original, "resized exports leave the source texture unchanged");

            // XNA's save overloads use nearest-neighbor source coordinates rather than filtering.
            // A 2x2 -> 3x3 resize distinguishes floor(dst * src / dst) on both axes.
            Texture2D tinySource(device, 2, 2);
            const std::array<Color, 4> tinyPixels{
                Color(10, 20, 30), Color(40, 50, 60),
                Color(70, 80, 90), Color(100, 110, 120)};
            tinySource.SetData(tinyPixels.data(), 4);
            const std::array<Color, 9> expectedNearest{
                tinyPixels[0], tinyPixels[0], tinyPixels[1],
                tinyPixels[0], tinyPixels[0], tinyPixels[1],
                tinyPixels[2], tinyPixels[2], tinyPixels[3]};
            System::IO::MemoryStream nearestPng, nearestJpeg;
            tinySource.SaveAsPng(&nearestPng, 3, 3);
            tinySource.SaveAsJpeg(&nearestJpeg, 3, 3);
            nearestPng.setPositionProperty(0);
            Context().Check(Pixels(Texture2D::FromStream(device, nearestPng)) ==
                                std::vector<Color>(expectedNearest.begin(), expectedNearest.end()),
                            "PNG resizing uses XNA's nearest source texel mapping on both axes");

            Texture2D expectedTexture(device, 3, 3);
            expectedTexture.SetData(expectedNearest.data(), 9);
            System::IO::MemoryStream expectedJpeg;
            expectedTexture.SaveAsJpeg(&expectedJpeg, 3, 3);
            Context().Check(nearestJpeg.ToArray() == expectedJpeg.ToArray(),
                            "JPEG resizing encodes the same nearest-neighbor image as an authored target-sized texture");

            // XNA canonicalizes RGB only when alpha is exactly zero. Keep the two halves far wider
            // than a JPEG block so its lossy colour boundary cannot affect the center samples.
            Texture2D alphaSource(device, 64, 32);
            std::vector<Color> alphaPixels(64 * 32);
            for (int y = 0; y < 32; ++y)
                for (int x = 0; x < 64; ++x)
                    alphaPixels[static_cast<std::size_t>(y * 64 + x)] =
                        x < 32 ? Color(255, 0, 255, 0) : Color(255, 0, 255, 128);
            alphaSource.SetData(alphaPixels.data(), static_cast<int>(alphaPixels.size()));
            System::IO::MemoryStream alphaPng, alphaJpeg;
            alphaSource.SaveAsPng(&alphaPng, 64, 32);
            alphaSource.SaveAsJpeg(&alphaJpeg, 64, 32);
            alphaPng.setPositionProperty(0);
            alphaJpeg.setPositionProperty(0);
            const std::vector<Color> pngAlphaPixels = Pixels(Texture2D::FromStream(device, alphaPng));
            const std::vector<Color> jpegAlphaPixels = Pixels(Texture2D::FromStream(device, alphaJpeg));
            std::vector<Color> expectedPng = alphaPixels;
            for (int y = 0; y < 32; ++y)
                for (int x = 0; x < 32; ++x)
                    expectedPng[static_cast<std::size_t>(y * 64 + x)] = Color(0, 0, 0, 0);
            Context().Check(pngAlphaPixels == expectedPng,
                            "PNG clears RGB only for alpha zero and preserves RGB when alpha is nonzero");
            const Color jpegTransparent = jpegAlphaPixels[16 * 64 + 16];
            const Color jpegPartial = jpegAlphaPixels[16 * 64 + 48];
            Context().Check(jpegTransparent.getRProperty() <= 4 && jpegTransparent.getGProperty() <= 4 &&
                                jpegTransparent.getBProperty() <= 4 && jpegTransparent.getAProperty() == 255 &&
                                jpegPartial.getRProperty() >= 250 && jpegPartial.getGProperty() <= 5 &&
                                jpegPartial.getBProperty() >= 250 && jpegPartial.getAProperty() == 255,
                            "JPEG drops alpha after normalizing transparent RGB but retains nonzero-alpha colour");
            Context().Print("Both thumbnail streams decode to 64x32; all images below are drawn at 2x their own size.");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            // Original and full-size round trips, with smaller exports above their full-size versions.
            spriteBatch_->Begin();
            spriteBatch_->DrawString(*font_, "Requested size", Vector2(16, 16), Color::White);
            spriteBatch_->DrawString(*font_, "64 x 32", Vector2(16, 40), Color::White);
            spriteBatch_->DrawString(*font_, "PNG 64x32", Vector2(272, 2), Color::White);
            spriteBatch_->DrawString(*font_, "JPEG 64x32", Vector2(528, 2), Color::White);
            spriteBatch_->Draw(resizedPng_, Rectangle(272, 24, 128, 64), Color::White);
            spriteBatch_->Draw(resizedJpeg_, Rectangle(528, 24, 128, 64), Color::White);
            spriteBatch_->DrawString(*font_, "Source 128x128", Vector2(16, 90), Color::White);
            spriteBatch_->DrawString(*font_, "PNG 128x128", Vector2(272, 90), Color::White);
            spriteBatch_->DrawString(*font_, "JPEG 128x128", Vector2(528, 90), Color::White);
            spriteBatch_->Draw(source_, Rectangle(16, 112, 256, 256), Color::White);
            spriteBatch_->Draw(fromPng_, Rectangle(272, 112, 256, 256), Color::White);
            spriteBatch_->Draw(fromJpeg_, Rectangle(528, 112, 256, 256), Color::White);
            spriteBatch_->DrawString(*font_, "Export dimensions do not change the source texture.", Vector2(16, 408), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D source_;
        Texture2D fromPng_;
        Texture2D fromJpeg_;
        Texture2D resizedPng_, resizedJpeg_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(SaveAsImageDemo, {
    .id = "graphics.texture2d.save_as_image",
    .title = "Save a texture as PNG or JPEG",
    .description = "Encodes textures into memory streams with Texture2D::SaveAsPng (lossless) and SaveAsJpeg "
                   "(lossy), checks exact XNA nearest-neighbor resizing and transparent-RGB normalization, and verifies "
                   "sources stay unchanged.",
    .apis = {"Texture2D::SaveAsPng", "Texture2D::SaveAsJpeg", "Texture2D::FromStream", "Texture2D::Width",
             "Texture2D::Height"},
    .tags = {"texture", "png", "jpeg", "save", "screenshot"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/gradient.png", "fonts/pixel_font.cnb"},
})
