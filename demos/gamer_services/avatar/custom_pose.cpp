// SPDX-License-Identifier: MS-PL
// AvatarRenderer::Draw with game-supplied bone transforms and an AvatarExpression.
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

namespace
{
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
                Context().Check(parents.getCountProperty() == AvatarRenderer::BoneCount &&
                                    renderer_->getBindPoseProperty().getCountProperty() == AvatarRenderer::BoneCount,
                                "ParentBones and BindPose describe all 71 bones");
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

CNA_DEMOS_GAME(CustomPoseDemo, {
    .id = "gamer_services.avatar.custom_pose",
    .title = "Avatar pose and expression",
    .description = "Draws an avatar from game-supplied bone transforms -- the right arm raised and waving through "
                   "AvatarBone indices -- with an AvatarExpression set per feature (smile, wink, raised brow); "
                   "once the avatar is Ready, ParentBones and BindPose describe the 71-bone skeleton.",
    .apis = {"AvatarRenderer::Draw", "AvatarRenderer::ParentBones", "AvatarRenderer::BindPose",
             "AvatarRenderer::BoneCount", "AvatarBone", "AvatarExpression::Mouth", "AvatarExpression::LeftEye",
             "AvatarExpression::RightEye", "AvatarExpression::LeftEyebrow", "AvatarExpression::RightEyebrow",
             "AvatarMouth", "AvatarEye", "AvatarEyebrow"},
    .tags = {"gamer-services", "avatar", "3d", "skeleton", "bones", "face"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 60,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
