// SPDX-License-Identifier: MS-PL
// VertexBuffer::GetData: reading vertices back from a buffer (tools, collision data, debugging).
#include <algorithm>
#include <array>
#include <memory>

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
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class GetDataDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f));

            // Two triangles of a quad, uploaded once. BufferUsage::None (not WriteOnly) keeps the
            // buffer readable, as XNA requires for GetData.
            const std::array<VertexPositionColor, 6> quad = {
                VertexPositionColor(Vector3(200.0f, 100.0f, 0.0f), Color::Red),
                VertexPositionColor(Vector3(600.0f, 100.0f, 0.0f), Color::Lime),
                VertexPositionColor(Vector3(200.0f, 380.0f, 0.0f), Color::Blue),
                VertexPositionColor(Vector3(600.0f, 100.0f, 0.0f), Color::Lime),
                VertexPositionColor(Vector3(600.0f, 380.0f, 0.0f), Color::Yellow),
                VertexPositionColor(Vector3(200.0f, 380.0f, 0.0f), Color::Blue),
            };
            buffer_ = std::make_unique<VertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(),
                                                     static_cast<int>(quad.size()), BufferUsage::None);
            Context().Check(buffer_->getBufferUsageProperty() == BufferUsage::None,
                            "BufferUsage reports the readable usage chosen at construction");
            buffer_->SetData(quad.data(), static_cast<int>(quad.size()));

            // Read everything back...
            std::array<VertexPositionColor, 6> all{};
            buffer_->GetData(all.data(), static_cast<int>(all.size()));
            Context().Check(all == quad, "GetData returns every vertex that was set");

            // ...or a range: the source is chosen by a byte offset (here vertices 3 and 4, the second
            // triangle's first two corners); startIndex is where they go in the destination array.
            std::array<VertexPositionColor, 2> part{};
            const int stride = VertexPositionColor::getVertexDeclarationStatic().getVertexStrideProperty();
            buffer_->GetData(3 * stride, part.data(), 0, 2, stride);
            Context().Check(part[0] == quad[3] && part[1] == quad[4], "GetData(offsetInBytes, ...) reads a range");

            // A typical use: the bounds of the geometry, computed from the buffer itself.
            float minX = all[0].Position.X;
            float maxX = all[0].Position.X;
            for (const VertexPositionColor& v : all)
            {
                minX = std::min(minX, v.Position.X);
                maxX = std::max(maxX, v.Position.X);
            }
            Context().Print("{} vertices read back, x from {} to {}", all.size(), minX, maxX);
            Context().Check(minX == 200.0f && maxX == 600.0f, "the bounds computed from the read-back data");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(30, 30, 40));
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(buffer_.get());
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList, 0, 2);
            }
            Game::Draw(gameTime);
        }

        void UnloadContent() override
        {
            buffer_->Dispose();
            Context().Check(buffer_->getIsDisposedProperty(), "VertexBuffer.Dispose releases the readable buffer");
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<VertexBuffer> buffer_;
    };
}

CNA_DEMOS_GAME(GetDataDemo, {
    .id = "graphics.vertex_buffer.get_data",
    .title = "Reading a vertex buffer back",
    .description = "Fills a readable VertexBuffer (BufferUsage::None) with a coloured quad, reads all of it and a "
                   "range of it back with GetData, computes the geometry's bounds from the read-back vertices and "
                   "draws the quad.",
    .apis = {"VertexBuffer::GetData", "VertexBuffer::SetData", "VertexBuffer::BufferUsage",
             "VertexBuffer::Dispose", "BufferUsage::None", "VertexBuffer::VertexBuffer",
             "VertexDeclaration::VertexStride"},
    .tags = {"3d", "buffer", "vertices", "readback", "tools"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
