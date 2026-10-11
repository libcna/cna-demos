// SPDX-License-Identifier: MS-PL
// glTF animation sampler interpolation -- STEP, LINEAR, CUBICSPLINE -- as CNA imports it into
// node-animation clips (CNAEXT).
#include <cmath>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/SkinnedModelEXT.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using System::TimeSpan;

namespace
{
    class ClipInterpolationDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            // slide_rig.gltf: three boxes keyed at x = -2, 0, 2 at 0, 1 and 2 s -- the top one's
            // sampler STEP, the middle one's LINEAR, the bottom one's CUBICSPLINE with flat tangents.
            rig_.emplace(getContentProperty().Load<Model>("models/slide_rig"));
            const auto* animations = dynamic_cast<const ModelAnimationsEXT*>(rig_->getTagProperty());
            ctx.Check(animations != nullptr && animations->Clips.contains("Slide"), "the model has the clip Slide");
            if (animations == nullptr || !animations->Clips.contains("Slide"))
            {
                return;
            }
            slide_ = &animations->Clips.at("Slide");

            // A quarter of the way to the second key: STEP still holds the first value, LINEAR is a
            // quarter of the way there, the eased CUBICSPLINE 3(1/4)^2 - 2(1/4)^3 = 0.156 of it.
            ApplyClipToBonesEXT(*rig_, *slide_, TimeSpan::FromSeconds(0.25));
            const float step = X("StepBox");
            const float linear = X("LinearBox");
            const float cubic = X("CubicBox");
            ctx.Print("x at 0.25 s: STEP {:.4f}, LINEAR {:.4f}, CUBICSPLINE {:.4f}", step, linear, cubic);
            // (CNA-DEMOS-ISSUE-031: CNA keeps only the keys and plays every sampler as LINEAR.)
            ctx.Check(std::abs(step + 2.0f) < 1e-3f, "STEP holds the first key's value until the next key");
            ctx.Check(std::abs(linear + 1.5f) < 1e-3f, "LINEAR is a quarter of the way: -1.5");
            ctx.Check(std::abs(cubic + 1.6875f) < 0.01f, "CUBICSPLINE with flat tangents eases in: -1.6875");

            // At a key every sampler is at that key's value.
            ApplyClipToBonesEXT(*rig_, *slide_, TimeSpan::FromSeconds(1.0));
            ctx.Check(std::abs(X("StepBox")) < 1e-3f && std::abs(X("LinearBox")) < 1e-3f &&
                          std::abs(X("CubicBox")) < 1e-3f,
                      "at 1 s, the second key, all three are at x = 0");

            view_ = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 7.0f), Vector3::Zero, Vector3::Up);
            projection_ = Matrix::CreatePerspectiveFieldOfView(
                0.8f, getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty(), 0.1f, 100.0f);
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            if (slide_ != nullptr)
            {
                const double t = std::fmod(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty(), 2.0);
                ApplyClipToBonesEXT(*rig_, *slide_, TimeSpan::FromSeconds(t));
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(36, 38, 48));
            device.setDepthStencilStateProperty(DepthStencilState::Default);
            rig_->Draw(Matrix::getIdentityProperty(), view_, projection_);
            DemoGame::Draw(gameTime);
        }

    private:
        // The box's x: its bone's translation (the clip writes the bones' local transforms).
        float X(const char* bone)
        {
            return rig_->getBonesProperty()[bone]->getTransformProperty().getTranslationProperty().X;
        }

        std::optional<Model> rig_;
        const AnimationClip* slide_ = nullptr;
        Matrix view_;
        Matrix projection_;
    };
}

CNA_DEMOS_GAME(ClipInterpolationDemo, {
    .id = "graphics.model.clip_interpolation",
    .title = "Animation interpolation (glTF)",
    .description = "Three boxes keyed alike with STEP, LINEAR and CUBICSPLINE glTF samplers, imported as one "
                   "node-animation clip: sampled with ApplyClipToBonesEXT at 0.25 s they are at -2 (held), -1.5 "
                   "(linear) and -1.6875 (eased).",
    .apis = {"ApplyClipToBonesEXT", "ModelAnimationsEXT", "AnimationClipEXT", "Model::Tag", "ModelBone::Transform",
             "Model::Bones"},
    .tags = {"model", "gltf", "animation", "interpolation", "cnaext"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .assets = {"models/slide_rig.cnb"},
    .knownIssue = "CNA-DEMOS-ISSUE-031",
})
