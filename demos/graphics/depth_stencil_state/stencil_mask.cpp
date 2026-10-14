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

            // The buffer starts at binary 10. Each failure path replaces only bit 0 with the
            // reference's low bit, producing binary 11 while preserving bit 1.
            stencilFailMask_.setStencilEnableProperty(true);
            stencilFailMask_.setStencilFunctionProperty(CompareFunction::Never);
            stencilFailMask_.setStencilFailProperty(StencilOperation::Replace);
            stencilFailMask_.setReferenceStencilProperty(1);
            stencilFailMask_.setStencilWriteMaskProperty(0x01);
            stencilFailMask_.setDepthBufferEnableProperty(false);

            depthFailMask_.setStencilEnableProperty(true);
            depthFailMask_.setStencilFunctionProperty(CompareFunction::Always);
            depthFailMask_.setStencilDepthBufferFailProperty(StencilOperation::Replace);
            depthFailMask_.setReferenceStencilProperty(1);
            depthFailMask_.setStencilWriteMaskProperty(0x01);
            depthFailMask_.setDepthBufferEnableProperty(true);
            depthFailMask_.setDepthBufferWriteEnableProperty(false);
            depthFailMask_.setDepthBufferFunctionProperty(CompareFunction::Never);
            noColor_.setColorWriteChannelsProperty(ColorWriteChannels::None);

            // An exact comparison first proves the unwritable high bit survived.
            exactMask_.setStencilEnableProperty(true);
            exactMask_.setStencilFunctionProperty(CompareFunction::Equal);
            exactMask_.setReferenceStencilProperty(3);
            exactMask_.setDepthBufferEnableProperty(false);

            // The visible pass compares only bit 0: stored 3 matches reference 1 through mask 1.
            insideMask_.setStencilEnableProperty(true);
            insideMask_.setStencilFunctionProperty(CompareFunction::Equal);
            insideMask_.setStencilPassProperty(StencilOperation::Keep);
            insideMask_.setReferenceStencilProperty(1);
            insideMask_.setStencilMaskProperty(0x01);
            insideMask_.setDepthBufferEnableProperty(false);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(ClearOptions::Target | ClearOptions::DepthBuffer | ClearOptions::Stencil, Color::DarkSlateGray,
                         1.0f, 2);

            // The vertical arm takes the stencil-test-fail path; the horizontal arm takes the
            // depth-test-fail path. Both update stencil without writing colour.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &noColor_, nullptr, &stencilFailMask_, nullptr);
            spriteBatch_->Draw(*pixel_, Rectangle(330, 60, 140, 360), Color::White);
            spriteBatch_->End();
            spriteBatch_->Begin(SpriteSortMode::Deferred, &noColor_, nullptr, &depthFailMask_, nullptr);
            spriteBatch_->Draw(*pixel_, Rectangle(190, 170, 420, 140), Color::White);
            spriteBatch_->End();

            // Stored 3 is observable with a full-mask comparison. Clear only colour/depth after
            // this probe, retaining the stencil values for the masked comparison below.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, nullptr, &exactMask_, nullptr);
            spriteBatch_->Draw(*pixel_, Rectangle(0, 0, 800, 480), Color::White);
            spriteBatch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels exact(device);
                Context().Check(exact.At(400, 240) == Color::White && exact.At(100, 100) == Color::DarkSlateGray,
                                "StencilWriteMask changes bit 0 while preserving the initial high bit");
            }
            device.Clear(ClearOptions::Target | ClearOptions::DepthBuffer, Color::DarkSlateGray, 1.0f, 0);

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
        DepthStencilState stencilFailMask_;
        DepthStencilState depthFailMask_;
        DepthStencilState exactMask_;
        DepthStencilState insideMask_;
        BlendState noColor_;
    };
}

CNA_DEMOS_GAME(StencilMaskDemo, {
    .id = "graphics.depth_stencil_state.stencil_mask",
    .title = "Stencil masking",
    .description = "Writes a plus-shaped mask through stencil-fail and depth-fail paths while preserving masked-out "
                   "bits, then uses a read mask to show a texture only inside the shape.",
    .apis = {"DepthStencilState::StencilEnable", "DepthStencilState::StencilFunction", "DepthStencilState::StencilPass",
             "DepthStencilState::StencilFail", "DepthStencilState::StencilDepthBufferFail",
             "DepthStencilState::StencilMask", "DepthStencilState::StencilWriteMask",
             "DepthStencilState::ReferenceStencil", "StencilOperation::Replace", "CompareFunction::Equal",
             "ClearOptions::Stencil", "DepthFormat::Depth24Stencil8"},
    .tags = {"stencil", "mask", "state", "2d"},
    .needs = CnaDemos::Need::StencilBuffer,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
