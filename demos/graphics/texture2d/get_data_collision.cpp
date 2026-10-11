// SPDX-License-Identifier: MS-PL
// Texture2D::GetData for per-pixel collision: read a sprite's alpha once, test overlaps on the CPU.
#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class GetDataCollisionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            circle_ = getContentProperty().Load<Texture2D>("textures/soft_circle");

            // Copy the texels to the CPU once, at load time; GetData every frame would stall.
            pixels_.resize(static_cast<std::size_t>(circle_.getWidthProperty()) * circle_.getHeightProperty());
            circle_.GetData(pixels_.data(), static_cast<int>(pixels_.size()));
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);

            // Two pairs of circles. Both pairs' bounding rectangles overlap; only the second pair's
            // visible pixels do.
            const Vector2 pairs[2][2] = {{Vector2(140.0f, 160.0f), Vector2(186.0f, 206.0f)},
                                         {Vector2(480.0f, 180.0f), Vector2(520.0f, 190.0f)}};
            // soft_circle.png is a loose PNG with straight alpha, so it is blended NonPremultiplied
            // (see graphics.sprite_batch.loose_png_alpha).
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::NonPremultiplied, nullptr, nullptr, nullptr);
            for (int i = 0; i < 2; ++i)
            {
                const bool boxes = Bounds(pairs[i][0]).Intersects(Bounds(pairs[i][1]));
                const bool hit = PixelsCollide(pairs[i][0], pairs[i][1]);
                const Color tint = hit ? Color::OrangeRed : Color::LightGreen;
                spriteBatch_->Draw(circle_, pairs[i][0], tint);
                spriteBatch_->Draw(circle_, pairs[i][1], tint);
                if (FramesDrawn() == 0)
                {
                    Context().Print("pair {}: rectangles overlap {}, pixels overlap {}", i + 1, boxes, hit);
                    Context().Check(boxes, "the bounding rectangles overlap");
                    Context().Check(hit == (i == 1), i == 0 ? "the first pair's opaque pixels do not touch"
                                                            : "the second pair's opaque pixels overlap");
                }
            }
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        [[nodiscard]] Rectangle Bounds(Vector2 position) const
        {
            return Rectangle(static_cast<int>(position.X), static_cast<int>(position.Y), circle_.getWidthProperty(),
                             circle_.getHeightProperty());
        }

        // The classic XNA per-pixel test: walk the overlap of the two rectangles and look for a
        // pixel that is solid in both sprites.
        [[nodiscard]] bool PixelsCollide(Vector2 a, Vector2 b) const
        {
            const Rectangle ra = Bounds(a);
            const Rectangle rb = Bounds(b);
            const int top = std::max(ra.getTopProperty(), rb.getTopProperty());
            const int bottom = std::min(ra.getBottomProperty(), rb.getBottomProperty());
            const int left = std::max(ra.getLeftProperty(), rb.getLeftProperty());
            const int right = std::min(ra.getRightProperty(), rb.getRightProperty());
            const int width = circle_.getWidthProperty();
            for (int y = top; y < bottom; ++y)
            {
                for (int x = left; x < right; ++x)
                {
                    const Color pa = pixels_[static_cast<std::size_t>(y - ra.Y) * width + (x - ra.X)];
                    const Color pb = pixels_[static_cast<std::size_t>(y - rb.Y) * width + (x - rb.X)];
                    if (pa.getAProperty() > 128 && pb.getAProperty() > 128)
                        return true;
                }
            }
            return false;
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D circle_;
        std::vector<Color> pixels_;
    };
}

CNA_DEMOS_GAME(GetDataCollisionDemo, {
    .id = "graphics.texture2d.get_data_collision",
    .title = "Per-pixel collision",
    .description = "Copies a sprite's texels to the CPU with Texture2D::GetData and uses their alpha to tell real "
                   "overlaps from merely overlapping bounding rectangles.",
    .apis = {"Texture2D::GetData", "Rectangle::Intersects", "Rectangle::Left", "Rectangle::Right", "Rectangle::Top",
             "Rectangle::Bottom", "Texture2D::Width", "Texture2D::Height"},
    .tags = {"texture", "collision", "pixels", "2d"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/soft_circle.png"},
})
