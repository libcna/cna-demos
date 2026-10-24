// SPDX-License-Identifier: MS-PL
// Rendering into a RenderTarget2D, then drawing that render target as a texture.
#include <memory>
#include <optional>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentInterval.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class RenderTarget2DDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");

            CheckMoveIdentity(device);
            CheckBoundDestruction(device);
            CheckCrossDeviceMove(device);

            // A 128x128 off-screen colour buffer. RenderTarget2D derives from Texture2D, so once
            // rendered it can be drawn like any texture.
            target_ = std::make_unique<RenderTarget2D>(device, 128, 128);
            target_->ContentLost += [this](System::Object*, const System::EventArgs&) { ++contentLost_; };
            target_->NotifyContentLostEXT();
            Context().Check(target_->getIsContentLostProperty() && contentLost_ == 1,
                            "a content-loss notification raises ContentLost and marks the target lost");
        }

        void UnloadContent() override
        {
            // GPU resources are released explicitly with Dispose(), as in XNA.
            target_->Dispose();
            Context().Check(target_->getIsDisposedProperty(), "the render target reports itself disposed");
        }

        void Draw(const GameTime& gameTime) override
        {
            auto& device = getGraphicsDeviceProperty();

            // 1. Draw into the render target.
            device.SetRenderTarget(target_.get());
            if (!checkedRestore_)
            {
                checkedRestore_ = true;
                Context().Check(!target_->getIsContentLostProperty(),
                                "binding the target for new rendering clears its lost-content state");
            }
            device.Clear(Color::DarkSlateGray);
            spriteBatch_->Begin();
            spriteBatch_->Draw(arrow_, Rectangle(32, 32, 64, 64), Color::White);
            spriteBatch_->End();

            // 2. Back to the back buffer, then draw the render target's contents four times.
            device.SetRenderTarget(nullptr);
            device.Clear(Color::CornflowerBlue);
            spriteBatch_->Begin();
            spriteBatch_->Draw(*target_, Rectangle(40, 40, 128, 128), Color::White);
            spriteBatch_->Draw(*target_, Rectangle(200, 40, 256, 256), Color::White);
            spriteBatch_->Draw(*target_, Rectangle(480, 40, 64, 64), Color::White);
            spriteBatch_->Draw(*target_, Rectangle(480, 120, 64, 64), Color::Orange);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        void CheckMoveIdentity(GraphicsDevice& device)
        {
            RenderTarget2D destination(device, 32, 16, false, SurfaceFormat::Color,
                                       DepthFormat::Depth24Stencil8, 0,
                                       RenderTargetUsage::DiscardContents);
            RenderTarget2D source(device, 128, 128, false, SurfaceFormat::Color,
                                  DepthFormat::None, 0, RenderTargetUsage::PreserveContents);

            device.SetRenderTarget(&destination);
            device.Clear(Color::Gold);
            bool disposeRefused = false;
            try { destination.Dispose(); }
            catch (const System::InvalidOperationException&) { disposeRefused = true; }
            const auto disposeBindings = device.GetRenderTargets();
            Context().Check(disposeRefused && !destination.getIsDisposedProperty() &&
                                disposeBindings.size() == 1 &&
                                disposeBindings[0].getRenderTargetProperty() == &destination,
                            "disposing a bound target is refused without detaching or invalidating it");

            bool refused = false;
            try { destination = std::move(source); }
            catch (const System::InvalidOperationException&) { refused = true; }
            const auto refusedBindings = device.GetRenderTargets();
            Context().Check(refused && refusedBindings.size() == 1 &&
                                refusedBindings[0].getRenderTargetProperty() == &destination &&
                                destination.getWidthProperty() == 32 &&
                                destination.getHeightProperty() == 16 &&
                                destination.getDepthStencilFormatProperty() == DepthFormat::Depth24Stencil8 &&
                                destination.getRenderTargetUsageProperty() == RenderTargetUsage::DiscardContents &&
                                source.getWidthProperty() == 128 && source.getHeightProperty() == 128 &&
                                !source.getIsDisposedProperty(),
                            "move assignment refuses to overwrite a bound target without changing either wrapper");

            device.SetRenderTarget(nullptr);
            const Rectangle origin(0, 0, 1, 1);
            Color pixel = Color::Black;
            destination.GetData(0, &origin, &pixel, 0, 1);
            Context().Check(pixel == Color::Gold,
                            "refused move assignment preserves the bound target's rendered content");

            device.SetRenderTarget(&source);
            device.Clear(Color::LimeGreen);
            RenderTarget2D moved(std::move(source));
            const auto constructedBindings = device.GetRenderTargets();
            Context().Check(source.getIsDisposedProperty() && constructedBindings.size() == 1 &&
                                constructedBindings[0].getRenderTargetProperty() == &moved &&
                                moved.getWidthProperty() == 128 && moved.getHeightProperty() == 128 &&
                                moved.getRenderTargetUsageProperty() == RenderTargetUsage::PreserveContents,
                            "move construction transfers the active render-target binding and properties");

            destination = std::move(moved);
            const auto assignedBindings = device.GetRenderTargets();
            Context().Check(moved.getIsDisposedProperty() && assignedBindings.size() == 1 &&
                                assignedBindings[0].getRenderTargetProperty() == &destination &&
                                destination.getWidthProperty() == 128 &&
                                destination.getHeightProperty() == 128 &&
                                destination.getDepthStencilFormatProperty() == DepthFormat::None &&
                                destination.getRenderTargetUsageProperty() == RenderTargetUsage::PreserveContents,
                            "move assignment from a bound source transfers its binding and complete target shape");

            device.Clear(Color::Orange);
            device.SetRenderTarget(nullptr);
            pixel = Color::Black;
            destination.GetData(0, &origin, &pixel, 0, 1);
            Context().Check(pixel == Color::Orange,
                            "the moved target remains writable through the transferred active binding");
        }

        void CheckBoundDestruction(GraphicsDevice& device)
        {
            const Rectangle backBufferBounds = device.getViewportProperty().getBoundsProperty();
            std::optional<RenderTarget2D> slot;
            slot.emplace(device, 24, 12, false, SurfaceFormat::Color, DepthFormat::None, 0,
                         RenderTargetUsage::PreserveContents);
            const RenderTarget2D* const firstAddress = &*slot;
            device.SetRenderTarget(&*slot);
            slot.reset();

            bool presentRefused = false;
            try { device.Present(); }
            catch (const System::InvalidOperationException&) { presentRefused = true; }
            Context().Check(device.GetRenderTargets().empty() && presentRefused,
                            "destroying a bound target removes its dangling public binding while Present stays refused");

            device.SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));
            Context().Check(device.GetRenderTargets().empty() &&
                                device.getViewportProperty().getBoundsProperty() == backBufferBounds,
                            "an explicit unbind after bound destruction restores the complete back-buffer viewport");

            slot.emplace(device, 24, 12, false, SurfaceFormat::Color, DepthFormat::None, 0,
                         RenderTargetUsage::PreserveContents);
            const bool addressReused = &*slot == firstAddress;
            device.SetRenderTarget(&*slot);
            const auto replacementBindings = device.GetRenderTargets();
            Context().Check(addressReused && replacementBindings.size() == 1 &&
                                replacementBindings[0].getRenderTargetProperty() == &*slot &&
                                device.getViewportProperty().getBoundsProperty() == Rectangle(0, 0, 24, 12),
                            "a replacement at the destroyed target's address is genuinely rebound");

            device.Clear(Color::CornflowerBlue);
            device.SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));
            const Rectangle origin(0, 0, 1, 1);
            Color pixel = Color::Black;
            slot->GetData(0, &origin, &pixel, 0, 1);
            Context().Check(pixel == Color::CornflowerBlue,
                            "the same-address replacement receives rendering through its new binding");
        }

        void CheckCrossDeviceMove(GraphicsDevice& primary)
        {
            PresentationParameters parameters;
            parameters.setBackBufferWidthProperty(32);
            parameters.setBackBufferHeightProperty(32);
            parameters.setIsFullScreenProperty(false);
            parameters.setDepthStencilFormatProperty(DepthFormat::None);
            parameters.setPresentationIntervalProperty(PresentInterval::Immediate);
            secondary_ = std::make_unique<GraphicsDevice>(
                GraphicsAdapter::getDefaultAdapterProperty(), GraphicsProfile::HiDef, parameters);

            RenderTarget2D destination(primary, 8, 4, false, SurfaceFormat::Color,
                                       DepthFormat::Depth16, 0, RenderTargetUsage::DiscardContents);
            primary.getTexturesProperty()(3, &destination);

            RenderTarget2D source(*secondary_, 20, 10, false, SurfaceFormat::Color,
                                  DepthFormat::Depth24Stencil8, 0,
                                  RenderTargetUsage::PreserveContents);
            const Rectangle origin(0, 0, 1, 1);
            const Color lime = Color::LimeGreen;
            secondary_->SetRenderTarget(&source);
            secondary_->Clear(lime);
            secondary_->SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));
            Color pixel = Color::Black;
            source.GetData(0, &origin, &pixel, 0, 1);
            Context().Check(pixel == lime,
                            "a secondary EasyGL device binds its own render target before clearing it");

            // Keep the cross-device move proof independent from the EasyGL context bug above.
            source.SetData(0, &origin, &lime, 0, 1);
            secondary_->SetRenderTarget(&source);

            destination = std::move(source);
            const auto bindings = secondary_->GetRenderTargets();
            Context().Check(source.getIsDisposedProperty() &&
                                destination.getGraphicsDeviceProperty() == secondary_.get() &&
                                destination.getWidthProperty() == 20 &&
                                destination.getHeightProperty() == 10 &&
                                destination.getDepthStencilFormatProperty() == DepthFormat::Depth24Stencil8 &&
                                destination.getRenderTargetUsageProperty() == RenderTargetUsage::PreserveContents &&
                                primary.getTexturesProperty()[3] == nullptr &&
                                bindings.size() == 1 &&
                                bindings[0].getRenderTargetProperty() == &destination,
                            "cross-device move assignment transfers ownership and the active target binding");

            secondary_->SetRenderTarget(static_cast<RenderTarget2D*>(nullptr));
            pixel = Color::Black;
            destination.GetData(0, &origin, &pixel, 0, 1);
            Context().Check(pixel == lime,
                            "cross-device move assignment preserves the source target payload");

            const Color orange = Color::Orange;
            destination.SetData(0, &origin, &orange, 0, 1);
            pixel = Color::Black;
            destination.GetData(0, &origin, &pixel, 0, 1);
            Context().Check(pixel == Color::Orange,
                            "the cross-device moved target remains writable after unbinding");
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<RenderTarget2D> target_;
        Texture2D arrow_;
        int contentLost_ = 0;
        bool checkedRestore_ = false;
        // Keep the second device alive through the primary frame. Its teardown has separate
        // OPENGLES3 coverage under CNA-DEMOS-ISSUE-086.
        std::unique_ptr<GraphicsDevice> secondary_;
    };
}

CNA_DEMOS_GAME(RenderTarget2DDemo, {
    .id = "graphics.render_target.render_target2d",
    .title = "Render to a texture",
    .description = "Checks bound-target move, cross-device ownership and destruction identity, draws a sprite "
                   "into a RenderTarget2D, then draws the target as an ordinary texture at several sizes and "
                   "with a tint.",
    .apis = {"RenderTarget2D::RenderTarget2D", "RenderTarget2D::ContentLost", "RenderTarget2D::IsContentLost",
             "GraphicsDevice::SetRenderTarget", "GraphicsDevice::GetRenderTargets", "GraphicsDevice::Present",
             "GraphicsResource::Dispose",
             "GraphicsResource::IsDisposed"},
    .tags = {"offscreen", "render-to-texture"},
    .frames = 10,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
})
