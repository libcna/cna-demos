// SPDX-License-Identifier: MS-PL
// A render target with its own depth buffer: 3D drawn off-screen needs DepthFormat to be depth-tested.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    std::array<VertexPositionColor, 6> Square(float cx, float z, Color color)
    {
        const VertexPositionColor tl(Vector3(cx - 1.0f, 1.0f, z), color), tr(Vector3(cx + 1.0f, 1.0f, z), color),
            bl(Vector3(cx - 1.0f, -1.0f, z), color), br(Vector3(cx + 1.0f, -1.0f, z), color);
        return {tl, tr, bl, tr, br, bl};
    }

    class RenderTargetDepthDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // Same size and colour format; only the depth format differs.
            withoutDepth_ = std::make_unique<RenderTarget2D>(device, 320, 240, false, SurfaceFormat::Color,
                                                             DepthFormat::None);
            withDepth_ = std::make_unique<RenderTarget2D>(device, 320, 240, false, SurfaceFormat::Color,
                                                          DepthFormat::Depth24);

            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(
                Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, 320.0f / 240.0f, 0.1f, 100.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            // The same scene into both targets: near red square first, far blue square second.
            for (RenderTarget2D* target : {withoutDepth_.get(), withDepth_.get()})
            {
                device.SetRenderTarget(target);
                device.Clear(Color::Black);
                device.setDepthStencilStateProperty(DepthStencilState::Default);
                device.setRasterizerStateProperty(RasterizerState::CullNone);
                const auto nearSquare = Square(-0.4f, 1.0f, Color::Red);
                const auto farSquare = Square(0.6f, -1.0f, Color::Blue);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, nearSquare.data(), 0, 2);
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, farSquare.data(), 0, 2);
                }
            }
            device.SetRenderTarget(nullptr);

            device.Clear(Color::DimGray);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*withoutDepth_, Vector2(40.0f, 80.0f), Color::White);
            spriteBatch_->Draw(*withDepth_, Vector2(440.0f, 80.0f), Color::White);
            spriteBatch_->DrawString(*font_, "DepthFormat::None", Vector2(40.0f, 340.0f), Color::White);
            spriteBatch_->DrawString(*font_, "DepthFormat::Depth24", Vector2(440.0f, 340.0f), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // The middle of each target is where the two squares overlap.
                const Color without = pixels.At(40 + 175, 80 + 120);
                const Color with = pixels.At(440 + 175, 80 + 120);
                Context().Print("overlap: without depth buffer {}, with depth buffer {}", without.ToString(),
                                with.ToString());
                Context().Check(without == Color::Blue, "without a depth buffer the later (far) square covers");
                Context().Check(with == Color::Red, "with Depth24 the nearer square stays in front");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<RenderTarget2D> withoutDepth_;
        std::unique_ptr<RenderTarget2D> withDepth_;
        std::unique_ptr<BasicEffect> effect_;
    };
}

CNA_DEMOS_GAME(RenderTargetDepthDemo, {
    .id = "graphics.render_target.depth_buffer",
    .title = "Render target with a depth buffer",
    .description = "Renders the same overlapping 3D squares into a RenderTarget2D with DepthFormat::None and one with "
                   "DepthFormat::Depth24: only the second is depth-tested.",
    .apis = {"RenderTarget2D::RenderTarget2D", "RenderTarget2D::DepthStencilFormat", "DepthFormat::Depth24",
             "DepthFormat::None"},
    .tags = {"render-target", "depth", "3d", "offscreen"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
