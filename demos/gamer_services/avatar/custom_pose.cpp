// SPDX-License-Identifier: MS-PL
// AvatarRenderer::Draw with game-supplied bone transforms and an AvatarExpression.
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarBodyType.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarBone.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarDescription.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarExpression.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarEye.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarEyebrow.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarMouth.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarRenderer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarRendererState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

#define CNA_DEMOS_AVATAR_BONES(APPLY) \
    APPLY(Root) APPLY(BackLower) APPLY(HipLeft) APPLY(HipRight) APPLY(BackUpper) APPLY(KneeLeft) APPLY(KneeRight) \
    APPLY(AnkleLeft) APPLY(CollarLeft) APPLY(Neck) APPLY(AnkleRight) APPLY(CollarRight) APPLY(Head) \
    APPLY(ShoulderLeft) APPLY(ToeLeft) APPLY(ShoulderRight) APPLY(ToeRight) APPLY(ElbowLeft) APPLY(ElbowRight) \
    APPLY(WristLeft) APPLY(WristRight) APPLY(FingerIndexLeft) APPLY(FingerMiddleLeft) APPLY(FingerRingLeft) \
    APPLY(FingerSmallLeft) APPLY(PropLeft) APPLY(SpecialLeft) APPLY(FingerThumbLeft) APPLY(FingerIndexRight) \
    APPLY(FingerMiddleRight) APPLY(FingerRingRight) APPLY(FingerSmallRight) APPLY(PropRight) APPLY(SpecialRight) \
    APPLY(FingerThumbRight) APPLY(FingerIndex2Left) APPLY(FingerMiddle2Left) APPLY(FingerRing2Left) \
    APPLY(FingerSmall2Left) APPLY(FingerThumb2Left) APPLY(FingerIndex2Right) APPLY(FingerMiddle2Right) \
    APPLY(FingerRing2Right) APPLY(FingerSmall2Right) APPLY(FingerThumb2Right) APPLY(FingerIndex3Left) \
    APPLY(FingerMiddle3Left) APPLY(FingerRing3Left) APPLY(FingerSmall3Left) APPLY(FingerThumb3Left) \
    APPLY(FingerIndex3Right) APPLY(FingerMiddle3Right) APPLY(FingerRing3Right) APPLY(FingerSmall3Right) \
    APPLY(FingerThumb3Right)
#define CNA_DEMOS_AVATAR_EYES(APPLY) \
    APPLY(Neutral) APPLY(Sad) APPLY(Angry) APPLY(Confused) APPLY(Laughing) APPLY(Shocked) APPLY(Happy) \
    APPLY(Yawning) APPLY(Sleeping) APPLY(LookUp) APPLY(LookDown) APPLY(LookLeft) APPLY(LookRight) APPLY(Blink)
#define CNA_DEMOS_AVATAR_MOUTHS(APPLY) \
    APPLY(Neutral) APPLY(Sad) APPLY(Angry) APPLY(Confused) APPLY(Laughing) APPLY(Shocked) APPLY(Happy) \
    APPLY(PhoneticO) APPLY(PhoneticAi) APPLY(PhoneticEe) APPLY(PhoneticFv) APPLY(PhoneticW) APPLY(PhoneticL) \
    APPLY(PhoneticDth)
#define CNA_DEMOS_AVATAR_EYEBROWS(APPLY) APPLY(Neutral) APPLY(Sad) APPLY(Angry) APPLY(Confused) APPLY(Raised)

namespace
{
#define CNA_DEMOS_AVATAR_BONE_VALUE(name) AvatarBone::name,
    constexpr std::array<AvatarBone, 55> NamedBones = {{CNA_DEMOS_AVATAR_BONES(CNA_DEMOS_AVATAR_BONE_VALUE)}};
#undef CNA_DEMOS_AVATAR_BONE_VALUE

#define CNA_DEMOS_AVATAR_EYE_VALUE(name) AvatarEye::name,
    constexpr std::array<AvatarEye, 14> EyeShapes = {{CNA_DEMOS_AVATAR_EYES(CNA_DEMOS_AVATAR_EYE_VALUE)}};
#undef CNA_DEMOS_AVATAR_EYE_VALUE

#define CNA_DEMOS_AVATAR_MOUTH_VALUE(name) AvatarMouth::name,
    constexpr std::array<AvatarMouth, 14> MouthShapes = {{CNA_DEMOS_AVATAR_MOUTHS(CNA_DEMOS_AVATAR_MOUTH_VALUE)}};
#undef CNA_DEMOS_AVATAR_MOUTH_VALUE

#define CNA_DEMOS_AVATAR_EYEBROW_VALUE(name) AvatarEyebrow::name,
    constexpr std::array<AvatarEyebrow, 5> EyebrowShapes = {{CNA_DEMOS_AVATAR_EYEBROWS(CNA_DEMOS_AVATAR_EYEBROW_VALUE)}};
#undef CNA_DEMOS_AVATAR_EYEBROW_VALUE

