// SPDX-License-Identifier: MS-PL
// RasterizerState::CullMode and winding order: which side of a triangle is the front.
#include <array>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
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
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    struct Row
    {
        const char* name;
        const RasterizerState* state;
        bool drawsClockwise;
        bool drawsCounterClockwise;
    };

    class CullModeDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
            effect_->setVertexColorEnabledProperty(true);
            // A standard 3D camera looking down -Z, so "clockwise" means clockwise on screen.
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 5.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreateOrthographic(8.0f, 4.8f, 0.1f, 10.0f));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            // Three rows, each with a clockwise (left) and a counter-clockwise (right) triangle.
            const std::array<Row, 3> rows = {{
                {"CullCounterClockwise (XNA default)", &RasterizerState::CullCounterClockwise, true, false},
                {"CullClockwise", &RasterizerState::CullClockwise, false, true},
                {"CullNone", &RasterizerState::CullNone, true, true},
            }};
            for (std::size_t r = 0; r < rows.size(); ++r)
            {
                const float y = 1.4f - static_cast<float>(r) * 1.4f;
                const std::array<VertexPositionColor, 6> triangles = {
                    // clockwise on screen: top, right, left
                    VertexPositionColor(Vector3(0.5f, y + 0.5f, 0.0f), Color::Lime),
                    VertexPositionColor(Vector3(1.1f, y - 0.5f, 0.0f), Color::Lime),
                    VertexPositionColor(Vector3(-0.1f, y - 0.5f, 0.0f), Color::Lime),
                    // counter-clockwise on screen: top, left, right
                    VertexPositionColor(Vector3(2.5f, y + 0.5f, 0.0f), Color::Orange),
                    VertexPositionColor(Vector3(1.9f, y - 0.5f, 0.0f), Color::Orange),
                    VertexPositionColor(Vector3(3.1f, y - 0.5f, 0.0f), Color::Orange),
                };
                device.setRasterizerStateProperty(*rows[r].state);
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawUserPrimitives(PrimitiveType::TriangleList, triangles.data(), 0, 2);
                }
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                for (std::size_t r = 0; r < rows.size(); ++r)
                {
                    // Triangle centres in pixels (100 pixels per unit): x = 450 and 650, y = 100 + 140 r.
                    const int py = 100 + static_cast<int>(r) * 140;
                    const bool cw = pixels.At(450, py) != Color::Black;
                    const bool ccw = pixels.At(650, py) != Color::Black;
                    Context().Check(cw == rows[r].drawsClockwise && ccw == rows[r].drawsCounterClockwise,
                                    std::string(rows[r].name) + ": clockwise " + (rows[r].drawsClockwise ? "drawn" : "culled") +
                                        ", counter-clockwise " + (rows[r].drawsCounterClockwise ? "drawn" : "culled"));
                }
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            for (std::size_t r = 0; r < rows.size(); ++r)
                spriteBatch_->DrawString(*font_, rows[r].name, Vector2(20.0f, 92.0f + static_cast<float>(r) * 140.0f),
                                         Color::White);
            spriteBatch_->DrawString(*font_, "clockwise", Vector2(396.0f, 20.0f), Color::Lime);
            spriteBatch_->DrawString(*font_, "counter-clockwise", Vector2(548.0f, 20.0f), Color::Orange);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<BasicEffect> effect_;
    };
}

CNA_DEMOS_GAME(CullModeDemo, {
    .id = "graphics.rasterizer_state.cull_mode",
    .title = "Back-face culling",
    .description = "Draws a clockwise and a counter-clockwise triangle under RasterizerState::CullCounterClockwise, "
                   "CullClockwise and CullNone: XNA treats clockwise triangles as front faces.",
    .apis = {"RasterizerState::CullCounterClockwise", "RasterizerState::CullClockwise", "RasterizerState::CullNone",
             "RasterizerState::CullMode", "GraphicsDevice::RasterizerState", "CullMode::CullCounterClockwiseFace",
             "CullMode::CullClockwiseFace", "CullMode::None"},
    .tags = {"3d", "culling", "winding", "rasterizer", "state"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
