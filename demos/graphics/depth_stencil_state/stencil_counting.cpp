// SPDX-License-Identifier: MS-PL
// StencilOperation::Increment: count how many shapes cover each pixel, then colour by the count.
#include <array>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/ClearOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/ColorWriteChannels.hpp"
#include "Microsoft/Xna/Framework/Graphics/CompareFunction.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/StencilOperation.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const std::array<Color, 4> Heat = {Color::Black, Color::SeaGreen, Color::Gold, Color::Crimson};

    class StencilCountingDemo final : public CnaDemos::DemoGame
    {
    public:
        StencilCountingDemo() { Graphics().setPreferredDepthStencilFormatProperty(DepthFormat::Depth24Stencil8); }

    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // Counting pass: every pixel a shape covers gets its stencil value incremented.
            count_.setStencilEnableProperty(true);
            count_.setStencilFunctionProperty(CompareFunction::Always);
            count_.setStencilPassProperty(StencilOperation::Increment);
            count_.setDepthBufferEnableProperty(false);
            noColor_.setColorWriteChannelsProperty(ColorWriteChannels::None);

            // Showing pass: one state per count, drawing only where the stencil equals it.
            for (int k = 1; k <= 3; ++k)
            {
                DepthStencilState& show = show_[static_cast<std::size_t>(k - 1)];
                show.setStencilEnableProperty(true);
                show.setStencilFunctionProperty(CompareFunction::Equal);
                show.setReferenceStencilProperty(k);
                show.setStencilPassProperty(StencilOperation::Keep);
                show.setDepthBufferEnableProperty(false);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(ClearOptions::Target | ClearOptions::DepthBuffer | ClearOptions::Stencil, Heat[0], 1.0f, 0);

            spriteBatch_->Begin(SpriteSortMode::Deferred, &noColor_, nullptr, &count_, nullptr);
            spriteBatch_->Draw(*pixel_, Rectangle(150, 80, 300, 220), Color::White);
            spriteBatch_->Draw(*pixel_, Rectangle(330, 160, 300, 220), Color::White);
            spriteBatch_->Draw(*pixel_, Rectangle(250, 230, 200, 200), Color::White);
            spriteBatch_->End();

            for (int k = 1; k <= 3; ++k)
            {
                spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, nullptr, &show_[static_cast<std::size_t>(k - 1)],
                                    nullptr);
                spriteBatch_->Draw(*pixel_, Rectangle(0, 0, 800, 480), Heat[static_cast<std::size_t>(k)]);
                spriteBatch_->End();
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                Context().Check(p.At(50, 50) == Heat[0], "no shape: count 0");
                Context().Check(p.At(200, 120) == Heat[1], "one shape: count 1");
                Context().Check(p.At(400, 200) == Heat[2], "two shapes: count 2");
                Context().Check(p.At(400, 260) == Heat[3], "three shapes: count 3");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
        DepthStencilState count_;
        std::array<DepthStencilState, 3> show_;
        BlendState noColor_;
    };
}

CNA_DEMOS_GAME(StencilCountingDemo, {
    .id = "graphics.depth_stencil_state.stencil_counting",
    .title = "Counting overlaps with the stencil buffer",
    .description = "Increments the stencil buffer under three overlapping rectangles, then colours each pixel by how "
                   "many covered it, with one CompareFunction::Equal pass per count.",
    .apis = {"StencilOperation::Increment", "StencilOperation::Keep", "DepthStencilState::StencilPass",
             "DepthStencilState::StencilFunction", "DepthStencilState::ReferenceStencil", "CompareFunction::Equal",
             "CompareFunction::Always"},
    .tags = {"stencil", "overlap", "count", "state", "2d"},
    .needs = CnaDemos::Need::StencilBuffer,
    .frames = 3,
    .deterministic = true,
})
