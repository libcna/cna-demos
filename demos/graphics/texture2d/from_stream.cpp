// SPDX-License-Identifier: MS-PL
// Texture2D::FromStream: load PNG/GIF data at run time without the content pipeline.
#include <array>
#include <cstdint>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr const char* ImagePath = "assets/textures/sprite_strip.png";
    constexpr std::array<std::uint8_t, 79> GifBytes = {
        0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x02, 0x00, 0x02, 0x00, 0xF1, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xC8, 0xA7, 0x11, 0xD1, 0xAE, 0x10, 0x00, 0x00,
        0x00, 0x21, 0xF9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x21, 0xFF, 0x0B,
        0x49, 0x6D, 0x61, 0x67, 0x65, 0x4D, 0x61, 0x67, 0x69, 0x63, 0x6B, 0x0D,
        0x67, 0x61, 0x6D, 0x6D, 0x61, 0x3D, 0x30, 0x2E, 0x34, 0x35, 0x34, 0x35,
        0x35, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00,
        0x02, 0x03, 0x04, 0x14, 0x05, 0x00, 0x3B
    };

    class FromStreamDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);

            // Any readable Stream holding PNG, JPEG or GIF bytes; here a file shipped with the game.
            {
                const std::unique_ptr<System::IO::Stream> stream = TitleContainer::OpenStream(ImagePath);
                original_ = Texture2D::FromStream(device, *stream);
            }
            const Rectangle bounds = original_.getBoundsProperty();
            Context().Print("FromStream: {}x{}, format {}, {} level(s), Bounds {{X:{} Y:{} Width:{} Height:{}}}",
                            original_.getWidthProperty(), original_.getHeightProperty(),
                            CnaDemos::Name(original_.getFormatProperty()), original_.getLevelCountProperty(), bounds.X,
                            bounds.Y, bounds.Width, bounds.Height);
            Context().Check(original_.getWidthProperty() == 128 && original_.getHeightProperty() == 32,
                            "the texture has the image's size");
            Context().Check(original_.getLevelCountProperty() == 1, "FromStream creates no mipmaps");

            // Asked for 64x64 with zoom = false, the image is scaled to fit and keeps its 4:1 shape.
            {
                const std::unique_ptr<System::IO::Stream> stream = TitleContainer::OpenStream(ImagePath);
                fitted_ = Texture2D::FromStream(device, *stream, 64, 64, false);
            }
            Context().Print("fit  64x64 -> {}x{}", fitted_.getWidthProperty(), fitted_.getHeightProperty());
            Context().Check(fitted_.getWidthProperty() == 64 && fitted_.getHeightProperty() == 16,
                            "zoom = false keeps the aspect ratio");

            // With zoom = true it fills exactly 64x64 and the overflow (the outer frames) is cropped.
            {
                const std::unique_ptr<System::IO::Stream> stream = TitleContainer::OpenStream(ImagePath);
                zoomed_ = Texture2D::FromStream(device, *stream, 64, 64, true);
            }
            Context().Print("zoom 64x64 -> {}x{}", zoomed_.getWidthProperty(), zoomed_.getHeightProperty());
            Context().Check(zoomed_.getWidthProperty() == 64 && zoomed_.getHeightProperty() == 64,
                            "zoom = true produces exactly the requested size");

            System::IO::MemoryStream gifStream(GifBytes.data(),
                                                static_cast<System::IO::intcs>(GifBytes.size()));
            gif_ = Texture2D::FromStream(device, gifStream);
            std::array<Color, 4> gifPixels;
            gif_.GetData(gifPixels.data(), static_cast<int>(gifPixels.size()));
            Context().Check(gif_.getWidthProperty() == 2 && gif_.getHeightProperty() == 2 &&
                                gifPixels == std::array<Color, 4>{Color::Transparent, Color::Transparent,
                                                                 Color(209, 174, 16, 255),
                                                                 Color(200, 167, 17, 255)},
                            "a paletted GIF decodes its transparent and opaque pixels exactly");

            bool truncatedRefused = false;
            try
            {
                System::IO::MemoryStream truncated(GifBytes.data(), 20);
                (void)Texture2D::FromStream(device, truncated);
            }
            catch (const System::InvalidOperationException&)
            {
                truncatedRefused = true;
            }
            Context().Check(truncatedRefused,
                            "a truncated recognized image is refused as invalid encoded data");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // Each texture drawn 3x its own size.
            spriteBatch_->Draw(original_, Rectangle(16, 40, 384, 96), Color::White);
            spriteBatch_->Draw(fitted_, Rectangle(16, 200, 192, 48), Color::White);
            spriteBatch_->Draw(zoomed_, Rectangle(240, 200, 192, 192), Color::White);
            spriteBatch_->Draw(gif_, Rectangle(480, 200, 192, 192), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D original_;
        Texture2D fitted_;
        Texture2D zoomed_;
        Texture2D gif_;
    };
}

CNA_DEMOS_GAME(FromStreamDemo, {
    .id = "graphics.texture2d.from_stream",
    .title = "Load an image file at run time",
    .description = "Creates textures from a PNG stream as stored, fitted and center-cropped; decodes exact palette "
                   "and transparency values from a GIF; and rejects a truncated recognized image.",
    .apis = {"Texture2D::FromStream", "Texture2D::Width", "Texture2D::Height", "Texture2D::Bounds", "Texture::Format",
             "Texture::LevelCount", "TitleContainer::OpenStream"},
    .tags = {"texture", "image", "png", "gif", "load", "runtime"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/sprite_strip.png"},
})