    bool IsFinite(const Matrix& value)
    {
        const float elements[] = {value.M11, value.M12, value.M13, value.M14, value.M21, value.M22, value.M23,
                                  value.M24, value.M31, value.M32, value.M33, value.M34, value.M41, value.M42,
                                  value.M43, value.M44};
        for (const float element : elements)
        {
            if (!std::isfinite(element))
                return false;
        }
        return true;
    }

    class CustomPoseDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            description_.emplace(AvatarDescription::CreateRandom(AvatarBodyType::Male));
            renderer_ = std::make_unique<AvatarRenderer>(&*description_, false);

            const float aspect = getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty();
            renderer_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 1.0f, 3.0f), Vector3(0.0f, 0.95f, 0.0f),
                                                            Vector3::Up));
            renderer_->setProjectionProperty(
                Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, aspect, 0.1f, 50.0f));
            renderer_->setWorldProperty(Matrix::CreateRotationY(MathHelper::Pi));

            bool expressionCatalogRoundTrips = true;
            for (const AvatarMouth mouth : MouthShapes)
            {
                expression_.setMouthProperty(mouth);
                expressionCatalogRoundTrips = expressionCatalogRoundTrips && expression_.getMouthProperty() == mouth;
            }
            for (const AvatarEye eye : EyeShapes)
            {
                expression_.setLeftEyeProperty(eye);
                expression_.setRightEyeProperty(eye);
                expressionCatalogRoundTrips = expressionCatalogRoundTrips &&
                    expression_.getLeftEyeProperty() == eye && expression_.getRightEyeProperty() == eye;
            }
            for (const AvatarEyebrow eyebrow : EyebrowShapes)
            {
                expression_.setLeftEyebrowProperty(eyebrow);
                expression_.setRightEyebrowProperty(eyebrow);
                expressionCatalogRoundTrips = expressionCatalogRoundTrips &&
                    expression_.getLeftEyebrowProperty() == eyebrow && expression_.getRightEyebrowProperty() == eyebrow;
            }
            Context().Check(expressionCatalogRoundTrips,
                            "all mouth, eye and eyebrow shapes round-trip through AvatarExpression");

            // The face: mouth, eyes and eyebrows are chosen independently.
            expression_.setMouthProperty(AvatarMouth::Happy);
            expression_.setLeftEyeProperty(AvatarEye::Happy);
            expression_.setRightEyeProperty(AvatarEye::Blink); // a wink
            expression_.setLeftEyebrowProperty(AvatarEyebrow::Raised);
            expression_.setRightEyebrowProperty(AvatarEyebrow::Neutral);
        }

        void UnloadContent() override { renderer_.reset(); }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            time_ += static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
            if (!skeletonChecked_ && renderer_->getStateProperty() == AvatarRendererState::Ready)
            {
                skeletonChecked_ = true;
                // The skeleton, available once the avatar is Ready: 71 bones, each with its parent
                // (the root has none) and its bind pose relative to that parent. A game reads
                // these to build its own animation.
                const auto parents = renderer_->getParentBonesProperty();
                const auto bindPose = renderer_->getBindPoseProperty();
                Context().Check(parents.getCountProperty() == AvatarRenderer::BoneCount &&
                                    bindPose.getCountProperty() == AvatarRenderer::BoneCount,
                                "ParentBones and BindPose describe all 71 bones");
                std::array<bool, AvatarRenderer::BoneCount> namedSlots{};
                bool everyNamedBoneMapsToSkeleton = true;
                for (const AvatarBone bone : NamedBones)
                {
                    const int slot = static_cast<int>(bone);
                    const int parent = parents[slot];
                    everyNamedBoneMapsToSkeleton = everyNamedBoneMapsToSkeleton && slot >= 0 &&
                        slot < AvatarRenderer::BoneCount && !namedSlots[static_cast<std::size_t>(slot)] &&
                        parent >= -1 && parent < slot && IsFinite(bindPose[slot]);
                    namedSlots[static_cast<std::size_t>(slot)] = true;
                }
                Context().Check(everyNamedBoneMapsToSkeleton,
                                "all 55 named AvatarBone slots map once to a finite hierarchical bind transform");
                Context().Check(parents[static_cast<int>(AvatarBone::Root)] == -1, "the root bone has no parent");
                Context().Check(parents[static_cast<int>(AvatarBone::ElbowRight)] ==
                                    static_cast<int>(AvatarBone::ShoulderRight),
                                "the right elbow hangs from the right shoulder");
            }
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                Context().Check(skeletonChecked_, "the renderer reached Ready during the run");
            }
            DemoGame::EndRun();
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(52, 44, 64));

            // Bone transforms are local rotations relative to the bind pose: identity everywhere
            // is the bind pose itself. Raise the right arm and wave the forearm.
            std::vector<Matrix> bones(AvatarRenderer::BoneCount, Matrix::getIdentityProperty());
            bones[static_cast<int>(AvatarBone::ShoulderRight)] = Matrix::CreateRotationZ(MathHelper::ToRadians(75.0f));
            bones[static_cast<int>(AvatarBone::ElbowRight)] =
                Matrix::CreateRotationZ(MathHelper::ToRadians(30.0f + 25.0f * std::sin(time_ * 6.0f)));
            bones[static_cast<int>(AvatarBone::Head)] = Matrix::CreateRotationZ(MathHelper::ToRadians(-8.0f));
            renderer_->Draw(bones, expression_);

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "AvatarRenderer.Draw(bones, expression)", Vector2(20.0f, 16.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "mouth Happy, left eye Happy, right eye Blink, left brow Raised",
                                     Vector2(20.0f, 44.0f), Color::LightGray);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::optional<AvatarDescription> description_;
        std::unique_ptr<AvatarRenderer> renderer_;
        AvatarExpression expression_;
        float time_ = 0.0f;
        bool skeletonChecked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

