// SPDX-License-Identifier: MS-PL
// The depth buffer: with DepthStencilState::Default the nearer surface wins, whatever the draw order.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A square centred on c, facing the camera, as two clockwise triangles.
    std::array<VertexPositionColor, 6> Square(Vector3 c, float half, Color color)
    {
        const VertexPositionColor tl(Vector3(c.X - half, c.Y + half, c.Z), color),
            tr(Vector3(c.X + half, c.Y + half, c.Z), color), bl(Vector3(c.X - half, c.Y - half, c.Z), color),
            br(Vector3(c.X + half, c.Y - half, c.Z), color);
        return {tl, tr, bl, tr, br, bl};
    }

    // Each pair: a red square near the camera and a larger blue one further away, up and to the
    // right, so the blue one shows around the red one even where the depth test hides it.
    Vector3 NearCentre(float x) { return Vector3(x - 0.3f, 0.0f, 1.0f); }
    Vector3 FarCentre(float x) { return Vector3(x + 0.6f, 0.6f, -1.0f); }
    // A point on the red square that the blue one also covers on screen.
    Vector3 OverlapPoint(float x) { return NearCentre(x) + Vector3(0.7f, 0.0f, 0.0f); }

    class DepthBufferDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // Clearing the depth buffer to 1.0 (the far plane) is part of every 3D frame.
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            // Both halves draw the near red square first, then the far blue one overlapping it.
            DrawPair(DepthStencilState::None, -2.0f);     // no depth test: the later (far) square covers
            DrawPair(DepthStencilState::Default, 2.0f);   // depth test: the near square stays in front

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // The pixel at the centre of each red square, found by projecting it to the screen.
                const auto pixelOf = [&](Vector3 point) {
                    const Vector3 p = device.getViewportProperty().Project(point, effect_->getProjectionProperty(),
                                                                           effect_->getViewProperty(),
                                                                           Matrix::getIdentityProperty());
                    return pixels.At(static_cast<int>(p.X), static_cast<int>(p.Y));
                };
                const Color left = pixelOf(OverlapPoint(-2.0f));
                const Color right = pixelOf(OverlapPoint(2.0f));
                Context().Print("overlap without depth test {}, with depth test {}", left.ToString(), right.ToString());
                Context().Check(left == Color::Blue, "DepthStencilState::None: the square drawn last wins");
                Context().Check(right == Color::Red, "DepthStencilState::Default: the nearer square wins");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "DepthStencilState::None", Vector2(40.0f, 420.0f), Color::White);
            spriteBatch_->DrawString(*font_, "DepthStencilState::Default", Vector2(470.0f, 420.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        void DrawPair(const DepthStencilState& state, float x)
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.setDepthStencilStateProperty(state);
            const auto nearSquare = Square(NearCentre(x), 1.0f, Color::Red);
            const auto farSquare = Square(FarCentre(x), 1.4f, Color::Blue);
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawUserPrimitives(PrimitiveType::TriangleList, nearSquare.data(), 0, 2);
                device.DrawUserPrimitives(PrimitiveType::TriangleList, farSquare.data(), 0, 2);
            }
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<BasicEffect> effect_;
    };
}

CNA_DEMOS_GAME(DepthBufferDemo, {
    .id = "graphics.depth_stencil_state.depth_buffer",
    .title = "Depth testing",
    .description = "Draws a near red square, then a larger far blue one behind it, with DepthStencilState::None "
                   "(draw order wins) and DepthStencilState::Default (the nearer surface wins).",
    .apis = {"DepthStencilState::Default", "DepthStencilState::None", "GraphicsDevice::DepthStencilState",
             "DepthStencilState::DepthBufferEnable", "Viewport::Project"},
    .tags = {"3d", "depth", "z-buffer", "state"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
