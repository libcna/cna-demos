// SPDX-License-Identifier: MS-PL
// Texture2D::SaveAsPng and SaveAsJpeg: encode a texture as an image file, here into memory.
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
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
            double error = 0.0;
            for (std::size_t i = 0; i < original.size(); ++i)
                error += std::abs(original[i].getRProperty() - decoded[i].getRProperty()) +
                         std::abs(original[i].getGProperty() - decoded[i].getGProperty()) +
                         std::abs(original[i].getBProperty() - decoded[i].getBProperty());
            error /= static_cast<double>(original.size()) * 3.0;
            Context().Print("JPEG mean channel error: {:.2f} of 255", error);
            Context().Check(error < 4.0, "the JPEG round trip stays close to the original");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            // Original, PNG round trip, JPEG round trip.
            spriteBatch_->Begin();
            spriteBatch_->Draw(source_, Rectangle(16, 112, 256, 256), Color::White);
            spriteBatch_->Draw(fromPng_, Rectangle(272, 112, 256, 256), Color::White);
            spriteBatch_->Draw(fromJpeg_, Rectangle(528, 112, 256, 256), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D source_;
        Texture2D fromPng_;
        Texture2D fromJpeg_;
    };
}

CNA_DEMOS_GAME(SaveAsImageDemo, {
    .id = "graphics.texture2d.save_as_image",
    .title = "Save a texture as PNG or JPEG",
    .description = "Encodes a texture into memory streams with Texture2D::SaveAsPng (lossless) and SaveAsJpeg "
                   "(lossy) and decodes both again with FromStream to compare.",
    .apis = {"Texture2D::SaveAsPng", "Texture2D::SaveAsJpeg", "Texture2D::FromStream"},
    .tags = {"texture", "png", "jpeg", "save", "screenshot"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/gradient.png"},
})
