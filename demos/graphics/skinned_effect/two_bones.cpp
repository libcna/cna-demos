// SPDX-License-Identifier: MS-PL
// SkinnedEffect: vertices follow bones by weight -- a bar bending at a joint, like an elbow.
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
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
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Rows = 13;  // a strip from y = -1.5 to 1.5; the joint is at y = 0

    class TwoBonesDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();

            // Each vertex names up to four bones and how much each moves it (weights sum to 1).
            // Below the joint: bone 0 only; above it: bone 1 only; near it: a blend, so it bends smoothly.
            std::vector<VertexPositionNormalTextureSkinned> vertices;
            for (int row = 0; row < Rows; ++row)
            {
                const float y = -1.5f + 3.0f * static_cast<float>(row) / (Rows - 1);
                const float upper = MathHelper::Clamp((y + 0.25f) / 0.5f, 0.0f, 1.0f);  // 0 below, 1 above
                for (const float x : {-0.25f, 0.25f})
                    vertices.emplace_back(Vector3(x, y, 0.0f), Vector3::UnitZ, Vector2(0.0f, 0.0f),
                                          Vector4(1.0f - upper, upper, 0.0f, 0.0f),
                                          std::array<std::uint8_t, 4>{0, 1, 0, 0});
            }
            std::vector<std::uint16_t> indices;
            for (int row = 0; row + 1 < Rows; ++row)
            {
                const auto a = static_cast<std::uint16_t>(2 * row);
                indices.insert(indices.end(), {a, static_cast<std::uint16_t>(a + 2), static_cast<std::uint16_t>(a + 1),
                                               static_cast<std::uint16_t>(a + 1), static_cast<std::uint16_t>(a + 2),
                                               static_cast<std::uint16_t>(a + 3)});
            }
            vertices_ = std::make_unique<VertexBuffer>(device, VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(),
                                                       static_cast<int>(vertices.size()), BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), static_cast<int>(vertices.size()));
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits,
                                                     static_cast<int>(indices.size()), BufferUsage::WriteOnly);
            indices_->SetData(indices.data(), static_cast<int>(indices.size()));

            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);

            effect_ = std::make_unique<SkinnedEffect>(device);
            effect_->setTextureProperty(white_.get());
            effect_->setDiffuseColorProperty(Color::Gold.ToVector3());
            effect_->EnableDefaultLighting();
            effect_->setWeightsPerVertexProperty(2);  // this mesh uses two weights per vertex
            effect_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 6.0f), Vector3::Zero, Vector3::Up));
            effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
                MathHelper::PiOver4, device.getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f));

            // Bone 0 stays put; bone 1 rotates 60 degrees about the joint (rotate around the origin,
            // which is where the joint is in this mesh).
            effect_->SetBoneTransforms({Matrix::getIdentityProperty(),
                                        Matrix::CreateRotationZ(MathHelper::ToRadians(-60.0f))});
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::MidnightBlue);
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

            if (FramesDrawn() == 0)
            {
                // Where the top of the bar would be unbent, and where bone 1 has turned it to.
                const CnaDemos::BackBufferPixels p(device);
                const auto pixel = [&](Vector3 world) {
                    const Vector3 s = device.getViewportProperty().Project(world, effect_->getProjectionProperty(),
                                                                           effect_->getViewProperty(),
                                                                           Matrix::getIdentityProperty());
                    return p.At(static_cast<int>(s.X), static_cast<int>(s.Y));
                };
                const Vector3 tip(0.0f, 1.3f, 0.0f);
                const Vector3 bentTip = Vector3::Transform(tip, Matrix::CreateRotationZ(MathHelper::ToRadians(-60.0f)));
                Context().Check(pixel(Vector3(0.0f, -1.3f, 0.0f)) != Color::MidnightBlue, "the lower half stays in place");
                Context().Check(pixel(tip) == Color::MidnightBlue, "the upper half is no longer where it was modelled");
                Context().Check(pixel(bentTip) != Color::MidnightBlue, "it follows bone 1 to the rotated position");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<Texture2D> white_;
        std::unique_ptr<SkinnedEffect> effect_;
    };
}

CNA_DEMOS_GAME(TwoBonesDemo, {
    .id = "graphics.skinned_effect.two_bones",
    .title = "Skinning with two bones",
    .description = "Bends a strip at a joint with SkinnedEffect: vertices carry bone indices and weights, "
                   "SetBoneTransforms rotates bone 1, and vertices near the joint blend between the bones.",
    .apis = {"SkinnedEffect::SkinnedEffect", "SkinnedEffect::SetBoneTransforms", "SkinnedEffect::WeightsPerVertex",
             "SkinnedEffect::Texture", "VertexPositionNormalTextureSkinned::VertexPositionNormalTextureSkinned"},
    .tags = {"3d", "skinning", "animation", "bones", "effect"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
