// SPDX-License-Identifier: MS-PL
// GraphicsDevice::DrawUserIndexedPrimitives: share vertices between triangles through an index list.
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DrawUserIndexedPrimitivesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f));

            // A hexagon: one centre vertex and six around it -- 7 vertices instead of 18.
            const std::array<Color, 6> rim = {Color::Red, Color::Yellow, Color::Lime, Color::Cyan, Color::Blue,
                                              Color::Magenta};
            vertices_[0] = VertexPositionColor(Vector3(400.0f, 240.0f, 0.0f), Color::White);
            for (int i = 0; i < 6; ++i)
            {
                const float angle = MathHelper::TwoPi * static_cast<float>(i) / 6.0f;
                vertices_[static_cast<std::size_t>(i) + 1] = VertexPositionColor(
                    Vector3(400.0f + 200.0f * std::cos(angle), 240.0f + 200.0f * std::sin(angle), 0.0f),
                    rim[static_cast<std::size_t>(i)]);
            }

            // Each triangle names its three corners by index; the centre is used six times.
            for (int i = 0; i < 6; ++i)
            {
                indices_[static_cast<std::size_t>(i) * 3 + 0] = 0;
                indices_[static_cast<std::size_t>(i) * 3 + 1] = static_cast<std::uint16_t>(1 + i);
                indices_[static_cast<std::size_t>(i) * 3 + 2] = static_cast<std::uint16_t>(1 + (i + 1) % 6);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                // vertexOffset 0, numVertices 7, indexOffset 0, primitiveCount 6.
                device.DrawUserIndexedPrimitives(PrimitiveType::TriangleList, vertices_.data(), 0,
                                                 static_cast<int>(vertices_.size()), indices_.data(), 0, 6);
            }

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(CnaDemos::Near(pixels.At(400, 240), Color::White, 8),
                                "the shared centre vertex is white in every triangle");
                Context().Check(pixels.At(400, 100) != Color::Black && pixels.At(400, 380) != Color::Black,
                                "triangles above and below the centre are filled");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::array<VertexPositionColor, 7> vertices_{};
        std::array<std::uint16_t, 18> indices_{};
    };
}

CNA_DEMOS_GAME(DrawUserIndexedPrimitivesDemo, {
    .id = "graphics.primitives.draw_user_indexed_primitives",
    .title = "Indexed triangles",
    .description = "Draws a hexagon from 7 shared vertices and 18 16-bit indices with "
                   "GraphicsDevice::DrawUserIndexedPrimitives instead of 18 separate vertices.",
    .apis = {"GraphicsDevice::DrawUserIndexedPrimitives", "PrimitiveType::TriangleList"},
    .tags = {"3d", "indices", "primitives", "mesh"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
