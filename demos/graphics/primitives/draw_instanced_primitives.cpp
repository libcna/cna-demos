// SPDX-License-Identifier: MS-PL
// DrawInstancedPrimitives: one draw call for many copies of a mesh, each with its own world matrix.
#include <cmath>
#include <memory>
#include <type_traits>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/Geometry.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DynamicVertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SetDataOptions.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBufferBinding.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexDeclaration.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElement.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexElementUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int GridSize = 10;
    constexpr int InstanceCount = GridSize * GridSize;

    // The per-instance stream: one world matrix per cube. XNA's instancing sample packs a Matrix as
    // four Vector4 elements; CNA's stock effects read the first four elements of the per-instance
    // streams as exactly that matrix (rows M11..M14 to M41..M44) and apply it before World.
    static_assert(std::is_trivially_copyable_v<Matrix> && sizeof(Matrix) == 64, "a Matrix is 16 plain floats");

    const VertexDeclaration& InstanceDeclaration()
    {
        static const VertexDeclaration declaration{
            VertexElement(0, VertexElementFormat::Vector4, VertexElementUsage::TextureCoordinate, 1),
            VertexElement(16, VertexElementFormat::Vector4, VertexElementUsage::TextureCoordinate, 2),
            VertexElement(32, VertexElementFormat::Vector4, VertexElementUsage::TextureCoordinate, 3),
            VertexElement(48, VertexElementFormat::Vector4, VertexElementUsage::TextureCoordinate, 4),
        };
        return declaration;
    }

    class InstancingDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            effect_ = std::make_unique<BasicEffect>(device);
            effect_->EnableDefaultLighting();
            effect_->setDiffuseColorProperty(Vector3(0.35f, 0.6f, 1.0f));
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 9.0f, 14.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // The mesh, once: a static vertex buffer and index buffer of one cube.
            const CnaDemos::Mesh cube = CnaDemos::MakeCube(0.35f);
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionNormalTexture::getVertexDeclarationStatic(),
                                                       cube.VertexCount(), BufferUsage::WriteOnly);
            vertices_->SetData(cube.vertices.data(), cube.VertexCount());
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits,
                                                     static_cast<int>(cube.indices.size()), BufferUsage::WriteOnly);
            indices_->SetData(cube.indices.data(), static_cast<int>(cube.indices.size()));
            vertexCount_ = cube.VertexCount();
            primitiveCount_ = cube.PrimitiveCount();

            // The instances: rewritten every frame, so a dynamic buffer.
            instances_ = std::make_unique<DynamicVertexBuffer>(device, InstanceDeclaration(), InstanceCount,
                                                               BufferUsage::WriteOnly);
            worlds_.resize(InstanceCount);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            const float t = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());
            for (int z = 0; z < GridSize; ++z)
            {
                for (int x = 0; x < GridSize; ++x)
                {
                    const float phase = 0.45f * static_cast<float>(x + z);
                    const Vector3 position(static_cast<float>(x) - 4.5f, 0.6f * std::sin(t * 2.0f + phase),
                                           static_cast<float>(z) - 4.5f);
                    worlds_[static_cast<std::size_t>(z * GridSize + x)] =
                        Matrix::CreateRotationY(t + phase) * Matrix::CreateTranslation(position);
                }
            }
            instances_->SetData(worlds_.data(), 0, InstanceCount, SetDataOptions::Discard);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(24, 26, 36));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullCounterClockwise);

            // Stream 0 steps per vertex; stream 1 steps once per instance (instance frequency 1).
            device.SetVertexBuffers({VertexBufferBinding(vertices_.get(), 0, 0),
                                     VertexBufferBinding(instances_.get(), 0, 1)});
            device.setIndicesProperty(indices_.get());
            for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
            {
                pass.Apply();
                // One call draws all 100 cubes: the cube's primitives, repeated per instance.
                device.DrawInstancedPrimitives(PrimitiveType::TriangleList, 0, 0, vertexCount_, 0, primitiveCount_,
                                               InstanceCount);
            }

            if (FramesDrawn() == 0)
            {
                // Cubes at both sides of the screen prove that each instance had its own matrix.
                const CnaDemos::BackBufferPixels pixels(device);
                int left = 0;
                int right = 0;
                for (int y = 0; y < pixels.Height(); ++y)
                {
                    for (int x = 0; x < pixels.Width(); ++x)
                    {
                        if (!CnaDemos::Near(pixels.At(x, y), Color(24, 26, 36), 2))
                        {
                            ++(x < pixels.Width() / 2 ? left : right);
                        }
                    }
                }
                Context().Print("{} + {} pixels covered by {} instances", left, right, InstanceCount);
                Context().Check(left > 5000 && right > 5000, "one draw call spread cubes across the whole grid");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<BasicEffect> effect_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<DynamicVertexBuffer> instances_;
        std::vector<Matrix> worlds_;
        int vertexCount_ = 0;
        int primitiveCount_ = 0;
    };
}

CNA_DEMOS_GAME(InstancingDemo, {
    .id = "graphics.primitives.draw_instanced_primitives",
    .title = "Hardware instancing",
    .description = "Draws a grid of 100 spinning cubes with one DrawInstancedPrimitives call: the cube mesh in one "
                   "vertex stream and a world matrix per instance in a second stream bound with instance "
                   "frequency 1 (SetVertexBuffers), which CNA's stock BasicEffect applies before World.",
    .apis = {"GraphicsDevice::DrawInstancedPrimitives", "GraphicsDevice::SetVertexBuffers",
             "VertexBufferBinding::VertexBufferBinding", "DynamicVertexBuffer::SetData", "SetDataOptions::Discard",
             "VertexDeclaration::VertexDeclaration", "VertexElement::VertexElement", "GraphicsDevice::Indices"},
    .tags = {"3d", "instancing", "performance", "vertex-streams"},
    .needs = CnaDemos::Need::ThreeD | CnaDemos::Need::Instancing,
    .frames = 20,
})
