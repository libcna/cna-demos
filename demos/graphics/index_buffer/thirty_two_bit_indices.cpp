// SPDX-License-Identifier: MS-PL
// IndexElementSize::ThirtyTwoBits: meshes with more than 65,536 vertices (HiDef only).
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
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
    constexpr int Side = 300;  // a 300 x 300 grid: 90,000 vertices -- more than 16-bit indices can name

    class ThirtyTwoBitIndicesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 1.6f, 2.2f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // A rippled height field, coloured by height.
            std::vector<VertexPositionColor> grid;
            grid.reserve(static_cast<std::size_t>(Side) * Side);
            for (int z = 0; z < Side; ++z)
            {
                for (int x = 0; x < Side; ++x)
                {
                    const float u = static_cast<float>(x) / (Side - 1) * 2.0f - 1.0f;
                    const float v = static_cast<float>(z) / (Side - 1) * 2.0f - 1.0f;
                    const float h = 0.12f * std::sin(10.0f * std::sqrt(u * u + v * v));
                    const auto level = static_cast<int>(127.0f + 1000.0f * h);
                    grid.emplace_back(Vector3(u, h, v), Color(level, 80, 255 - level));
                }
            }

            std::vector<std::uint32_t> indices;
            indices.reserve(static_cast<std::size_t>(Side - 1) * (Side - 1) * 6);
            for (int z = 0; z + 1 < Side; ++z)
            {
                for (int x = 0; x + 1 < Side; ++x)
                {
                    const auto i = static_cast<std::uint32_t>(z * Side + x);
                    const auto below = i + static_cast<std::uint32_t>(Side);
                    indices.insert(indices.end(), {i, i + 1, below, i + 1, below + 1, below});
                }
            }

            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(),
                                                       static_cast<int>(grid.size()), BufferUsage::WriteOnly);
            vertices_->SetData(grid.data(), static_cast<int>(grid.size()));
            // 32-bit indices: four bytes each, required once a vertex number exceeds 65,535.
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::ThirtyTwoBits,
                                                     static_cast<int>(indices.size()), BufferUsage::WriteOnly);
            indices_->SetData(indices.data(), static_cast<int>(indices.size()));

            Context().Print("{} vertices, {} triangles, largest index {}", grid.size(), indices.size() / 3,
                            grid.size() - 1);
            Context().Check(grid.size() > 65536 && indices_->getIndexElementSizeProperty() == IndexElementSize::ThirtyTwoBits,
                            "the mesh needs, and has, 32-bit indices");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, vertices_->getVertexCountProperty(), 0,
                                             indices_->getIndexCountProperty() / 3);
            }
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
    };
}

CNA_DEMOS_GAME(ThirtyTwoBitIndicesDemo, {
    .id = "graphics.index_buffer.thirty_two_bit_indices",
    .title = "32-bit indices",
    .description = "Draws a 300 x 300 height-field grid (90,000 vertices) through an IndexBuffer with "
                   "IndexElementSize::ThirtyTwoBits, which XNA allows under HiDef.",
    .apis = {"IndexElementSize::ThirtyTwoBits", "IndexBuffer::IndexBuffer", "IndexBuffer::SetData",
             "GraphicsDevice::DrawIndexedPrimitives"},
    .tags = {"3d", "indices", "terrain", "mesh", "hidef"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 2,
    .deterministic = true,
})
