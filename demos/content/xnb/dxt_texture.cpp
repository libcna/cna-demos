// SPDX-License-Identifier: MS-PL
// XNB content: textures compiled into XNA's own format, one of them DXT-compressed.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DxtTextureDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // content_xnb/ is built with `cna_add_content(... FORMAT xnb)`: .xnb files as an XNA
            // content project wrote them, loaded by the same ContentManager::Load. One wheel uses
            // the processor's default Color format, the other textureFormat "DxtCompressed".
            colour_ = getContentProperty().Load<Texture2D>("xnb/textures/colour_wheel");
            dxt_ = getContentProperty().Load<Texture2D>("xnb/textures/colour_wheel_dxt");
            Context().Print("colour_wheel: {}, colour_wheel_dxt: {}", CnaDemos::Name(colour_.getFormatProperty()),
                            CnaDemos::Name(dxt_.getFormatProperty()));
            Context().Check(colour_.getFormatProperty() == SurfaceFormat::Color, "the default output is SurfaceFormat::Color");
            // An opaque image needs no more than DXT1's one-bit alpha: 4 bits per pixel, an
            // eighth of Color's 32.
            Context().Check(dxt_.getFormatProperty() == SurfaceFormat::Dxt1,
                            "DxtCompressed picks Dxt1 for an image without partial transparency");
            Context().Check(dxt_.getWidthProperty() == 128 && dxt_.getHeightProperty() == 128, "the size is unchanged");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(colour_, Rectangle(40, 100, 128, 128), Color::White);
            spriteBatch_->Draw(dxt_, Rectangle(200, 100, 128, 128), Color::White);
            // Zoomed in on the same spot: DXT stores 4x4 blocks of two colours plus blends.
            const Rectangle corner(52, 2, 24, 24);
            spriteBatch_->Draw(colour_, Rectangle(380, 100, 192, 192), corner, Color::White);
            spriteBatch_->Draw(dxt_, Rectangle(590, 100, 192, 192), corner, Color::White);
            spriteBatch_->DrawString(*font_, "XNB: " + std::string(CnaDemos::Name(colour_.getFormatProperty())) +
                                                 " and " + std::string(CnaDemos::Name(dxt_.getFormatProperty())),
                                     Vector2(40.0f, 30.0f), Color::White);
            spriteBatch_->DrawString(*font_, "Color", Vector2(40.0f, 240.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "Dxt1", Vector2(200.0f, 240.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "zoomed: Color | Dxt1", Vector2(380.0f, 310.0f), Color::LightGray);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D colour_;
        Texture2D dxt_;
    };
}

CNA_DEMOS_GAME(DxtTextureDemo, {
    .id = "content.xnb.dxt_texture",
    .title = "XNB textures and DXT compression",
    .description = "Loads two textures compiled into XNA's XNB format: one as SurfaceFormat::Color, one with the "
                   "TextureProcessor's DxtCompressed output (Dxt1), drawn side by side and zoomed to show the blocks.",
    .apis = {"ContentManager::Load", "Texture::Format", "SurfaceFormat"},
    .tags = {"content", "xnb", "dxt", "compression", "s3tc", "texture"},
    .frames = 3,
    .assets = {"fonts/pixel_font.cnb", "xnb/textures/colour_wheel.xnb", "xnb/textures/colour_wheel_dxt.xnb"},
})
