// SPDX-License-Identifier: MS-PL
// A 2D camera: the transformMatrix argument of SpriteBatch::Begin.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/CullMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Rectangle ClipArea(180, 100, 440, 280);

    class TransformMatrixDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");
            solid_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            solid_->SetData(&white, 1);
            scissorState_ = std::make_unique<RasterizerState>();
            scissorState_->setCullModeProperty(CullMode::None);
            scissorState_->setScissorTestEnableProperty(true);
        }

        void Update(GameTime& gameTime) override
        {
            // The camera pans right and zooms in over time.
            const float t = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
            cameraX_ = t * 60.0f;
            zoom_ = 1.0f + t * 0.5f;
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);

            // World positions stay fixed; the matrix maps them to the screen. Move the world
            // opposite to the camera, zoom, then centre on the screen.
            const Matrix camera = FramesDrawn() == 0
                ? Matrix::CreateScale(2.0f) * Matrix::CreateTranslation(100.0f, 60.0f, 0.0f)
                : Matrix::CreateTranslation(-cameraX_ - 400.0f, -240.0f, 0.0f)
                    * Matrix::CreateScale(zoom_) * Matrix::CreateTranslation(400.0f, 240.0f, 0.0f);

            // Scissor coordinates stay in the back buffer after the world-space vertices have
            // been transformed. On the first frame the red world rectangle maps to
            // [200,680)x[140,460), whose right and bottom edges are clipped by ClipArea.
            device.setScissorRectangleProperty(ClipArea);

            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque,
                                &SamplerState::PointClamp, nullptr, scissorState_.get(), nullptr,
                                camera);
            spriteBatch_->Draw(*solid_, Rectangle(50, 40, 240, 160), Color::Red);
            for (int i = 0; i < 6; ++i)
            {
                spriteBatch_->Draw(checker_, Rectangle(i * 160, 200, 64, 64), Color::White);
            }
            spriteBatch_->Draw(arrow_, Vector2(400.0f, 300.0f), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(pixels.At(200, 140) == Color::Red,
                                "the matrix maps the world rectangle's top-left pixel to (200,140)");
                Context().Check(pixels.At(199, 150) == Color::DarkSlateGray &&
                                    pixels.At(210, 139) == Color::DarkSlateGray,
                                "pixels inside the scissor but before the transformed geometry remain clear");
                Context().Check(pixels.At(619, 379) == Color::Red,
                                "the last pixel inside both the transformed rectangle and scissor is drawn");
                Context().Check(pixels.At(620, 379) == Color::DarkSlateGray &&
                                    pixels.At(619, 380) == Color::DarkSlateGray,
                                "the scissor clips transformed geometry at its exclusive right and bottom edges");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        Texture2D arrow_;
        std::unique_ptr<Texture2D> solid_;
        std::unique_ptr<RasterizerState> scissorState_;
        float cameraX_ = 0.0f;
        float zoom_ = 1.0f;
    };
}

CNA_DEMOS_GAME(TransformMatrixDemo, {
    .id = "graphics.sprite_batch.transform_matrix",
    .title = "2D camera with a transform matrix",
    .description = "Pans and zooms a 2D scene by passing a camera matrix to SpriteBatch::Begin, then proves that "
                   "a framebuffer-space scissor clips the transformed world geometry at exact pixel edges.",
    .apis = {"SpriteBatch::Begin", "Matrix::CreateTranslation", "Matrix::CreateScale",
             "GraphicsDevice::ScissorRectangle", "RasterizerState::ScissorTestEnable"},
    .tags = {"sprite", "camera", "zoom", "scrolling", "clipping", "2d"},
    .frames = 30,
    .assets = {"textures/checker.png", "textures/arrow.png"},
    .restrictions = "The first frame is checked by programmatic back-buffer readback. Later camera frames are time-dependent; "
                    "no screenshot or whole-animation visual validation is claimed.",
})
