// SPDX-License-Identifier: MS-PL
// SkinnedModelEXT owns parts; attachment moves them into a shared, caller-compatible skeleton.
#include <array>
#include <cstdint>
#include <memory>
#include <utility>
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
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexElementSize.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTextureSkinned.hpp"
#include "System/TimeSpan.hpp"
#include "System/ArgumentException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using System::TimeSpan;

namespace
{
    // Every piece is authored against this SAME skeleton and bone order, not just the same count.
    SkinnedModelEXT MakePiece(GraphicsDevice& device, const char* name, Color color, float left, float right)
    {
        SkinnedModelEXT model;
        model.BoneCount = 1;
        model.ParentBoneIndices = {-1};
        model.BindPoseLocal = {Matrix::CreateTranslation(0, .25f, 0)};
        model.InverseBindPoseGlobal = {Matrix::CreateTranslation(0, -.25f, 0)};
        const Vector3 normal(0, 0, 1);
        const Vector2 uv(.5f, .5f);
        const Vector4 weights(1, 0, 0, 0);
        const std::array<std::uint8_t, 4> joints{0, 0, 0, 0};
        const std::array<VertexPositionNormalTextureSkinned, 4> vertices = {
            VertexPositionNormalTextureSkinned(Vector3(left,  -.4f, .5f), normal, uv, weights, joints),
            VertexPositionNormalTextureSkinned(Vector3(right, -.4f, .5f), normal, uv, weights, joints),
            VertexPositionNormalTextureSkinned(Vector3(right,  .4f, .5f), normal, uv, weights, joints),
            VertexPositionNormalTextureSkinned(Vector3(left,   .4f, .5f), normal, uv, weights, joints),
        };
        const std::array<std::uint16_t, 6> indices{0, 1, 2, 0, 2, 3};
        auto vb = std::make_unique<VertexBuffer>(device, VertexPositionNormalTextureSkinned::getVertexDeclarationStatic(),
                                                4, BufferUsage::WriteOnly);
        auto ib = std::make_unique<IndexBuffer>(device, IndexElementSize::SixteenBits, 6, BufferUsage::WriteOnly);
        vb->SetData(vertices.data(), static_cast<int>(vertices.size()));
        ib->SetData(indices.data(), static_cast<int>(indices.size()));
        auto part = std::make_unique<ModelMeshPart>(vb.get(), ib.get(), 4, 2, 0, 0);
        Texture2D texture(device, 1, 1);
        texture.SetData(&color, 1);
        // Unlike ModelMesh's non-owning pointers, this API takes ownership of all three objects.
        model.AddPartEXT(name, std::move(vb), std::move(ib), std::move(part), std::move(texture));
        return model;
    }

