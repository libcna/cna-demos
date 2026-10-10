// SPDX-License-Identifier: MS-PL
// Texture2D::FromStream: load a PNG at run time without the content pipeline, optionally resized.
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
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr const char* ImagePath = "assets/textures/sprite_strip.png";

    class FromStreamDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);

            // Any readable Stream holding PNG, JPEG, BMP or GIF bytes; here a file shipped with the game.
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
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            // Each texture drawn 3x its own size.
            spriteBatch_->Draw(original_, Rectangle(16, 40, 384, 96), Color::White);
            spriteBatch_->Draw(fitted_, Rectangle(16, 200, 192, 48), Color::White);
            spriteBatch_->Draw(zoomed_, Rectangle(240, 200, 192, 192), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D original_;
        Texture2D fitted_;
        Texture2D zoomed_;
    };
}

CNA_DEMOS_GAME(FromStreamDemo, {
    .id = "graphics.texture2d.from_stream",
    .title = "Load an image file at run time",
    .description = "Creates textures from a PNG stream with Texture2D::FromStream: as stored, fitted into 64x64 and "
                   "zoomed (filled and cropped) to 64x64.",
    .apis = {"Texture2D::FromStream", "Texture2D::Width", "Texture2D::Height", "Texture2D::Bounds", "Texture::Format",
             "Texture::LevelCount", "TitleContainer::OpenStream"},
    .tags = {"texture", "image", "png", "load", "runtime"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/sprite_strip.png"},
})
