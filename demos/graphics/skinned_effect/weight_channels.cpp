// SPDX-License-Identifier: MS-PL
// The declared skinning weight count selects active channels; unused lanes need not be zero.
#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/BufferUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
namespace
{
    class WeightChannelsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            // Active weights sum to one for each mesh. The first two deliberately retain
            // nonzero unused lanes, so an implementation always summing four would draw wrongly.
            const std::array<Vector4, 3> weights{Vector4(1, .25f, .5f, .75f),
                Vector4(.25f, .75f, .5f, .25f), Vector4(.1f, .2f, .3f, .4f)};
            const std::array<std::uint8_t, 4> indices{2, 0, 3, 1}; // index order differs from palette order
            std::vector<VertexPositionNormalTextureSkinned> vertices;
            for (const auto& weight : weights)
                for (const Vector2& position : {Vector2(-.14f, -.14f), Vector2(.14f, -.14f),
                                               Vector2(.14f, .14f), Vector2(-.14f, .14f)})
                    vertices.emplace_back(Vector3(position.X, position.Y, .5f), Vector3::UnitZ,
                                          Vector2::Zero, weight, indices);
            vertices_ = std::make_unique<VertexBuffer>(device,
                VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(), 12, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), 12);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            const std::array<std::uint16_t, 6> triangles{0, 1, 2, 0, 2, 3};
            indices_->SetData(triangles.data(), 6);
            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);
            effect_ = std::make_unique<SkinnedEffect>(device);
            effect_->setTextureProperty(white_.get());
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::getIdentityProperty());
            effect_->setAmbientLightColorProperty(Vector3::One);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
            effect_->SetBoneTransforms({Matrix::CreateTranslation(-.12f, -.4f, 0),
                Matrix::CreateTranslation(.5f, -.1f, 0), Matrix::CreateTranslation(-.3f, .4f, 0),
                Matrix::CreateTranslation(.12f, .2f, 0)});
            Context().Print("Gold: one active lane. Blue: two active lanes. Orange: four nonzero influences. Bone indices: 2,0,3,1; unused lanes in the first two meshes are deliberately nonzero.");
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::MidnightBlue);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            const std::array<int, 3> counts{1, 2, 4};
            const std::array<Color, 3> colors{Color::Gold, Color::CornflowerBlue, Color::Orange};
            for (int mesh = 0; mesh < 3; ++mesh)
            {
                effect_->setWeightsPerVertexProperty(counts[mesh]);
                effect_->setWorldProperty(Matrix::CreateTranslation(.5f * (mesh - 1), 0, 0));
                effect_->setDiffuseColorProperty(colors[mesh].ToVector3());
                for (EffectPass& pass : effect_->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 4 * mesh, 0, 4, 0, 2);
                }
            }
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // Weighted translations in the selected index order, then each panel's World:
                // 1: bone2; 2: .25*bone2+.75*bone0; 4: .1*bone2+.2*bone0+.3*bone3+.4*bone1.
                const std::array<Vector2, 3> centers{Vector2(-.8f, .4f), Vector2(-.165f, -.2f), Vector2(.682f, -.02f)};
                const std::array<const char*, 3> labels{
                    "one active weight ignores all three nonzero unused lanes",
                    "two active weights ignore the last two nonzero lanes",
                    "four nonzero weights blend the four explicitly indexed bone transforms"};
                for (int mesh = 0; mesh < 3; ++mesh)
                {
                    bool matches = true;
                    // Edge-adjacent probes, not just a center that a partially wrong quad might cover.
                    for (const Vector2& offset : {Vector2::Zero, Vector2(-.12f, 0), Vector2(.12f, 0),
                                                 Vector2(0, -.12f), Vector2(0, .12f)})
                    {
                        const auto point = centers[mesh] + offset;
                        const int x = static_cast<int>((point.X + 1) * .5f * pixels.Width());
                        const int y = static_cast<int>((1 - point.Y) * .5f * pixels.Height());
                        matches = matches && CnaDemos::Near(pixels.At(x, y), colors[mesh], 1);
                    }
                    Context().Check(matches, labels[mesh]);
                    const int bindX = static_cast<int>((.5f * (mesh - 1) + 1) * .5f * pixels.Width());
                    Context().Check(pixels.At(bindX, pixels.Height() / 2) == Color::MidnightBlue,
                                    "the mesh has left its unskinned bind-pose center");
                }
            }
            Game::Draw(time);
        }
    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> white_;
        std::unique_ptr<SkinnedEffect> effect_;
    };
}
CNA_DEMOS_GAME(WeightChannelsDemo, {
    .id = "graphics.skinned_effect.weight_channels",
    .title = "Active skinning weight channels",
    .description = "Renders one-, two- and four-influence meshes with explicit bone indices; nonzero unused lanes "
                   "test that WeightsPerVertex selects the active channels, while edge pixels verify the four-bone blend.",
    .apis = {"SkinnedEffect::WeightsPerVertex", "SkinnedEffect::SetBoneTransforms", "SkinnedEffect::World",
             "VertexPositionNormalTextureSkinned::VertexPositionNormalTextureSkinned"},
    .tags = {"skinning", "bones", "effect", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
