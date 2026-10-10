// SPDX-License-Identifier: MS-PL
// VertexBuffer + IndexBuffer + DrawIndexedPrimitives: a cube from 8 vertices and 36 indices.
#include <array>
#include <cstdint>
#include <memory>

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
    class IndexedCubeDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->setVertexColorEnabledProperty(true);
            // A fixed, slightly turned view so three faces are visible.
            effect_->setWorldProperty(Matrix::CreateRotationY(MathHelper::PiOver4 * 0.7f) *
                                      Matrix::CreateRotationX(MathHelper::Pi / 7.0f));
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 4.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // The eight corners, each with its own colour.
            const std::array<VertexPositionColor, 8> corners = {
                VertexPositionColor(Vector3(-1, -1, -1), Color::Black), VertexPositionColor(Vector3(1, -1, -1), Color::Red),
                VertexPositionColor(Vector3(1, 1, -1), Color::Yellow), VertexPositionColor(Vector3(-1, 1, -1), Color::Lime),
                VertexPositionColor(Vector3(-1, -1, 1), Color::Blue), VertexPositionColor(Vector3(1, -1, 1), Color::Magenta),
                VertexPositionColor(Vector3(1, 1, 1), Color::White), VertexPositionColor(Vector3(-1, 1, 1), Color::Cyan),
            };
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(), 8,
                                                       BufferUsage::WriteOnly);
            vertices_->SetData(corners.data(), static_cast<int>(corners.size()));

            // Six faces, two clockwise triangles each, as indices into the corners.
            const std::array<std::uint16_t, 36> faces = {
                4, 7, 6, 4, 6, 5,  // front  (z = +1)
                1, 2, 3, 1, 3, 0,  // back   (z = -1)
                0, 3, 7, 0, 7, 4,  // left   (x = -1)
                5, 6, 2, 5, 2, 1,  // right  (x = +1)
                7, 3, 2, 7, 2, 6,  // top    (y = +1)
                0, 4, 5, 0, 5, 1,  // bottom (y = -1)
            };
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 36, BufferUsage::WriteOnly);
            indices_->SetData(faces.data(), static_cast<int>(faces.size()));
            Context().Check(indices_->getIndexCountProperty() == 36 &&
                                indices_->getIndexElementSizeProperty() == IndexElementSize::SixteenBits,
                            "36 sixteen-bit indices");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::SlateGray);
            // Solid 3D needs the depth test and back-face culling (XNA's defaults).
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                // baseVertex 0, minVertexIndex 0, numVertices 8, startIndex 0, primitiveCount 12.
                device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 8, 0, 12);
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
    };
}

CNA_DEMOS_GAME(IndexedCubeDemo, {
    .id = "graphics.index_buffer.indexed_cube",
    .title = "Indexed cube",
    .description = "Draws a vertex-coloured cube from a VertexBuffer with 8 corners and an IndexBuffer with 36 "
                   "16-bit indices through GraphicsDevice::DrawIndexedPrimitives.",
    .apis = {"IndexBuffer::IndexBuffer", "IndexBuffer::SetData", "IndexBuffer::IndexCount", "IndexBuffer::IndexElementSize",
             "GraphicsDevice::Indices", "GraphicsDevice::DrawIndexedPrimitives", "IndexElementSize::SixteenBits"},
    .tags = {"3d", "cube", "indices", "buffer", "mesh"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
