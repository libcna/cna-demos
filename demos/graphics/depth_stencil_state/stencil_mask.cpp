// SPDX-License-Identifier: MS-PL
// Stencil masking in 2D: write a shape into the stencil buffer, then draw only where it was written.
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
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/StencilOperation.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class StencilMaskDemo final : public CnaDemos::DemoGame
    {
    public:
        StencilMaskDemo()
        {
            // The back buffer needs stencil bits.
            Graphics().setPreferredDepthStencilFormatProperty(DepthFormat::Depth24Stencil8);
        }

    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);

            // Pass 1 writes 1 into the stencil buffer wherever it draws, and no colour at all.
            writeMask_.setStencilEnableProperty(true);
            writeMask_.setStencilFunctionProperty(CompareFunction::Always);
            writeMask_.setStencilPassProperty(StencilOperation::Replace);
            writeMask_.setReferenceStencilProperty(1);
            writeMask_.setDepthBufferEnableProperty(false);
            noColor_.setColorWriteChannelsProperty(ColorWriteChannels::None);

            // Pass 2 draws only where the stencil buffer holds 1.
            insideMask_.setStencilEnableProperty(true);
            insideMask_.setStencilFunctionProperty(CompareFunction::Equal);
            insideMask_.setStencilPassProperty(StencilOperation::Keep);
            insideMask_.setReferenceStencilProperty(1);
            insideMask_.setDepthBufferEnableProperty(false);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(ClearOptions::Target | ClearOptions::DepthBuffer | ClearOptions::Stencil, Color::DarkSlateGray,
                         1.0f, 0);

            // The mask: a plus sign made of two rectangles.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &noColor_, nullptr, &writeMask_, nullptr);
            spriteBatch_->Draw(*pixel_, Rectangle(330, 60, 140, 360), Color::White);
            spriteBatch_->Draw(*pixel_, Rectangle(190, 170, 420, 140), Color::White);
            spriteBatch_->End();

            // The content: a full-window texture, visible only through the mask.
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointWrap, &insideMask_, nullptr);
            spriteBatch_->Draw(checker_, Rectangle(0, 0, 800, 480), Rectangle(0, 0, 800, 480), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(pixels.At(100, 100) == Color::DarkSlateGray, "outside the mask nothing is drawn");
                Context().Check(pixels.At(400, 240) != Color::DarkSlateGray, "inside the mask the texture is drawn");
                Context().Check(pixels.At(250, 100) == Color::DarkSlateGray,
                                "the mask's own colour was never written (ColorWriteChannels::None)");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        std::unique_ptr<Texture2D> pixel_;
        DepthStencilState writeMask_;
        DepthStencilState insideMask_;
        BlendState noColor_;
    };
}

CNA_DEMOS_GAME(StencilMaskDemo, {
    .id = "graphics.depth_stencil_state.stencil_mask",
    .title = "Stencil masking",
    .description = "Writes a plus-shaped mask into the stencil buffer (no colour), then draws a full-window texture "
                   "with a stencil test so it shows only inside the mask.",
    .apis = {"DepthStencilState::StencilEnable", "DepthStencilState::StencilFunction", "DepthStencilState::StencilPass",
             "DepthStencilState::ReferenceStencil", "StencilOperation::Replace", "CompareFunction::Equal",
             "ClearOptions::Stencil", "DepthFormat::Depth24Stencil8"},
    .tags = {"stencil", "mask", "state", "2d"},
    .needs = CnaDemos::Need::StencilBuffer,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
