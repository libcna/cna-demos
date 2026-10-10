// SPDX-License-Identifier: MS-PL
// The first 3D triangle: vertex colours, BasicEffect and GraphicsDevice::DrawUserPrimitives.
#include <array>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DrawUserPrimitivesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // BasicEffect is XNA's built-in shader. With VertexColorEnabled it colours each pixel
            // by interpolating the colours of the triangle's corners.
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setWorldProperty(Matrix::getIdentityProperty());
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 3.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // Clockwise as seen by the camera: the side XNA treats as the front.
            vertices_ = {VertexPositionColor(Vector3(0.0f, 1.0f, 0.0f), Color::Red),
                         VertexPositionColor(Vector3(1.0f, -1.0f, 0.0f), Color::Lime),
                         VertexPositionColor(Vector3(-1.0f, -1.0f, 0.0f), Color::Blue)};
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::CornflowerBlue);

            // An effect can have several passes; each one is applied, then the geometry drawn.
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices_.data(), 0, 1);
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const Color centre = pixels.At(400, 260);
                Context().Print("pixel in the middle of the triangle: {}", centre.ToString());
                Context().Check(centre != Color::CornflowerBlue, "the triangle covers the middle of the screen");
                Context().Check(pixels.At(10, 10) == Color::CornflowerBlue, "the corner keeps the clear colour");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::array<VertexPositionColor, 3> vertices_{};
    };
}

CNA_DEMOS_GAME(DrawUserPrimitivesDemo, {
    .id = "graphics.primitives.draw_user_primitives",
    .title = "Hello triangle",
    .description = "Draws one vertex-coloured triangle in 3D: VertexPositionColor vertices, a BasicEffect with world, "
                   "view and projection matrices, and GraphicsDevice::DrawUserPrimitives once per effect pass.",
    .apis = {"GraphicsDevice::DrawUserPrimitives", "VertexPositionColor::VertexPositionColor", "BasicEffect::BasicEffect",
             "BasicEffect::VertexColorEnabled", "BasicEffect::World", "BasicEffect::View", "BasicEffect::Projection",
             "Effect::CurrentTechnique", "EffectTechnique::Passes", "EffectPass::Apply", "PrimitiveType::TriangleList"},
    .tags = {"3d", "triangle", "hello", "vertex", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
