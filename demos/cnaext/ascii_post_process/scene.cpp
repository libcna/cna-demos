// SPDX-License-Identifier: MS-PL
// CNAEXT AsciiPostProcessEffect: a finished frame redrawn as a grid of glyph cells, on any renderer.
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>

#include "CNA/Graphics/AsciiPostProcessEffect.hpp"
#include "CNA/Graphics/AsciiQuantizeMode.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
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
    class AsciiSceneDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            gradient_ = getContentProperty().Load<Texture2D>("textures/gradient");
            const auto& pp = device.getPresentationParametersProperty();
            scene_ = std::make_unique<RenderTarget2D>(device, pp.getBackBufferWidthProperty(), pp.getBackBufferHeightProperty());
            ascii_ = std::make_unique<CNA::Graphics::AsciiPostProcessEffect>(device);

            int cellWidth = 0;
            int cellHeight = 0;
            ascii_->getCellSize(cellWidth, cellHeight);
            Context().Check(cellWidth == 8 && cellHeight == 8,
                            "the source-cell size starts at the documented 8x8 default");
            bool rejected = false;
            try { ascii_->setCellSize(0, 12); }
            catch (const std::invalid_argument&) { rejected = true; }
            ascii_->getCellSize(cellWidth, cellHeight);
            Context().Check(rejected && cellWidth == 8 && cellHeight == 8,
                            "a non-positive cell size is rejected without changing either dimension");
            ascii_->setQuantizeMode(CNA::Graphics::AsciiQuantizeMode::Color);
            ascii_->setCellSize(8, 12); // source pixels per character cell
            ascii_->getCellSize(cellWidth, cellHeight);
            Context().Check(cellWidth == 8 && cellHeight == 12 &&
                                ascii_->getQuantizeMode() == CNA::Graphics::AsciiQuantizeMode::Color,
                            "the configured colour mode and rectangular source-cell size read back");
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            time_ += static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // 1. Draw the game as usual, but into a render target.
            device.SetRenderTarget(scene_.get());
            device.Clear(Color(20, 24, 40));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::LinearClamp, nullptr, nullptr);
            spriteBatch_->Draw(gradient_, Rectangle(60, 60, 300, 300), Color::White);
            const int x = 420 + static_cast<int>(120.0f * (1.0f + std::sin(time_ * 2.0f)));
            spriteBatch_->Draw(gradient_, Rectangle(x, 140, 120, 120), Color::Orange);
            spriteBatch_->DrawString(*font_, "ASCII", Vector2(480.0f, 320.0f), Color::White, 0.0f, Vector2::Zero, 3.0f,
                                     SpriteEffects::None, 0.0f);
            spriteBatch_->End();

            // 2. Back to the screen: the effect reads the finished picture and draws it as glyphs.
            device.SetRenderTarget(nullptr);
            device.Clear(Color::Black);
            // The convenience overload fills the viewport in colour. Draw the same source over
            // the right half with the rectangle overload to compare monochrome quantization.
            ascii_->setQuantizeMode(CNA::Graphics::AsciiQuantizeMode::Color);
            ascii_->Draw(*scene_);
            const auto& viewport = device.getViewportProperty();
            ascii_->setQuantizeMode(CNA::Graphics::AsciiQuantizeMode::BlackWhite);
            ascii_->Draw(*scene_, Rectangle(viewport.getWidthProperty() / 2, 0,
                                            viewport.getWidthProperty() - viewport.getWidthProperty() / 2,
                                            viewport.getHeightProperty()));

            int columns = 0;
            int rows = 0;
            ascii_->GetLastGridDimensions(columns, rows);
            if (!checked_)
            {
                checked_ = true;
                Context().Print("grid: {} columns x {} rows", columns, rows);
                Context().Check(columns == scene_->getWidthProperty() / 8 && rows == scene_->getHeightProperty() / 12,
                                "one cell per 8x12 source pixels");
            }
            ascii_->setQuantizeMode(CNA::Graphics::AsciiQuantizeMode::Color);
            DemoGame::Draw(gameTime);
        }

    private:
        float time_ = 0.0f;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        Texture2D gradient_;
        std::unique_ptr<RenderTarget2D> scene_;
        std::unique_ptr<CNA::Graphics::AsciiPostProcessEffect> ascii_;
    };
}

CNA_DEMOS_GAME(AsciiSceneDemo, {
    .id = "cnaext.ascii_post_process.scene",
    .title = "ASCII post-process (CNAEXT)",
    .description = "Draws a scene into a RenderTarget2D and redraws it with CNA's AsciiPostProcessEffect: colour "
                   "across the viewport and monochrome in the right half, one glyph cell per 8x12 source pixels. "
                   "It reads the frame back on the CPU, so it works on every renderer.",
    .apis = {"CNA::Graphics::AsciiPostProcessEffect::AsciiPostProcessEffect", "CNA::Graphics::AsciiPostProcessEffect::Draw",
             "CNA::Graphics::AsciiPostProcessEffect::setCellSize", "CNA::Graphics::AsciiPostProcessEffect::getCellSize",
             "CNA::Graphics::AsciiPostProcessEffect::setQuantizeMode",
             "CNA::Graphics::AsciiPostProcessEffect::getQuantizeMode",
             "CNA::Graphics::AsciiPostProcessEffect::GetLastGridDimensions", "CNA::Graphics::AsciiQuantizeMode"},
    .tags = {"cnaext", "post-process", "ascii", "retro", "render-target"},
    .frames = 10,
    .assets = {"fonts/pixel_font.cnb", "textures/gradient.png"},
})
