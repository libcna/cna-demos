// SPDX-License-Identifier: MS-PL
// RasterizerState::DepthBias: pull a decal towards the camera so it wins against the surface it lies on.
#include <array>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/CullMode.hpp"
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
    std::array<VertexPositionColor, 6> Quad(Rectangle r, float z, Color color)
    {
        const auto x0 = static_cast<float>(r.X), y0 = static_cast<float>(r.Y);
        const auto x1 = static_cast<float>(r.X + r.Width), y1 = static_cast<float>(r.Y + r.Height);
        const VertexPositionColor a(Vector3(x0, y0, z), color), b(Vector3(x1, y0, z), color),
            c(Vector3(x0, y1, z), color), d(Vector3(x1, y1, z), color);
        return {a, b, c, b, d, c};
    }

    class DepthBiasDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 10.0f));

            // A negative bias moves the decal's depth towards the camera by a fixed amount
            // (in normalized depth units), enough to beat the tiny gap below.
            biased_ = std::make_unique<RasterizerState>();
            biased_->setCullModeProperty(CullMode::None);
            biased_->setDepthBiasProperty(-0.01f);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);

            // The floor, then two decals placed (as rounding or modelling error would) just behind it.
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            Draw(Quad(Rectangle(40, 60, 720, 320), -5.0f, Color::ForestGreen));
            Draw(Quad(Rectangle(120, 140, 200, 160), -5.02f, Color::White));
            device.setRasterizerStateProperty(*biased_);
            Draw(Quad(Rectangle(480, 140, 200, 160), -5.02f, Color::White));

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                Context().Check(p.At(220, 220) == Color::ForestGreen, "without bias the decal is hidden by the floor");
                Context().Check(p.At(580, 220) == Color::White, "with DepthBias -0.01 the decal is drawn on top");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "DepthBias 0", Vector2(160.0f, 400.0f), Color::White);
            spriteBatch_->DrawString(*font_, "DepthBias -0.01", Vector2(510.0f, 400.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
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
        std::unique_ptr<RasterizerState> biased_;
    };
}

CNA_DEMOS_GAME(DepthBiasDemo, {
    .id = "graphics.rasterizer_state.depth_bias",
    .title = "Depth bias for decals",
    .description = "Draws two decals a hair behind a floor: the plain one loses the depth test, the one drawn with "
                   "RasterizerState::DepthBias -0.01 is pulled in front.",
    .apis = {"RasterizerState::DepthBias", "RasterizerState::RasterizerState"},
    .tags = {"3d", "depth", "decal", "z-fighting", "rasterizer", "state"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
