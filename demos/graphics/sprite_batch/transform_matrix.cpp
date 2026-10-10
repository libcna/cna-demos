// SPDX-License-Identifier: MS-PL
// A 2D camera: the transformMatrix argument of SpriteBatch::Begin.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
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
    class TransformMatrixDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");
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
            getGraphicsDeviceProperty().Clear(Color::CornflowerBlue);

            // World positions stay fixed; the matrix maps them to the screen. Move the world
            // opposite to the camera, zoom, then centre on the screen.
            const Matrix camera = Matrix::CreateTranslation(-cameraX_ - 400.0f, -240.0f, 0.0f)
                                  * Matrix::CreateScale(zoom_) * Matrix::CreateTranslation(400.0f, 240.0f, 0.0f);

            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::AlphaBlend, &SamplerState::PointClamp, nullptr,
                                nullptr, nullptr, camera);
            for (int i = 0; i < 6; ++i)
            {
                spriteBatch_->Draw(checker_, Rectangle(i * 160, 200, 64, 64), Color::White);
            }
            spriteBatch_->Draw(arrow_, Vector2(400.0f, 300.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        Texture2D arrow_;
        float cameraX_ = 0.0f;
        float zoom_ = 1.0f;
    };
}

CNA_DEMOS_GAME(TransformMatrixDemo, {
    .id = "graphics.sprite_batch.transform_matrix",
    .title = "2D camera with a transform matrix",
    .description = "Pans and zooms a 2D scene by passing a camera matrix to SpriteBatch::Begin; sprites keep their "
                   "world positions.",
    .apis = {"SpriteBatch::Begin", "Matrix::CreateTranslation", "Matrix::CreateScale"},
    .tags = {"sprite", "camera", "zoom", "scrolling", "2d"},
    .frames = 30,
    .assets = {"textures/checker.png", "textures/arrow.png"},
})
