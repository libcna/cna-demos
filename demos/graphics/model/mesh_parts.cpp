// SPDX-License-Identifier: MS-PL
// Two material parts select different triangles from shared vertex and index buffers.
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class MeshPartsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            red_ = std::make_unique<BasicEffect>(device);
            green_ = std::make_unique<BasicEffect>(device);
            for (BasicEffect* effect : {red_.get(), green_.get()})
            {
                effect->setWorldProperty(Matrix::getIdentityProperty());
                effect->setViewProperty(Matrix::getIdentityProperty());
                effect->setProjectionProperty(Matrix::getIdentityProperty());
                effect->setLightingEnabledProperty(false);
            }
            red_->setDiffuseColorProperty(Vector3(1, 0, 0));
            green_->setDiffuseColorProperty(Vector3(0, 1, 0));
            // Identity matrices put these two quads directly in clip space. Each part draws
            // only one of its quad's triangles: the missing half makes a wrong StartIndex visible.
            const std::array<VertexPositionColor, 8> vertices = {
                VertexPositionColor(Vector3(-.8f, -.6f, .5f), Color::White),
                VertexPositionColor(Vector3(-.2f, -.6f, .5f), Color::White),
                VertexPositionColor(Vector3(-.2f,  .6f, .5f), Color::White),
                VertexPositionColor(Vector3(-.8f,  .6f, .5f), Color::White),
                VertexPositionColor(Vector3( .2f, -.6f, .5f), Color::White),
                VertexPositionColor(Vector3( .8f, -.6f, .5f), Color::White),
                VertexPositionColor(Vector3( .8f,  .6f, .5f), Color::White),
                VertexPositionColor(Vector3( .2f,  .6f, .5f), Color::White),
            };
            const std::array<std::uint16_t, 6> indices{0, 1, 2, 0, 2, 3};
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionColor::getVertexDeclarationStatic(),
                                                       8, BufferUsage::WriteOnly);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), static_cast<int>(vertices.size()));
            indices_->SetData(indices.data(), static_cast<int>(indices.size()));
            // numVertices, primitiveCount, startIndex, vertexOffset. Buffers and effects stay
            // owned by this demo; these public CNA constructors store pointers, not ownership.
            left_ = std::make_unique<ModelMeshPart>(vertices_.get(), indices_.get(), 4, 1, 0, 0);
            right_ = std::make_unique<ModelMeshPart>(vertices_.get(), indices_.get(), 4, 1, 3, 4);
            mesh_ = std::make_unique<ModelMesh>(&device, "Two parts", std::vector<ModelMeshPart*>{left_.get(), right_.get()});
            Context().Check(left_->getVertexBufferProperty() == right_->getVertexBufferProperty() &&
                            left_->getIndexBufferProperty() == right_->getIndexBufferProperty(),
                            "the two mesh parts share vertex and index buffers");
            Context().Print("Right part: {} vertices, {} primitive, start index {}, vertex offset {}",
                            right_->getNumVerticesProperty(), right_->getPrimitiveCountProperty(),
                            right_->getStartIndexProperty(), right_->getVertexOffsetProperty());

            // Use the part setter: directly editing mesh.Effects would bypass this relationship.
            left_->setEffectProperty(red_.get());
            right_->setEffectProperty(red_.get());
            const auto& effects = mesh_->getEffectsProperty();
            Context().Check(effects.getCountProperty() == 1 && effects.Contains(red_.get()),
                            "two parts using the same effect produce one Effects entry");
            right_->setEffectProperty(green_.get());
            Context().Check(effects.getCountProperty() == 2 && effects.Contains(red_.get()) && effects.Contains(green_.get()),
                            "replacing one part's effect preserves the effect still used by its sibling");
            left_->setEffectProperty(green_.get());
            Context().Check(effects.getCountProperty() == 1 && effects.Contains(green_.get()) && !effects.Contains(red_.get()),
                            "replacing the last use removes the old effect and deduplicates the replacement");
            left_->setEffectProperty(red_.get());
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            mesh_->Draw(); // applies each part's effect and its own buffer ranges
            if (!checked_)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const auto at = [&](float x, float y) {
                    return pixels.At(static_cast<int>((x + 1) * .5f * pixels.Width()),
                                     static_cast<int>((1 - y) * .5f * pixels.Height()));
                };
                Context().Check(CnaDemos::Near(at(-.35f, -.3f), Color::Red, 2), "the left indexed triangle uses the red effect");
                Context().Check(CnaDemos::Near(at(.35f, .3f), Color::Lime, 2), "the right base-vertex range uses the green effect");
                Context().Check(CnaDemos::Near(at(-.65f, .3f), Color::DarkSlateGray, 2), "the unselected half of the left quad stays clear");
                Context().Check(CnaDemos::Near(at(.65f, -.3f), Color::DarkSlateGray, 2), "the right StartIndex selects the other triangle only");
                checked_ = true;
            }
            Game::Draw(time);
        }

    private:
        // Reverse destruction order drops the mesh and its parts before the referenced objects.
        std::unique_ptr<BasicEffect> red_, green_;
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<ModelMeshPart> left_, right_;
        std::unique_ptr<ModelMesh> mesh_;
        bool checked_ = false;
    };
}

CNA_DEMOS_GAME(MeshPartsDemo, {
    .id = "graphics.model.mesh_parts",
    .title = "Mesh parts share buffers and choose their own effects",
    .description = "Builds two parts with shared buffers but different index/base-vertex ranges, checks effect "
                   "collection membership on replacement, and verifies the two selected triangles with pixels.",
    .apis = {"ModelMesh::ModelMesh", "ModelMesh::Draw", "ModelMesh::Effects", "ModelMeshPart::ModelMeshPart",
             "ModelMeshPart::Effect", "ModelMeshPart::NumVertices", "ModelMeshPart::PrimitiveCount",
             "ModelMeshPart::StartIndex", "ModelMeshPart::VertexOffset", "ModelMeshPart::VertexBuffer", "ModelMeshPart::IndexBuffer"},
    .tags = {"model", "mesh", "buffers", "effect", "cnaext", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
