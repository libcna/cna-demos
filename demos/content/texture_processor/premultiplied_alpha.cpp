// SPDX-License-Identifier: MS-PL
// Compiled textures have premultiplied alpha (XNA's default), loose image files straight alpha --
// and SpriteBatch's default blend state expects the first.
#include <cstdlib>
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A pixel on the soft edge of the 64x64 disc, where alpha is about half.
    constexpr int EdgeX = 32;
    constexpr int EdgeY = 8;

    Color Pixel(const Texture2D& texture, int x, int y)
    {
        std::vector<Color> pixels(static_cast<std::size_t>(texture.getWidthProperty() * texture.getHeightProperty()));
        texture.GetData(pixels.data(), static_cast<int>(pixels.size()));
        return pixels[static_cast<std::size_t>(y * texture.getWidthProperty() + x)];
    }

    class PremultipliedAlphaDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            // The same white disc twice: as a loose PNG, and compiled by the content pipeline,
            // whose TextureProcessor premultiplies by default (premultiplyAlpha = true, as in XNA).
            loose_ = getContentProperty().Load<Texture2D>("textures/soft_circle");
            compiled_ = getContentProperty().Load<Texture2D>("textures/soft_circle_compiled");

            const Color straight = Pixel(loose_, EdgeX, EdgeY);
            const Color premultiplied = Pixel(compiled_, EdgeX, EdgeY);
            Context().Print("edge pixel: loose ({}, {}, {}, {}), compiled ({}, {}, {}, {})", straight.getRProperty(),
                            straight.getGProperty(), straight.getBProperty(), straight.getAProperty(),
                            premultiplied.getRProperty(), premultiplied.getGProperty(), premultiplied.getBProperty(),
                            premultiplied.getAProperty());
            Context().Check(straight.getRProperty() == 255 && straight.getAProperty() < 255,
                            "the loose PNG stores straight alpha: full white with partial alpha");
            Context().Check(premultiplied.getAProperty() == straight.getAProperty() &&
                                std::abs(premultiplied.getRProperty() - premultiplied.getAProperty()) <= 1,
                            "the compiled texture stores premultiplied alpha: colour scaled by alpha");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            // Left: compiled + the default AlphaBlend (premultiplied) -- correct.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend, &SamplerState::PointClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(compiled_, Rectangle(40, 120, 192, 192), Color::White);
            // Middle: loose (straight) + AlphaBlend -- the soft edge comes out too bright.
            spriteBatch_->Draw(loose_, Rectangle(304, 120, 192, 192), Color::White);
            spriteBatch_->End();
            // Right: loose + NonPremultiplied, the blend state for straight alpha -- correct again.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::NonPremultiplied, &SamplerState::PointClamp,
                                nullptr, nullptr);
            spriteBatch_->Draw(loose_, Rectangle(568, 120, 192, 192), Color::White);
            spriteBatch_->End();

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "compiled", Vector2(40.0f, 330.0f), Color::White);
            spriteBatch_->DrawString(*font_, "AlphaBlend", Vector2(40.0f, 355.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "loose PNG", Vector2(304.0f, 330.0f), Color::White);
            spriteBatch_->DrawString(*font_, "AlphaBlend (wrong)", Vector2(304.0f, 355.0f), Color::OrangeRed);
            spriteBatch_->DrawString(*font_, "loose PNG", Vector2(568.0f, 330.0f), Color::White);
            spriteBatch_->DrawString(*font_, "NonPremultiplied", Vector2(568.0f, 355.0f), Color::LightGray);
            spriteBatch_->End();

            if (!checked_)
            {
                // The edge pixel, drawn 3x: both correct versions agree, the mismatched one is brighter.
                const CnaDemos::BackBufferPixels screen(device);
                const Color right = screen.At(40 + EdgeX * 3 + 1, 120 + EdgeY * 3 + 1);
                const Color wrong = screen.At(304 + EdgeX * 3 + 1, 120 + EdgeY * 3 + 1);
                const Color fixedUp = screen.At(568 + EdgeX * 3 + 1, 120 + EdgeY * 3 + 1);
                Context().Print("edge on screen: compiled {}, loose+AlphaBlend {}, loose+NonPremultiplied {}",
                                right.getRProperty(), wrong.getRProperty(), fixedUp.getRProperty());
                // Colour only: the two blend states write different alpha into the back buffer.
                Context().Check(std::abs(right.getRProperty() - fixedUp.getRProperty()) <= 2,
                                "premultiplied + AlphaBlend gives the colour of straight + NonPremultiplied");
                Context().Check(wrong.getRProperty() > right.getRProperty() + 40,
                                "straight alpha under AlphaBlend is too bright at the soft edge");
                checked_ = true;
            }
            Game::Draw(gameTime);
        }

    private:
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D loose_;
        Texture2D compiled_;
    };
}

CNA_DEMOS_GAME(PremultipliedAlphaDemo, {
    .id = "content.texture_processor.premultiplied_alpha",
    .title = "Premultiplied alpha in compiled textures",
    .description = "Compares a soft-edged disc loaded as a loose PNG (straight alpha) and compiled by the content "
                   "pipeline (premultiplied, XNA's default): with SpriteBatch's default AlphaBlend only the compiled "
                   "one is right; the loose one needs BlendState::NonPremultiplied.",
    .apis = {"ContentManager::Load", "Texture2D::GetData", "BlendState::AlphaBlend", "BlendState::NonPremultiplied"},
    .tags = {"content", "pipeline", "premultiplied", "alpha", "blending", "texture"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb", "textures/soft_circle.png", "textures/soft_circle_compiled.cnb"},
})
