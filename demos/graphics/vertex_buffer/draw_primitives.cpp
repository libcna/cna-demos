// SPDX-License-Identifier: MS-PL
// VertexBuffer: upload vertices to the GPU once, then draw them every frame with DrawPrimitives.
#include <cmath>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Points = 5;

    class DrawPrimitivesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f));

            // A five-pointed star as a triangle list: one triangle per point, plus the pentagon.
            std::vector<VertexPositionColor> star;
            const auto corner = [](int i, float radius) {
                const float angle = -MathHelper::PiOver2 + MathHelper::TwoPi * static_cast<float>(i) / (2 * Points);
                return Vector3(400.0f + radius * std::cos(angle), 240.0f + radius * std::sin(angle), 0.0f);
            };
            for (int i = 0; i < Points; ++i)
            {
                star.emplace_back(corner(2 * i, 220.0f), Color::Gold);
                star.emplace_back(corner(2 * i + 1, 85.0f), Color::OrangeRed);
                star.emplace_back(corner(2 * i - 1, 85.0f), Color::OrangeRed);
            }
            for (int i = 1; i < Points - 1; ++i)
            {
                star.emplace_back(corner(1, 85.0f), Color::OrangeRed);
                star.emplace_back(corner(2 * i + 1, 85.0f), Color::OrangeRed);
                star.emplace_back(corner(2 * i + 3, 85.0f), Color::OrangeRed);
            }

            // The declaration describes the layout; WriteOnly tells the driver the CPU never reads it.
            vertexBuffer_ = std::make_unique<VertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(),
                                                           static_cast<int>(star.size()), BufferUsage::WriteOnly);
            vertexBuffer_->SetData(star.data(), static_cast<int>(star.size()));
            Context().Print("VertexCount {}, stride {} bytes", vertexBuffer_->getVertexCountProperty(),
                            vertexBuffer_->getVertexDeclarationProperty().getVertexStrideProperty());
            Context().Check(vertexBuffer_->getVertexCountProperty() == 3 * (2 * Points - 2),
                            "the buffer holds every vertex of the star");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::MidnightBlue);
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            // Bind the buffer, then draw from it: no vertex data crosses from the CPU per frame.
            device.SetVertexBuffer(vertexBuffer_.get());
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList, 0, vertexBuffer_->getVertexCountProperty() / 3);
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<VertexBuffer> vertexBuffer_;
    };
}

CNA_DEMOS_GAME(DrawPrimitivesDemo, {
    .id = "graphics.vertex_buffer.draw_primitives",
    .title = "Vertex buffer",
    .description = "Uploads a star's vertices into a VertexBuffer once and draws it each frame with "
                   "GraphicsDevice::SetVertexBuffer and DrawPrimitives.",
    .apis = {"VertexBuffer::VertexBuffer", "VertexBuffer::SetData", "VertexBuffer::VertexCount",
             "VertexBuffer::VertexDeclaration", "VertexDeclaration::VertexStride", "GraphicsDevice::SetVertexBuffer",
             "GraphicsDevice::DrawPrimitives", "BufferUsage::WriteOnly"},
    .tags = {"3d", "buffer", "vertices", "gpu"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
