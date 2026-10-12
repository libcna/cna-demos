// SPDX-License-Identifier: MS-PL
// CNAEXT rigid node animation: glTF clips that move a model's own bones (no skin).
#include <cmath>
#include <optional>
#include <stdexcept>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using System::TimeSpan;

namespace
{
    class NodeAnimationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            stand_.emplace(getContentProperty().Load<Model>("models/turntable"));
            // A glTF whose animations move ordinary nodes keeps its clips in Model.Tag, as a
            // skinned one keeps SkinningData there; every clip targets Model::Bones.
            animations_ = dynamic_cast<const ModelAnimationsEXT*>(stand_->getTagProperty());
            ctx.Check(animations_ != nullptr && animations_->Clips.contains("Spin") && animations_->Clips.contains("Open"),
                      "Model.Tag holds ModelAnimationsEXT with the clips Spin and Open");
            if (animations_ == nullptr)
            {
                return;
            }
            const AnimationClip& spin = animations_->Clips.at("Spin");
            ctx.Print("Spin: {:.1f} s, {} track(s); Open: {:.1f} s", spin.Duration.getTotalSecondsProperty(),
                      spin.Tracks.size(), animations_->Clips.at("Open").Duration.getTotalSecondsProperty());
            ctx.Check(spin.TargetSpace == ClipTargetSpaceEXT::SceneNode && spin.Duration == TimeSpan::FromSeconds(4.0),
                      "Spin is a four-second scene-node clip");

            // Posing at 1 s: a quarter turn of the platform, carried down to the crate on it.
            ModelBone* base = stand_->getBonesProperty()["Base"];
            const Matrix baseBefore = base->getTransformProperty();
            ApplyClipToBonesEXT(*stand_, spin, TimeSpan::FromSeconds(1.0));
            absolute_.resize(static_cast<std::size_t>(stand_->getBonesProperty().getCountProperty()));
            stand_->CopyAbsoluteBoneTransformsTo(absolute_);
            const ModelBone* crate = stand_->getBonesProperty()["Crate"];
            const Vector3 edge = Vector3::Transform(Vector3::UnitX, absolute_[static_cast<std::size_t>(crate->getIndexProperty())]);
            ctx.Print("crate's +X edge at 1 s: ({:.2f}, {:.2f}, {:.2f})", edge.X, edge.Y, edge.Z);
            ctx.Check(std::abs(edge.X) < 1e-3f && std::abs(edge.Z + 1.0f) < 1e-3f,
                      "a quarter turn about Y carries the crate's +X edge to -Z");
            ctx.Check(base->getTransformProperty() == baseBefore, "bones without a track keep their transform");

            // An untracked transform is application state, not an instruction to restore bind pose.
            const Vector3 offset(3, -1, 2);
            const Matrix movedBase = Matrix::CreateTranslation(offset);
            base->setTransformProperty(movedBase);
            ApplyClipToBonesEXT(*stand_, spin, TimeSpan::FromSeconds(1));
            stand_->CopyAbsoluteBoneTransformsTo(absolute_);
            const Vector3 movedEdge = Vector3::Transform(Vector3::UnitX,
                absolute_[static_cast<std::size_t>(crate->getIndexProperty())]);
            ctx.Check(base->getTransformProperty() == movedBase && Vector3::Distance(movedEdge, edge + offset) < 1e-3f,
                      "a nonidentity untracked base survives sampling and carries its animated descendants");

            std::vector<Matrix> before(absolute_.size()), after(absolute_.size());
            stand_->CopyBoneTransformsTo(before);
            AnimationClip wrongSpace = spin;
            wrongSpace.TargetSpace = ClipTargetSpaceEXT::JointPalette;
            bool refused = false;
            try { ApplyClipToBonesEXT(*stand_, wrongSpace, TimeSpan::Zero); }
            catch (const std::invalid_argument&) { refused = true; }
            ctx.Check(refused, "a joint-palette clip is refused instead of treating joint slots as scene-node indices");
            stand_->CopyBoneTransformsTo(after);
            ctx.Check(after == before, "refusing the wrong target space preserves every current local bone transform");

            ApplyClipToBonesEXT(*stand_, spin, TimeSpan::Zero);
            stand_->CopyBoneTransformsTo(before);
            ApplyClipToBonesEXT(*stand_, spin, TimeSpan::FromSeconds(1)); // disturb before testing the clamp
            ApplyClipToBonesEXT(*stand_, spin, TimeSpan::FromSeconds(-1));
            stand_->CopyBoneTransformsTo(after);
            ctx.Check(after == before && base->getTransformProperty() == movedBase,
                      "negative scene-node sampling clamps to the first pose and retains untracked state");
            ApplyClipToBonesEXT(*stand_, spin, spin.Duration);
            stand_->CopyBoneTransformsTo(before);
            ApplyClipToBonesEXT(*stand_, spin, TimeSpan::FromSeconds(1));
            ApplyClipToBonesEXT(*stand_, spin, spin.Duration + TimeSpan::FromSeconds(1));
            stand_->CopyBoneTransformsTo(after);
            ctx.Check(after == before && base->getTransformProperty() == movedBase,
                      "sampling past the duration clamps to the final pose instead of looping");
            // Restore the demonstration's authored placement before the usual interactive animation.
            base->setTransformProperty(baseBefore);
            ApplyClipToBonesEXT(*stand_, spin, TimeSpan::FromSeconds(1));

            view_ = Matrix::CreateLookAt(Vector3(0.0f, 3.5f, 6.0f), Vector3(0.0f, 0.8f, 0.0f), Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4,
                getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (animations_ == nullptr)
            {
                return;
            }
            // Each clip is evaluated at its own looping time; both write different bones.
            const double t = gameTime.getTotalGameTimeProperty().getTotalSecondsProperty();
            ApplyClipToBonesEXT(*stand_, animations_->Clips.at("Spin"), TimeSpan::FromSeconds(std::fmod(t, 4.0)));
            ApplyClipToBonesEXT(*stand_, animations_->Clips.at("Open"), TimeSpan::FromSeconds(std::fmod(t, 2.0)));
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(40, 44, 56));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            stand_->Draw(Matrix::getIdentityProperty(), view_, projection_);
            DemoGame::Draw(gameTime);
        }

    private:
        std::optional<Model> stand_;
        const ModelAnimationsEXT* animations_ = nullptr;
        std::vector<Matrix> absolute_;
        Matrix view_;
        Matrix projection_;
    };
}

CNA_DEMOS_GAME(NodeAnimationDemo, {
    .id = "graphics.model.node_animation",
    .title = "Node animation clips (CNAEXT)",
    .description = "Plays two glTF clips that animate ordinary nodes of a model -- a spinning platform and a crate "
                   "lid opening -- by reading ModelAnimationsEXT from Model.Tag and posing the bones with "
                   "ApplyClipToBonesEXT each frame; nonidentity untracked state survives, time clamps and wrong target spaces are refused.",
    .apis = {"ApplyClipToBonesEXT", "ModelAnimationsEXT", "AnimationClipEXT", "ClipTargetSpaceEXT", "Model::Tag",
             "Model::CopyAbsoluteBoneTransformsTo", "Model::CopyBoneTransformsTo", "ModelBone::Transform", "Model::Draw", "Model::Bones"},
    .tags = {"3d", "model", "gltf", "animation", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 40,
    .assets = {"models/turntable.cnb"},
})
