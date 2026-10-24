// SPDX-License-Identifier: MS-PL
// CNAEXT ColorMatrixEffect: recolour sprites with a 4x4 colour matrix and offset (sepia, negative).
#include <algorithm>
#include <array>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ColorMatrixEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // Row-major: out[row] = dot(row, (r, g, b, a)) + offset[row], clamped to [0, 1].
    constexpr std::array<float, 16> Sepia = {0.393f, 0.769f, 0.189f, 0.0f,  //
                                             0.349f, 0.686f, 0.168f, 0.0f,  //
                                             0.272f, 0.534f, 0.131f, 0.0f,  //
                                             0.0f,   0.0f,   0.0f,   1.0f};
    constexpr std::array<float, 16> Negate = {-1.0f, 0.0f, 0.0f, 0.0f,  //
                                              0.0f, -1.0f, 0.0f, 0.0f,  //
                                              0.0f, 0.0f, -1.0f, 0.0f,  //
                                              0.0f, 0.0f, 0.0f, 1.0f};

    class SepiaAndInvertDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            gradient_ = getContentProperty().Load<Texture2D>("textures/gradient");

            sepia_ = std::make_unique<ColorMatrixEffect>(device);
            sepia_->SetColorMatrix(Sepia);
            negative_ = std::make_unique<ColorMatrixEffect>(device);
            negative_->SetColorMatrix(Negate);
            negative_->SetColorOffset(Vector4(1.0f, 1.0f, 1.0f, 0.0f));  // 1 - colour

            // A fixed CPU transform that only some renderers apply; the others refuse it in Begin
            // rather than drawing the sprites untransformed. A renderer that accepts it is held to
            // the pixel checks below.
            try
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::LinearClamp, nullptr,
                                    nullptr, sepia_.get());
                spriteBatch_->End();
            }
            catch (const System::NotSupportedException&)
            {
                Context().Skip("this renderer refuses ColorMatrixEffect (SOFTWARE applies it)");
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            const std::array<ColorMatrixEffect*, 3> effects = {nullptr, sepia_.get(), negative_.get()};
            for (std::size_t i = 0; i < effects.size(); ++i)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::LinearClamp, nullptr,
                                    nullptr, effects[i]);
                spriteBatch_->Draw(gradient_, Rectangle(40 + 250 * static_cast<int>(i), 80, 220, 320), Color::White);
                spriteBatch_->End();
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                const Color original = p.At(200, 120), sepia = p.At(450, 120), negative = p.At(700, 120);
                Context().Print("same texel: original {}, sepia {}, negative {}", original.ToString(), sepia.ToString(),
                                negative.ToString());
                const double r = original.getRProperty() / 255.0, g = original.getGProperty() / 255.0,
                             b = original.getBProperty() / 255.0;
                Context().CheckNear(sepia.getGProperty(), 255.0 * std::min(1.0, 0.349 * r + 0.686 * g + 0.168 * b), 2.0,
                                    "sepia green = 0.349 r + 0.686 g + 0.168 b");
                Context().CheckNear(negative.getRProperty(), 255.0 - original.getRProperty(), 1.0, "negative red = 1 - r");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D gradient_;
        std::unique_ptr<ColorMatrixEffect> sepia_;
        std::unique_ptr<ColorMatrixEffect> negative_;
    };
}

CNA_DEMOS_GAME(SepiaAndInvertDemo, {
    .id = "graphics.color_matrix_effect.sepia_and_invert",
    .title = "Colour matrix for sprites",
    .description = "Draws a texture plain, through a sepia ColorMatrixEffect and through a negative one (matrix -1 "
                   "plus offset 1); CNA's fixed colour transform for SpriteBatch (CNAEXT).",
    .apis = {"ColorMatrixEffect::ColorMatrixEffect", "ColorMatrixEffect::SetColorMatrix",
             "ColorMatrixEffect::SetColorOffset", "SpriteBatch::Begin"},
    .tags = {"sprite", "color", "sepia", "filter", "cnaext"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/gradient.png"},
    .restrictions = "Skipped where the renderer refuses ColorMatrixEffect (NotSupportedException from Begin); today "
                    "SOFTWARE applies it.",
})
