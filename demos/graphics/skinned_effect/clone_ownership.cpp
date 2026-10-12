// SPDX-License-Identifier: MS-PL
// Cloned effects own independent palettes while sharing an explicitly owned texture.
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
    class CloneOwnershipDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            const Vector4 weight(1, 0, 0, 0);
            const std::array<std::uint8_t, 4> joint{0, 0, 0, 0};
            const std::array<VertexPositionNormalTextureSkinned, 4> vertices{
                VertexPositionNormalTextureSkinned(Vector3(-.18f, -.45f, .5f), Vector3::UnitZ, Vector2::Zero, weight, joint),
                VertexPositionNormalTextureSkinned(Vector3(.18f, -.45f, .5f), Vector3::UnitZ, Vector2::Zero, weight, joint),
                VertexPositionNormalTextureSkinned(Vector3(.18f, .45f, .5f), Vector3::UnitZ, Vector2::Zero, weight, joint),
                VertexPositionNormalTextureSkinned(Vector3(-.18f, .45f, .5f), Vector3::UnitZ, Vector2::Zero, weight, joint),
            };
            const std::array<std::uint16_t, 6> indices{0, 1, 2, 0, 2, 3};
            vertices_ = std::make_unique<VertexBuffer>(device,
                VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(), 4, BufferUsage::WriteOnly);
            indices_ = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
            vertices_->SetData(vertices.data(), static_cast<int>(vertices.size()));
            indices_->SetData(indices.data(), static_cast<int>(indices.size()));

            auto texture = std::make_shared<Texture2D>(device, 1, 1);
            // A non-white texel makes a lost texture/fallback visible in the rendered check.
            const Color tint(128, 255, 128);
            texture->SetData(&tint, 1);
            textureLifetime_ = texture;
            const std::vector<Matrix> leftPose{Matrix::CreateTranslation(-.45f, 0, 0)};
            const std::vector<Matrix> rightPose{Matrix::CreateTranslation(.45f, 0, 0)};
            {
                SkinnedEffect source(device);
                source.setWorldProperty(Matrix::getIdentityProperty());
                source.setViewProperty(Matrix::getIdentityProperty());
                source.setProjectionProperty(Matrix::getIdentityProperty());
                source.setWeightsPerVertexProperty(1);
                source.setAmbientLightColorProperty(Vector3::One);
                source.getDirectionalLight0Property().setEnabledProperty(false);
                source.getDirectionalLight1Property().setEnabledProperty(false);
                source.getDirectionalLight2Property().setEnabledProperty(false);
                source.setDiffuseColorProperty(Color::Gold.ToVector3());
                source.SetOwnedTexture(texture); // shared ownership, unlike the raw Texture setter
                source.SetBoneTransforms(leftPose);

                // Clone returns a caller-owned Effect*. Each actual object remains a SkinnedEffect.
                left_.reset(static_cast<SkinnedEffect*>(source.Clone()));
                right_.reset(static_cast<SkinnedEffect*>(source.Clone()));
                Context().Check(left_->GetBoneTransforms(1) == leftPose &&
                                right_->GetBoneTransforms(1) == leftPose &&
                                left_->getWeightsPerVertexProperty() == 1 && right_->getWeightsPerVertexProperty() == 1,
                                "both clones initially copy the source palette and weight count");
                Context().Check(left_->getTextureProperty() == texture.get() &&
                                right_->getTextureProperty() == texture.get(),
                                "clones reference the same explicitly owned texture object");
                right_->SetBoneTransforms(rightPose);
                right_->setDiffuseColorProperty(Color::CornflowerBlue.ToVector3());
                Context().Check(source.GetBoneTransforms(1) == leftPose && left_->GetBoneTransforms(1) == leftPose &&
                                source.getDiffuseColorProperty() == Color::Gold.ToVector3(),
                                "changing one clone's palette and material leaves its source and sibling unchanged");
                source.SetBoneTransforms({Matrix::getIdentityProperty()});
                Context().Check(left_->GetBoneTransforms(1) == leftPose && right_->GetBoneTransforms(1) == rightPose,
                                "later source palette changes do not overwrite either clone");
            } // the original effect no longer exists
            texture.reset(); // the caller also gives up its strong reference
            Context().Check(!textureLifetime_.expired(), "cloned effects retain the texture after source and caller release");
            Context().Print("Left: copied gold pose. Right: independent blue pose. Both use the retained green tint. Original effect and caller texture reference released.");
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::MidnightBlue);
            // If ownership were broken, the failed lifetime assertion must not lead to a
            // use-after-free through the effect's texture pointer during this diagnostic demo.
            if (textureLifetime_.expired()) { Game::Draw(time); return; }
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            device.SetVertexBuffer(vertices_.get());
            device.setIndicesProperty(indices_.get());
            for (SkinnedEffect* effect : {left_.get(), right_.get()})
                for (EffectPass& pass : effect->getCurrentTechniqueProperty()->getPassesProperty())
                {
                    pass.Apply();
                    device.DrawIndexedPrimitives(PrimitiveType::TriangleList, 0, 0, 4, 0, 2);
                }
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                // Identity view/projection: both meshes differ only through their bone palette.
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 11 / 40, pixels.Height() / 2), Color(128, 215, 0), 1),
                                "the surviving left clone renders its copied palette, material and retained texel");
                Context().Check(CnaDemos::Near(pixels.At(pixels.Width() * 29 / 40, pixels.Height() / 2), Color(50, 149, 119), 1),
                                "the surviving right clone renders its independent palette, material and retained texel");
                Context().Check(pixels.At(pixels.Width() / 2, pixels.Height() / 2) == Color::MidnightBlue,
                                "neither clone draws at the source's final identity pose");
            }
            Game::Draw(time);
        }

        void UnloadContent() override
        {
            left_.reset();
            Context().Check(!textureLifetime_.expired(), "releasing one clone leaves the sibling's texture ownership intact");
            right_.reset();
            Context().Check(textureLifetime_.expired(), "releasing the last clone releases the owned texture wrapper");
            indices_.reset();
            vertices_.reset();
        }

    private:
        std::unique_ptr<VertexBuffer> vertices_;
        std::unique_ptr<IndexBuffer> indices_;
        std::unique_ptr<SkinnedEffect> left_, right_;
        std::weak_ptr<Texture2D> textureLifetime_;
    };
}

CNA_DEMOS_GAME(CloneOwnershipDemo, {
    .id = "graphics.skinned_effect.clone_ownership",
    .title = "Cloned skin palettes and shared texture lifetime",
    .description = "Clones a SkinnedEffect, gives its copies independent bone palettes and materials, then renders "
                   "both after releasing the source and caller's texture reference; checks final shared-owner release.",
    .apis = {"SkinnedEffect::Clone", "SkinnedEffect::SetBoneTransforms", "SkinnedEffect::GetBoneTransforms",
             "SkinnedEffect::WeightsPerVertex", "SkinnedEffect::DiffuseColor", "SkinnedEffect::Texture",
             "Microsoft::Xna::Framework::Graphics::SkinnedEffect::SetOwnedTexture"},
    .tags = {"skinning", "effect", "clone", "ownership", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .restrictions = "Shared ownership keeps the Texture2D wrapper alive; explicit texture disposal and non-owning raw setters are separate contracts.",
})
