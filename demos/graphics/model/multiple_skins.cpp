// SPDX-License-Identifier: MS-PL
// Independent imported skins retain scene ancestry while ignoring each skinned mesh-node transform.
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMeshPart.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using System::TimeSpan;

namespace
{
    class MultipleSkinsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            model_.emplace(getContentProperty().Load<Model>("models/loose_multi_skin"));
            const auto& skins = model_->getSkinsEXTProperty();
            if (!Context().Check(skins.size() == 2, "the imported model exposes both independent skins")) return;
            // Mapping entries, skeletons and mesh pointers are borrowed from model_.
            const ModelSkinEXT* left = nullptr;
            const ModelSkinEXT* right = nullptr;
            for (const auto& skin : skins)
            {
                if (skin.Name == "LeftRig") left = &skin;
                if (skin.Name == "RightRig") right = &skin;
            }
            if (!Context().Check(left && right && left->Data && right->Data && left->Data != right->Data &&
                                  left->Meshes.size() == 1 && right->Meshes.size() == 1 && left->Meshes[0] != right->Meshes[0],
                                  "each named skin maps distinct skeleton data and its own mesh placement")) return;
            if (!Context().Check(left->Meshes[0]->getMeshPartsProperty().getCountProperty() == 1 &&
                                  right->Meshes[0]->getMeshPartsProperty().getCountProperty() == 1,
                                  "each placement carries one part from the shared authored mesh")) return;
            auto* red = dynamic_cast<SkinnedEffect*>(left->Meshes[0]->getMeshPartsProperty()[0]->getEffectProperty());
            auto* green = dynamic_cast<SkinnedEffect*>(right->Meshes[0]->getMeshPartsProperty()[0]->getEffectProperty());
            if (!Context().Check(red && green && red != green,
                                  "a shared authored material still has independent mutable palettes for the two skins")) return;
            if (!Context().Check(left->Data->BoneCount == 1 && right->Data->BoneCount == 1 &&
                                  left->Data->SkeletonRootPrefix.size() == 1 && right->Data->SkeletonRootPrefix.size() == 1,
                                  "each one-joint skeleton carries a scene-root prefix")) return;
            Context().Print("Imported root-prefix X: left {}, right {}",
                            left->Data->SkeletonRootPrefix[0].M41, right->Data->SkeletonRootPrefix[0].M41);
            Context().CheckNear(left->Data->SkeletonRootPrefix[0].M41, -.5, .0001,
                                "left prefix retains the non-joint ancestor translation -0.5");
            Context().CheckNear(right->Data->SkeletonRootPrefix[0].M41, .5, .0001,
                                "right prefix retains the non-joint ancestor translation +0.5");
            // The current loose importer parents skinned meshes to its identity root. It leaves
            // mesh-node transforms in the scene graph but does not apply them to skinned drawing.
            Context().Check(left->Meshes[0]->getParentBoneProperty() == model_->getRootProperty() &&
                            right->Meshes[0]->getParentBoneProperty() == model_->getRootProperty() &&
                            model_->getRootProperty()->getTransformProperty() == Matrix::getIdentityProperty(),
                            "both skinned mesh placements use the model's identity root");
            const auto& sceneBones = model_->getBonesProperty();
            ModelBone* leftNode = nullptr;
            ModelBone* rightNode = nullptr;
            Context().Check(sceneBones.TryGetValue("LeftMesh", leftNode) && sceneBones.TryGetValue("RightMesh", rightNode) &&
                            leftNode->getTransformProperty().M41 == 4 && rightNode->getTransformProperty().M41 == -4,
                            "authored mesh-node translations remain in the scene graph despite being ignored by skin placement");
            const int leftRoot = left->Data->SkeletonRootNodeIndexEXT;
            const int rightRoot = right->Data->SkeletonRootNodeIndexEXT;
            Context().Check(leftRoot > 0 && leftRoot < sceneBones.getCountProperty() &&
                            rightRoot > 0 && rightRoot < sceneBones.getCountProperty() &&
                            sceneBones[leftRoot]->getNameProperty() == "LeftJoint" &&
                            sceneBones[rightRoot]->getNameProperty() == "RightJoint",
                            "declared rig roots are scene-node indices, distinct from joint slot zero");
            AnimationPlayer leftRest(*left->Data), rightRest(*right->Data);
            Context().Check(red->GetBoneTransforms(1) == leftRest.GetSkinTransforms() &&
                            green->GetBoneTransforms(1) == rightRest.GetSkinTransforms(),
                            "the importer initializes each effect with its own contextual bind palette");
            // Replace only the root's LOCAL transform. Scene context must remain in its prefix.
            AnimationClip lift;
            lift.Duration = TimeSpan::FromSeconds(1);
            BoneTrackEXT track;
            track.BoneIndex = 0;
            track.Keys = {{TimeSpan::Zero, Vector3::Zero}, {lift.Duration, Vector3(0, .4f, 0)}};
            lift.Tracks.push_back(track);
            AnimationPlayer player(*left->Data); // data and clip both outlive this local player
            player.StartClip(lift);
            player.Update(lift.Duration, false, false);
            const auto leftPose = player.GetSkinTransforms();
            Context().Check(leftPose.size() == 1 && Vector3::Distance(leftPose[0].getTranslationProperty(), Vector3(-.5f, .4f, 0)) < .0001f,
                            "animating the root replaces its local pose without discarding scene ancestry");
            red->SetBoneTransforms(leftPose);
            green->SetBoneTransforms({Matrix::CreateTranslation(0, 7, 0)}); // deliberately disturb only the right palette
            const auto posed = ApplyBindPoseBoneTransformsEXT(*model_, *right->Data);
            Context().Check(posed == 1 && green->GetBoneTransforms(1) == rightRest.GetSkinTransforms(),
                            "explicit bind-pose application repairs only the right skin's one effect");
            Context().Check(red->GetBoneTransforms(1) == leftPose,
                            "restoring the right skin leaves the left skin's animation intact");
            // A bound can already be loose. Pick a distant pose from the observed bound itself,
            // so returning a stale bind/previous-pose sphere cannot accidentally pass this check.
            const auto originalBounds = model_->getBoundingSphereEXTProperty();
            if (!Context().Check(originalBounds.has_value(), "the posed model exposes aggregate bounds")) return;
            const Vector3 distantCenter(-.5f, originalBounds->Center.Y + originalBounds->Radius + 2, .5f);
            Context().Check(originalBounds->Contains(distantCenter) == ContainmentType::Disjoint,
                            "the temporary distant pose is definitely outside the currently reported bounds");
            red->SetBoneTransforms({Matrix::CreateTranslation(distantCenter.X, distantCenter.Y, 0)});
            const auto distantBounds = model_->getBoundingSphereEXTProperty();
            Context().Check(distantBounds && distantBounds->Contains(distantCenter) != ContainmentType::Disjoint,
                            "model bounds follow a changed live skin palette without a draw call");
            red->SetBoneTransforms(leftPose); // restore the visible animation before rendering
            const auto restoredBounds = model_->getBoundingSphereEXTProperty();
            Context().Check(restoredBounds && restoredBounds->Contains(distantCenter) == ContainmentType::Disjoint,
                            "restoring the visible palette also restores bounds that exclude the distant pose");
            for (auto* effect : {red, green})
            {
                effect->setWeightsPerVertexProperty(1);
                effect->setAmbientLightColorProperty(Vector3::One);
                effect->getDirectionalLight0Property().setEnabledProperty(false);
                effect->getDirectionalLight1Property().setEnabledProperty(false);
                effect->getDirectionalLight2Property().setEnabledProperty(false);
            }
            red->setDiffuseColorProperty(Vector3(1, 0, 0));
            green->setDiffuseColorProperty(Vector3(0, 1, 0));
            ready_ = true;
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            if (ready_)
            {
                device.setBlendStateProperty(BlendState::Opaque);
                device.setDepthStencilStateProperty(DepthStencilState::None);
                device.setRasterizerStateProperty(RasterizerState::CullNone);
                const Matrix identity = Matrix::getIdentityProperty();
                model_->Draw(identity, identity, identity);
                if (!checked_)
                {
                    const CnaDemos::BackBufferPixels pixels(device);
                    const auto at = [&](float x, float y) { return pixels.At(static_cast<int>((x + 1) * .5f * pixels.Width()),
                                                                             static_cast<int>((1 - y) * .5f * pixels.Height())); };
                    Context().Check(CnaDemos::Near(at(-.5f, .4f), Color::Red, 2), "the left skin appears at its animated scene position");
                    Context().Check(CnaDemos::Near(at(.5f, 0), Color::Lime, 2), "the right skin remains at its independent bind position");
                    Context().Check(CnaDemos::Near(at(-.5f, 0), Color::DarkSlateGray, 2), "the left skin's former position is clear");
                    checked_ = true;
                }
            }
            Game::Draw(time);
        }

    private:
        std::optional<Model> model_;
        bool ready_ = false, checked_ = false;
    };
}

