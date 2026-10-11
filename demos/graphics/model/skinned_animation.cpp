// SPDX-License-Identifier: MS-PL
// Skinned animation: a glTF skin and clip, played with AnimationPlayer into a SkinnedEffect.
#include <cmath>
#include <memory>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class SkinnedAnimationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            // content/models/bending_strip.gltf: a strip skinned to two joints, with a clip "Bend".
            strip_.emplace(getContentProperty().Load<Model>("models/bending_strip"));

            // As in XNA's Skinned Model sample, the skeleton and clips travel in Model.Tag.
            const auto* skinning = dynamic_cast<const SkinningData*>(strip_->getTagProperty());
            if (!Context().Check(skinning != nullptr && skinning->BoneCount == 2 && skinning->AnimationClips.contains("Bend"),
                                  "Model.Tag holds SkinningData for two joints and the Bend clip")) return;
            const AnimationClip& bend = skinning->AnimationClips.at("Bend");
            Context().Print("clip Bend: {} s, {} bone tracks", bend.Duration.getTotalSecondsProperty(), bend.Tracks.size());

            player_.emplace(*skinning);
            Context().Check(player_->getCurrentClipProperty() == nullptr &&
                            player_->getCurrentPositionProperty() == System::TimeSpan::Zero &&
                            player_->GetBoneTransforms() == skinning->BindPose,
                            "before StartClip the player has no active clip and retains local bind transforms");
            player_->StartClip(bend);
            Context().Check(player_->getCurrentClipProperty() == &bend &&
                            player_->getCurrentPositionProperty() == System::TimeSpan::Zero,
                            "StartClip borrows the selected clip and starts at zero");
            // The model owns SkinningData and its clips for the entire player lifetime.
            player_->Update(System::TimeSpan::FromSeconds(.25), true, false);
            player_->Update(System::TimeSpan::FromSeconds(.25), true, false);
            const auto advanced = player_->GetSkinTransforms();
            Context().Check(player_->getCurrentPositionProperty() == System::TimeSpan::FromSeconds(.5),
                            "two relative quarter-second updates accumulate to half a second");
            player_->Update(System::TimeSpan::FromSeconds(.5), false, false);
            Context().Check(player_->GetSkinTransforms() == advanced,
                            "an absolute half-second seek produces the same skin palette as relative advances");
            player_->Update(bend.Duration + System::TimeSpan::FromSeconds(.5), false, true);
            Context().Check(player_->getCurrentPositionProperty() == System::TimeSpan::FromSeconds(.5) &&
                            player_->GetSkinTransforms() == advanced,
                            "looping wraps a complete duration back to the same checked pose");
            player_->Update(bend.Duration + System::TimeSpan::FromSeconds(1), false, false);
            Context().Check(player_->getCurrentPositionProperty() == bend.Duration,
                            "non-looping playback clamps at the clip's end");
            player_->StartClip(bend);
            Context().Check(player_->getCurrentPositionProperty() == System::TimeSpan::Zero,
                            "restarting the same clip resets its playback position");
            // Jump to 0.5 s, where a key turns the upper joint +50 degrees about Z.
            player_->Update(System::TimeSpan::FromSeconds(0.5), false);
            const Matrix upper = player_->GetWorldTransforms()[1];
            const float degrees = MathHelper::ToDegrees(std::atan2(upper.M12, upper.M11));
            Context().Print("upper joint at 0.5 s: {:.1f} degrees", degrees);
            Context().CheckNear(degrees, 50.0, 0.01, "at 0.5 s the clip has turned the upper joint by 50 degrees");

            view_ = Matrix::CreateLookAt(Vector3(0.0f, 2.2f, 8.0f), Vector3(0.0f, 2.2f, 0.0f), Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4,
                getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Update(GameTime& gameTime) override
        {
            // Advance and loop the clip with the frame's elapsed time (after the first frame,
            // which shows the checked pose).
            if (checked_)
                player_->Update(gameTime.getElapsedGameTimeProperty(), true, true);
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::DarkSlateGray);
            if (!player_) { Game::Draw(gameTime); return; }
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            device.setRasterizerStateProperty(RasterizerState::CullNone);  // the strip is seen from both sides

            for (ModelMesh* mesh : strip_->getMeshesProperty())
            {
                for (Effect* effect : mesh->getEffectsProperty())
                {
                    // The skin transforms (bind pose to animated pose, per joint) go to the shader.
                    auto* skinned = dynamic_cast<SkinnedEffect*>(effect);
                    skinned->SetBoneTransforms(player_->GetSkinTransforms());
                    skinned->setViewProperty(view_);
                    skinned->setProjectionProperty(projection_);
                    skinned->EnableDefaultLighting();
                }
                mesh->Draw();
            }

            if (!checked_)
            {
                // The strip's top (0, 4, 0) swings around the joint at (0, 2, 0): to (-2 sin 50, 2 + 2 cos 50).
                const float a = MathHelper::ToRadians(50.0f);
                const Vector3 tip(-1.9f * std::sin(a), 2.0f + 1.9f * std::cos(a), 0.0f);
                const Vector3 onScreen = device.getViewportProperty().Project(tip, projection_, view_, Matrix::getIdentityProperty());
                const Color pixel = CnaDemos::BackBufferPixels(device).At(static_cast<int>(onScreen.X), static_cast<int>(onScreen.Y));
                Context().Print("near the bent tip ({:.0f}, {:.0f}): {} {} {}", onScreen.X, onScreen.Y, pixel.getRProperty(),
                                pixel.getGProperty(), pixel.getBProperty());
                Context().Check(pixel.getGProperty() > pixel.getRProperty() + 40 && pixel.getGProperty() > pixel.getBProperty() + 40,
                                "the skinned strip is drawn bent, its green top where the clip puts it");
                checked_ = true;
            }
            Game::Draw(gameTime);
        }

    private:
        std::optional<Model> strip_;
        std::optional<AnimationPlayer> player_;
        Matrix view_;
        Matrix projection_;
        bool checked_ = false;
    };
}

CNA_DEMOS_GAME(SkinnedAnimationDemo, {
    .id = "graphics.model.skinned_animation",
    .title = "Skinned animation",
    .description = "Plays the \"Bend\" clip of a skinned glTF strip: SkinningData from Model.Tag, AnimationPlayer "
                   "(start, advance, seek, loop, clamp and restart) feeding SkinnedEffect::SetBoneTransforms; the pose at 0.5 s "
                   "is checked numerically and with pixels.",
    .apis = {"AnimationPlayer::StartClip", "AnimationPlayer::Update", "AnimationPlayer::GetSkinTransforms",
             "AnimationPlayer::GetWorldTransforms", "AnimationPlayer::GetBoneTransforms",
             "AnimationPlayer::CurrentClip", "AnimationPlayer::CurrentPosition", "SkinningData", "AnimationClip", "Model::Tag",
             "SkinnedEffect::SetBoneTransforms", "SkinnedEffect::EnableDefaultLighting", "ModelMesh::Draw",
             "Model::Meshes", "ModelMesh::Effects", "SkinnedEffect::View", "SkinnedEffect::Projection"},
    .tags = {"model", "skinning", "animation", "skeleton", "gltf", "3d"},
    .needs = CnaDemos::Need::ThreeD,
})
