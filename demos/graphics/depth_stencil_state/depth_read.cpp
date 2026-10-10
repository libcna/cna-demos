// SPDX-License-Identifier: MS-PL
// DepthStencilState::DepthRead for transparent geometry: tested against the depth buffer, never written.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
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
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A screen-space rectangle at depth z (the camera looks down -Z: -2 is near, -8 far).
    std::array<VertexPositionColor, 6> Quad(Rectangle r, float z, Color color)
    {
        const auto x0 = static_cast<float>(r.X), y0 = static_cast<float>(r.Y);
        const auto x1 = static_cast<float>(r.X + r.Width), y1 = static_cast<float>(r.Y + r.Height);
        const VertexPositionColor a(Vector3(x0, y0, z), color), b(Vector3(x1, y0, z), color),
            c(Vector3(x0, y1, z), color), d(Vector3(x1, y1, z), color);
        return {a, b, c, b, d, c};
    }

    class DepthReadDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
            effect_->setVertexColorEnabledProperty(true);
            // Pixel coordinates in x and y, depth 0..10 along -Z.
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 10.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            // Both halves: an opaque wall, then two half-transparent panes in the "wrong" order --
            // the near red one first, the far blue one second.
            DrawScene(0, DepthStencilState::Default);
            DrawScene(400, DepthStencilState::DepthRead);

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                // (190, 250): both panes, no wall. (100, 250): the red pane only. (200, 140): wall and
                // blue pane behind it.
                Context().Check(p.At(190, 250) == p.At(100, 250),
                                "Default: the near pane wrote depth, so the far pane vanished behind it");
                Context().Check(p.At(590, 250) != p.At(500, 250),
                                "DepthRead: the far pane still shows through the near one");
                Context().Check(p.At(200, 140) == Color::Gray && p.At(600, 140) == Color::Gray,
                                "both: the opaque wall still hides the far pane (depth is tested)");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "DepthStencilState::Default", Vector2(40.0f, 420.0f), Color::White);
            spriteBatch_->DrawString(*font_, "DepthStencilState::DepthRead", Vector2(440.0f, 420.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        void DrawScene(int ox, const DepthStencilState& paneState)
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            const auto wall = Quad(Rectangle(ox + 150, 60, 100, 160), -5.0f, Color::Gray);
            const auto nearPane = Quad(Rectangle(ox + 60, 180, 180, 140), -2.0f, Color::Red * 0.5f);
            const auto farPane = Quad(Rectangle(ox + 140, 120, 200, 200), -8.0f, Color::Blue * 0.5f);

            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            Draw(wall);

            // Transparent geometry: blended, depth-tested, and with DepthRead not depth-written.
            device.setBlendStateProperty(BlendState::AlphaBlend);
            device.setDepthStencilStateProperty(paneState);
            Draw(nearPane);
            Draw(farPane);
        }

        void Draw(const std::array<VertexPositionColor, 6>& quad)
        {
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                getGraphicsDeviceProperty().DrawUserPrimitives(PrimitiveType::TriangleList, quad.data(), 0, 2);
            }
        }

        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<BasicEffect> effect_;
    };
}

CNA_DEMOS_GAME(DepthReadDemo, {
    .id = "graphics.depth_stencil_state.depth_read",
    .title = "Transparency with DepthRead",
    .description = "Draws two half-transparent panes, near one first, behind an opaque wall: with "
                   "DepthStencilState::Default the far pane vanishes behind the near one, with DepthRead it does not.",
    .apis = {"DepthStencilState::DepthRead", "DepthStencilState::Default", "DepthStencilState::DepthBufferWriteEnable",
             "BlendState::AlphaBlend"},
    .tags = {"3d", "depth", "transparency", "blend", "state"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
