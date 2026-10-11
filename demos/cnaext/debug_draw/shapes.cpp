// SPDX-License-Identifier: MS-PL
// CNAEXT DebugDraw: lines, boxes, spheres, frusta and crosses for seeing what the game computes.
#include <cmath>
#include <memory>
#include <optional>
#include <string>

#include "CNA/Graphics/DebugDraw.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DebugDrawDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            debug_ = std::make_unique<CNA::Graphics::DebugDraw>(getGraphicsDeviceProperty());
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            angle_ += 0.3f * static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(20, 22, 30));
            const float aspect = device.getViewportProperty().getAspectRatioProperty();
            const Matrix view = Matrix::CreateLookAt(Vector3(std::sin(angle_) * 9.0f, 5.0f, std::cos(angle_) * 9.0f),
                                                     Vector3::Zero, Vector3::Up);
            const Matrix projection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, aspect, 0.1f, 100.0f);

            // Shapes are collected between begin and end and drawn in one go (one draw call per
            // depth mode), so it costs little to leave debug drawing in a development build.
            debug_->begin(view, projection);
            for (int i = -4; i <= 4; ++i) // a floor grid
            {
                debug_->addLine(Vector3(static_cast<float>(i), 0.0f, -4.0f), Vector3(static_cast<float>(i), 0.0f, 4.0f),
                                Color::DimGray);
                debug_->addLine(Vector3(-4.0f, 0.0f, static_cast<float>(i)), Vector3(4.0f, 0.0f, static_cast<float>(i)),
                                Color::DimGray);
            }
            debug_->addBox(BoundingBox(Vector3(-3.0f, 0.0f, -1.0f), Vector3(-1.0f, 2.0f, 1.0f)), Color::Orange);
            debug_->addSphere(BoundingSphere(Vector3(1.5f, 1.0f, 0.0f), 1.0f), Color::DeepSkyBlue);
            // A spot light's view volume, as a frustum.
            const BoundingFrustum light(Matrix::CreateLookAt(Vector3(0.0f, 3.0f, 3.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3::Up) *
                                        Matrix::CreatePerspectiveFieldOfView(0.6f, 1.0f, 0.5f, 3.0f));
            debug_->addFrustum(light, Color::Yellow);
            // Overlay shapes are drawn through geometry: a target marker always visible.
            debug_->setDepthTested(false);
            debug_->addCross(Vector3(0.0f, 0.5f, -2.5f), 0.6f, Color::Magenta);
            debug_->setDepthTested(true);
            lines_ = debug_->getLineCount();
            debug_->end();

            if (!checked_)
            {
                checked_ = true;
                Context().Print("{} lines this frame", lines_);
                // Grid 18 + box 12 + frustum 12 + cross 3 = 45, plus the sphere's circles.
                Context().Check(lines_ > 45, "the shapes were turned into line segments");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "DebugDraw: " + std::to_string(lines_) + " lines", Vector2(20.0f, 16.0f),
                                     Color::White);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        float angle_ = 0.6f;
        int lines_ = 0;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<CNA::Graphics::DebugDraw> debug_;
    };
}

CNA_DEMOS_GAME(DebugDrawDemo, {
    .id = "cnaext.debug_draw.shapes",
    .title = "Debug drawing (CNAEXT)",
    .description = "Draws a floor grid, a bounding box, a bounding sphere and a light's frustum with CNA's DebugDraw "
                   "in one batch, plus an overlay cross that shows through geometry (setDepthTested false).",
    .apis = {"CNA::Graphics::DebugDraw::DebugDraw", "CNA::Graphics::DebugDraw::begin", "CNA::Graphics::DebugDraw::end",
             "CNA::Graphics::DebugDraw::addLine", "CNA::Graphics::DebugDraw::addBox",
             "CNA::Graphics::DebugDraw::addSphere", "CNA::Graphics::DebugDraw::addFrustum",
             "CNA::Graphics::DebugDraw::addCross", "CNA::Graphics::DebugDraw::setDepthTested",
             "CNA::Graphics::DebugDraw::getLineCount"},
    .tags = {"cnaext", "debug", "lines", "visualization", "3d", "tools"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
})
