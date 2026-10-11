// SPDX-License-Identifier: MS-PL
// DynamicIndexBuffer: choosing every frame which parts of fixed geometry to draw.
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DynamicIndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SetDataOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Cells = 8; // an 8x8 board of tiles

    class DynamicIndexBufferDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setProjectionProperty(Matrix::CreateOrthographicOffCenter(0.0f, 800.0f, 480.0f, 0.0f, 0.0f, 1.0f));

            // The vertices never change: four corners per tile.
            std::vector<VertexPositionColor> vertices;
            for (int y = 0; y < Cells; ++y)
            {
                for (int x = 0; x < Cells; ++x)
                {
                    const float left = 200.0f + 50.0f * static_cast<float>(x);
                    const float top = 40.0f + 50.0f * static_cast<float>(y);
                    const Color color((x + y) % 2 == 0 ? Color::Teal : Color::Gold);
                    vertices.emplace_back(Vector3(left + 2.0f, top + 2.0f, 0.0f), color);
                    vertices.emplace_back(Vector3(left + 48.0f, top + 2.0f, 0.0f), color);
                    vertices.emplace_back(Vector3(left + 2.0f, top + 48.0f, 0.0f), color);
                    vertices.emplace_back(Vector3(left + 48.0f, top + 48.0f, 0.0f), color);
                }
            }
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(),
                                                       static_cast<int>(vertices.size()), BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), static_cast<int>(vertices.size()));

            // Room for every tile; each frame uses only the visible ones.
            indices_ = std::make_unique<DynamicIndexBuffer>(device, IndexElementSize::SixteenBits, Cells * Cells * 6,
                                                            BufferUsage::WriteOnly);
            Context().Check(indices_->getIndexCountProperty() == Cells * Cells * 6 &&
                                indices_->getIndexElementSizeProperty() == IndexElementSize::SixteenBits,
                            "IndexCount and IndexElementSize describe the buffer");
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            time_ += static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(24, 24, 32));
            device.setRasterizerStateProperty(RasterizerState::CullNone);

            // A wave decides which tiles are shown this frame; only their indices are written.
            std::vector<std::uint16_t> indices;
            for (int y = 0; y < Cells; ++y)
            {
                for (int x = 0; x < Cells; ++x)
                {
                    if (std::sin(time_ * 3.0f + 0.7f * static_cast<float>(x + y)) < -0.2f)
                    {
                        continue;
                    }
                    const auto v = static_cast<std::uint16_t>(4 * (y * Cells + x));
                    for (std::uint16_t i : {0, 1, 2, 1, 3, 2})
                    {
                        indices.push_back(static_cast<std::uint16_t>(v + i));
                    }
                }
            }
            // Discard: the old contents are not needed, so the GPU need not wait for draws still
            // reading them -- the usual option for data rewritten every frame.
            indices_->SetData(indices.data(), 0, static_cast<int>(indices.size()), SetDataOptions::Discard);
            if (!checked_)
            {
                checked_ = true;
                Context().Print("first frame: {} of {} tiles drawn", indices.size() / 6, Cells * Cells);
                Context().Check(!indices_->getIsContentLostProperty(), "the content is not lost after SetData");
            }

            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                if (!indices.empty())
                {
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, vertices_->getVertexCountProperty(), 0,
                                                 static_cast<int>(indices.size()) / 3);
                }
            }
            Game::Draw(gameTime);
        }

    private:
        float time_ = 0.0f;
        bool checked_ = false;
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<DynamicIndexBuffer> indices_;
    };
}

CNA_DEMOS_GAME(DynamicIndexBufferDemo, {
    .id = "graphics.index_buffer.dynamic_index_buffer",
    .title = "Dynamic index buffer",
    .description = "Keeps an 8x8 board of tiles in a fixed VertexBuffer and rewrites a DynamicIndexBuffer every frame "
                   "with SetDataOptions::Discard to draw only the tiles a moving wave shows.",
    .apis = {"DynamicIndexBuffer::DynamicIndexBuffer", "DynamicIndexBuffer::SetData", "DynamicIndexBuffer::IsContentLost",
             "IndexBuffer::IndexCount", "IndexBuffer::IndexElementSize", "SetDataOptions::Discard",
             "GraphicsDevice::Indices", "GraphicsDevice::DrawIndexedPrimitives"},
    .tags = {"3d", "buffer", "indices", "dynamic", "gpu"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 30,
})