    class SkinnedPartsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty();
            model_ = MakePiece(device, "body", Color::Red, -.8f, -.2f);
            AnimationClipEXT clip;
            clip.Duration = TimeSpan::FromSeconds(1);
            BoneTrackEXT track;
            track.BoneIndex = 0;
            track.Keys = {{TimeSpan::Zero, Vector3(0, .25f, 0)},
                          {TimeSpan::FromSeconds(1), Vector3(0, .75f, 0)}};
            clip.Tracks.push_back(track);
            model_.Clips.emplace("Lift", clip);
            {
                auto accessory = MakePiece(device, "badge", Color::Blue, .2f, .8f);
                auto* transferred = accessory.Parts.front().Part;
                model_.AttachPartEXT(std::move(accessory));
                Context().Check(accessory.Parts.empty() && model_.Parts.size() == 2 &&
                                model_.Parts.back().Part == transferred,
                                "attachment moves the donor part into the receiving model");
            } // the donor is gone; the receiver must still own valid buffers and texture
            {
                auto replacement = MakePiece(device, "badge", Color::Lime, .2f, .8f);
                model_.AttachPartEXT(std::move(replacement));
                Context().Check(model_.Parts.size() == 2 && model_.Parts.back().Name == "badge" &&
                                replacement.Parts.empty() && model_.Clips.contains("Lift"),
                                "same-name attachment replaces the part and retains the receiver's clip");
                auto temporary = MakePiece(device, "temporary", Color::Blue, -.1f, .1f);
                model_.AttachPartEXT(std::move(temporary));
                Context().Check(model_.Parts.size() == 3, "the temporary center part is attached before removal");
                model_.RemovePartEXT("temporary"); // do not erase Parts: those are non-owning descriptors
                Context().Check(model_.Parts.size() == 2, "RemovePartEXT removes the temporary part from the public part list");
            }
            {
                auto incompatible = MakePiece(device, "wrong-skeleton", Color::Blue, -.1f, .1f);
                incompatible.BoneCount = 2;
                incompatible.ParentBoneIndices.push_back(0);
                incompatible.BindPoseLocal.push_back(Matrix::getIdentityProperty());
                incompatible.InverseBindPoseGlobal.push_back(Matrix::CreateTranslation(0, -.25f, 0));
                bool refused = false;
                try { model_.AttachPartEXT(std::move(incompatible)); }
                catch (const System::ArgumentException&) { refused = true; }
                Context().Check(refused && model_.Parts.size() == 2 && incompatible.Parts.size() == 1,
                                "a different bone count is refused before either model's parts are moved");
                // Equal counts alone cannot prove equal bone ordering: that remains the caller's job.
            }
            model_.ComputeBoneTransformsEXT("Lift", TimeSpan::FromSeconds(.5), false, bones_);
            Context().Check(bones_.size() == 1, "the sampled palette has one matrix for the shared skeleton");
            Context().CheckNear(bones_.at(0).M42, .25, .0001,
                                "sampled translation .5 minus inverse-bind offset .25 produces skin translation .25");
            effect_ = std::make_unique<SkinnedEffect>(device);
            effect_->setWorldProperty(Matrix::getIdentityProperty());
            effect_->setViewProperty(Matrix::getIdentityProperty());
            effect_->setProjectionProperty(Matrix::getIdentityProperty());
            effect_->setWeightsPerVertexProperty(1);
            effect_->SetBoneTransforms(bones_);
            effect_->setAmbientLightColorProperty(Vector3::One);
            effect_->getDirectionalLight0Property().setEnabledProperty(false);
            effect_->getDirectionalLight1Property().setEnabledProperty(false);
            effect_->getDirectionalLight2Property().setEnabledProperty(false);
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            device.setBlendStateProperty(BlendState::Opaque);
            device.setDepthStencilStateProperty(DepthStencilState::None);
            device.setRasterizerStateProperty(RasterizerState::CullNone);
            for (const auto& entry : model_.Parts)
            {
                effect_->setTextureProperty(entry.Texture);
                effect_->Apply();
                const auto* part = entry.Part;
                device.SetVertexBuffer(part->getVertexBufferProperty());
                device.SetIndexBuffer(part->getIndexBufferProperty());
                device.DrawIndexedPrimitives(PrimitiveType::TriangleList, part->getVertexOffsetProperty(), 0,
                                             part->getNumVerticesProperty(), part->getStartIndexProperty(),
                                             part->getPrimitiveCountProperty());
            }
            if (!checked_)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const auto at = [&](float x, float y) {
                    return pixels.At(static_cast<int>((x + 1) * .5f * pixels.Width()),
                                     static_cast<int>((1 - y) * .5f * pixels.Height()));
                };
                Context().Check(CnaDemos::Near(at(-.5f, .55f), Color::Red, 2), "the body's upper sample is lifted by the skin palette");
                Context().Check(CnaDemos::Near(at(.5f, .55f), Color::Lime, 2), "the replacement part survives donor destruction and uses the same skin palette");
                Context().Check(CnaDemos::Near(at(-.5f, -.3f), Color::DarkSlateGray, 2), "the original lower body position is vacated by skinning");
                Context().Check(CnaDemos::Near(at(0, .2f), Color::DarkSlateGray, 2), "the removed center part is not drawn");
                checked_ = true;
            }
            Game::Draw(time);
        }

    private:
        SkinnedModelEXT model_;
        std::unique_ptr<SkinnedEffect> effect_;
        std::vector<Matrix> bones_;
        bool checked_ = false;
    };
}

CNA_DEMOS_GAME(SkinnedPartsDemo, {
    .id = "graphics.model.skinned_parts",
    .title = "Own, replace and skin attached model parts",
    .description = "Builds pieces against one skeleton, transfers and replaces owned parts by name, removes a part, "
                   "then samples a clip with an inverse bind matrix and verifies the surviving skinned geometry with pixels.",
    .apis = {"Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::AddPartEXT",
             "Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::AttachPartEXT",
             "Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::RemovePartEXT",
             "Microsoft::Xna::Framework::Graphics::SkinnedModelEXT::ComputeBoneTransformsEXT",
             "Microsoft::Xna::Framework::Graphics::VertexPositionNormalTextureSkinned::VertexPositionNormalTextureSkinned",
             "SkinnedEffect::SetBoneTransforms"},
    .tags = {"model", "skinning", "ownership", "animation", "cnaext", "3d"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
})