CNA_DEMOS_GAME(MultipleSkinsDemo, {
    .id = "graphics.model.multiple_skins",
    .title = "Pose independent skins in scene context",
    .description = "Imports two skins sharing authored geometry and material, retains non-joint ancestry while ignoring skinned mesh-node placement, "
                   "animates one root, restores the other bind pose independently, and verifies that live palettes update model bounds.",
    .apis = {"Microsoft::Xna::Framework::Graphics::Model::SkinsEXT", "Microsoft::Xna::Framework::Graphics::ModelSkinEXT",
             "Microsoft::Xna::Framework::Graphics::SkinningData::SkeletonRootPrefix",
             "Microsoft::Xna::Framework::Graphics::SkinningData::SkeletonRootNodeIndexEXT",
             "Microsoft::Xna::Framework::Graphics::ApplyBindPoseBoneTransformsEXT",
             "Microsoft::Xna::Framework::Graphics::Model::BoundingSphereEXT", "AnimationPlayer::StartClip",
             "AnimationPlayer::Update", "SkinnedEffect::SetBoneTransforms", "SkinnedEffect::GetBoneTransforms", "Model::Draw"},
    .tags = {"model", "skinning", "hierarchy", "gltf", "animation", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"models/loose_multi_skin.gltf"},
    .restrictions = "The current loose importer ignores skinned mesh-node placement directly; "
                    "older root-prefix comments still describe inverse-mesh cancellation (CNA-DEMOS-ISSUE-042).",
})
