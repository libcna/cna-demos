// SPDX-License-Identifier: MS-PL
// Texture2D::SaveAsPng and SaveAsJpeg: encode a texture as an image file, here into memory.
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
    .description = "Encodes a texture into memory streams with Texture2D::SaveAsPng (lossless) and SaveAsJpeg "
                   "(lossy), decodes full-size and 64x32 exports with FromStream, and verifies the source stays unchanged.",
    .apis = {"Texture2D::SaveAsPng", "Texture2D::SaveAsJpeg", "Texture2D::FromStream", "Texture2D::Width",
             "Texture2D::Height"},
    .tags = {"texture", "png", "jpeg", "save", "screenshot"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/gradient.png", "fonts/pixel_font.cnb"},
})
