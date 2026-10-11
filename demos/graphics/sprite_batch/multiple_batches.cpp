// SPDX-License-Identifier: MS-PL
// Several Begin/End pairs per frame: background, camera-transformed world and screen-fixed HUD.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class MultipleBatchesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");
            strip_ = getContentProperty().Load<Texture2D>("textures/sprite_strip");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            // Each Begin/End pair has its own states and transform; a frame may use as many as it needs.
            // 1. Background: opaque, tiled with a wrapping sampler, dimmed.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointWrap, nullptr,
                                nullptr);
            spriteBatch_->Draw(checker_, Vector2::Zero, Rectangle(0, 0, 800, 480), Color::DimGray);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
                Context().Check(device.getBlendStateProperty().getColorDestinationBlendProperty() == Blend::Zero &&
                                device.getSamplerStatesProperty()[0].getAddressUProperty() == TextureAddressMode::Wrap &&
                                device.getSamplerStatesProperty()[0].getFilterProperty() == TextureFilter::Point,
                                "background End leaves its opaque blend and PointWrap sampler installed");

            // 2. World: everything through a camera matrix (scrolled and zoomed 2x), sorted by depth.
            const Matrix camera = Matrix::CreateTranslation(Vector3(-100.0f, -40.0f, 0.0f)) *
                                  Matrix::CreateScale(2.0f);
            spriteBatch_->Begin(SpriteSortMode::BackToFront, &BlendState::AlphaBlend, &SamplerState::PointClamp,
                                nullptr, nullptr, nullptr, camera);
            if (FramesDrawn() == 0)
                Context().Check(device.getBlendStateProperty().getColorDestinationBlendProperty() == Blend::Zero &&
                                device.getSamplerStatesProperty()[0].getAddressUProperty() == TextureAddressMode::Wrap,
                                "a deferred world Begin has not yet replaced the background's device states");
            spriteBatch_->Draw(arrow_, Vector2(140.0f, 80.0f), Color::White);
            spriteBatch_->Draw(arrow_, Vector2(260.0f, 140.0f), Color::LightSkyBlue);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
                Context().Check(device.getBlendStateProperty().getColorDestinationBlendProperty() == Blend::InverseSourceAlpha &&
                                device.getSamplerStatesProperty()[0].getAddressUProperty() == TextureAddressMode::Clamp &&
                                device.getSamplerStatesProperty()[0].getFilterProperty() == TextureFilter::Point &&
                                !device.getDepthStencilStateProperty().getDepthBufferEnableProperty() &&
                                device.getRasterizerStateProperty().getCullModeProperty() == CullMode::CullCounterClockwiseFace,
                                "world End retains its selected states instead of restoring the background's states");

            // 3. HUD: no transform, so it stays put whatever the camera does.
            spriteBatch_->Begin();
            spriteBatch_->Draw(strip_, Vector2(16.0f, 16.0f), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                Context().Check(device.getBlendStateProperty().getColorDestinationBlendProperty() == Blend::InverseSourceAlpha &&
                                device.getSamplerStatesProperty()[0].getFilterProperty() == TextureFilter::Linear &&
                                device.getSamplerStatesProperty()[0].getAddressUProperty() == TextureAddressMode::Clamp,
                                "default HUD End installs AlphaBlend and LinearClamp instead of restoring the world sampler");
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(pixels.At(120, 140) == Color(255, 200, 40),
                                "the world arrow is drawn at the camera-transformed position");
                Context().Check(CnaDemos::Near(pixels.At(23, 26), Color(230, 80, 80), 1),
                                "the HUD strip stays at its screen-space position after the transformed world batch");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        Texture2D arrow_;
        Texture2D strip_;
    };
}

CNA_DEMOS_GAME(MultipleBatchesDemo, {
    .id = "graphics.sprite_batch.multiple_batches",
    .title = "Several sprite batches per frame",
    .description = "Draws a frame in three Begin/End pairs with different states: a tiled opaque background, a "
                   "camera-transformed world and a screen-fixed HUD, with visible placements and device-state retention after End.",
    .apis = {"SpriteBatch::Begin", "SpriteBatch::End", "GraphicsDevice::BlendState", "GraphicsDevice::SamplerStates",
             "GraphicsDevice::DepthStencilState", "GraphicsDevice::RasterizerState"},
    .tags = {"sprite", "layers", "hud", "camera", "2d"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png", "textures/arrow.png", "textures/sprite_strip.png"},
})