#define CNA_DEMOS_AVATAR_BONE_API(name) "AvatarBone::" #name,
#define CNA_DEMOS_AVATAR_EYE_API(name) "AvatarEye::" #name,
#define CNA_DEMOS_AVATAR_MOUTH_API(name) "AvatarMouth::" #name,
#define CNA_DEMOS_AVATAR_EYEBROW_API(name) "AvatarEyebrow::" #name,

CNA_DEMOS_GAME(CustomPoseDemo, {
    .id = "gamer_services.avatar.custom_pose",
    .title = "Avatar pose and expression",
    .description = "Draws an avatar from game-supplied bone transforms -- the right arm raised and waving through "
                   "AvatarBone indices -- with an AvatarExpression set per feature (smile, wink, raised brow); "
                   "once the avatar is Ready, ParentBones and BindPose describe the 71-bone skeleton.",
    .apis = {"AvatarRenderer::Draw", "AvatarRenderer::ParentBones", "AvatarRenderer::BindPose",
             "AvatarRenderer::BoneCount", "AvatarBone",
             CNA_DEMOS_AVATAR_BONES(CNA_DEMOS_AVATAR_BONE_API)
             "AvatarExpression::Mouth", "AvatarExpression::LeftEye",
             "AvatarExpression::RightEye", "AvatarExpression::LeftEyebrow", "AvatarExpression::RightEyebrow",
             "AvatarMouth", CNA_DEMOS_AVATAR_MOUTHS(CNA_DEMOS_AVATAR_MOUTH_API)
             "AvatarEye", CNA_DEMOS_AVATAR_EYES(CNA_DEMOS_AVATAR_EYE_API)
             "AvatarEyebrow", CNA_DEMOS_AVATAR_EYEBROWS(CNA_DEMOS_AVATAR_EYEBROW_API)
             "AvatarRenderer::World", "AvatarRenderer::View",
             "AvatarRenderer::Projection", "AvatarRenderer::State"},
    .tags = {"gamer-services", "avatar", "3d", "skeleton", "bones", "face"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 60,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})

#undef CNA_DEMOS_AVATAR_BONE_API
#undef CNA_DEMOS_AVATAR_EYE_API
#undef CNA_DEMOS_AVATAR_MOUTH_API
#undef CNA_DEMOS_AVATAR_EYEBROW_API
#undef CNA_DEMOS_AVATAR_BONES
#undef CNA_DEMOS_AVATAR_EYES
#undef CNA_DEMOS_AVATAR_MOUTHS
#undef CNA_DEMOS_AVATAR_EYEBROWS
