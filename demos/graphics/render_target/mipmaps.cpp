// SPDX-License-Identifier: MS-PL
// A mipmapped RenderTarget2D: its smaller levels are generated automatically after rendering.
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class RenderTargetMipmapsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
            // mipMap = true: 256 -> 128 -> ... -> 1, nine levels.
            target_ = std::make_unique<RenderTarget2D>(device, 256, 256, true, SurfaceFormat::Color, DepthFormat::None);
            Context().Print("LevelCount = {}", target_->getLevelCountProperty());
            Context().Check(target_->getLevelCountProperty() == 9, "a 256x256 mipmapped target has nine levels");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // Render a fine black-and-white checkerboard into level 0 only...
            device.SetRenderTarget(target_.get());
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointWrap, nullptr, nullptr);
            spriteBatch_->Draw(checker_, Rectangle(0, 0, 256, 256), Rectangle(0, 0, 256, 256), Color::White);
            spriteBatch_->End();
            // ...and unbinding generates the other levels from it.
            device.SetRenderTarget(nullptr);

            if (FramesDrawn() == 0)
            {
                // The checker's cells are 8x8 pixels. Level 3 (32x32) still has one texel per cell;
                // at level 4 (16x16) each texel covers four cells, two of each colour, and holds
                // their average.
                std::vector<Color> level4(16 * 16);
                target_->GetData(4, nullptr, level4.data(), 0, static_cast<int>(level4.size()));
                const Color texel = level4[static_cast<std::size_t>(5) * 16 + 5];
                Context().Print("a level-4 texel: {}", texel.ToString());
                const Color expected = Color::Lerp(Color::White, Color(72, 61, 139), 0.5f);
                Context().Check(std::abs(texel.getRProperty() - expected.getRProperty()) <= 4 &&
                                    std::abs(texel.getBProperty() - expected.getBProperty()) <= 4,
                                "level 4 holds the average of the checker's two colours");
            }

            // Drawn small with linear filtering, the mip levels give a smooth grey instead of moire.
            device.Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::LinearClamp, nullptr, nullptr);
            int x = 20;
            for (int size = 256; size >= 16; size /= 2)
            {
                spriteBatch_->Draw(*target_, Rectangle(x, 112, size, size), Color::White);
                x += size + 20;
            }
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        std::unique_ptr<RenderTarget2D> target_;
    };
}

CNA_DEMOS_GAME(RenderTargetMipmapsDemo, {
    .id = "graphics.render_target.mipmaps",
    .title = "Mipmapped render target",
    .description = "Renders a checkerboard into a RenderTarget2D created with mipMap = true; unbinding it generates "
                   "the smaller levels, which hold the averaged colour.",
    .apis = {"RenderTarget2D::RenderTarget2D", "Texture::LevelCount", "Texture2D::GetData"},
    .tags = {"render-target", "mipmap", "offscreen", "filtering"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
