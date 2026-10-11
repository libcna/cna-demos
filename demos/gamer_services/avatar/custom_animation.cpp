// SPDX-License-Identifier: MS-PL
// IAvatarAnimation: a game's own avatar animation, drawn by AvatarRenderer like the stock ones.
#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarBodyType.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarBone.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarDescription.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarExpression.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarEye.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarMouth.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarRenderer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarRendererState.hpp"
#include "Microsoft/Xna/Framework/GamerServices/IAvatarAnimation.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Collections/ObjectModel/ReadOnlyCollection.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Color Background(40, 54, 70);

    // A one-second cheer computed in code: both arms up and pumping, the head bobbing, the mouth
    // laughing on the beat. Anything that implements IAvatarAnimation -- keyframes from a file, a
    // ragdoll, a blend of two clips -- is drawn the same way.
    class CheerAnimation final : public IAvatarAnimation
    {
    public:
        CheerAnimation() { Pose(); }

        [[nodiscard]] System::Collections::ObjectModel::ReadOnlyCollection<Matrix> getBoneTransformsProperty()
            const override
        {
            return System::Collections::ObjectModel::ReadOnlyCollection<Matrix>(bones_);
        }

        [[nodiscard]] System::TimeSpan getCurrentPositionProperty() const override { return position_; }

        void setCurrentPositionProperty(System::TimeSpan value) override
        {
            position_ = value;
            Pose();
        }

        [[nodiscard]] System::TimeSpan getLengthProperty() const override { return System::TimeSpan::FromSeconds(1.0); }

        [[nodiscard]] AvatarExpression getExpressionProperty() const override { return expression_; }

        // XNA's contract: advance by the elapsed time; past the end, wrap when looping, otherwise
        // stop at the last frame.
        void Update(System::TimeSpan elapsedAnimationTime, bool loop) override
        {
            const double length = getLengthProperty().getTotalSecondsProperty();
            double seconds = position_.getTotalSecondsProperty() + elapsedAnimationTime.getTotalSecondsProperty();
            seconds = loop ? std::fmod(seconds, length) : std::min(seconds, length);
            setCurrentPositionProperty(System::TimeSpan::FromSeconds(seconds));
        }

    private:
        // Bone transforms are rotations relative to the bind pose, one per AvatarBone.
        void Pose()
        {
            const float beat = static_cast<float>(std::sin(position_.getTotalSecondsProperty() * MathHelper::TwoPi));
            const float raise = MathHelper::ToRadians(120.0f + 20.0f * beat);
            const float bend = MathHelper::ToRadians(25.0f + 25.0f * beat);
            bones_.assign(AvatarRenderer::BoneCount, Matrix::getIdentityProperty());
            bones_[static_cast<int>(AvatarBone::ShoulderRight)] = Matrix::CreateRotationZ(raise);
            bones_[static_cast<int>(AvatarBone::ElbowRight)] = Matrix::CreateRotationZ(bend);
            bones_[static_cast<int>(AvatarBone::ShoulderLeft)] = Matrix::CreateRotationZ(-raise);
            bones_[static_cast<int>(AvatarBone::ElbowLeft)] = Matrix::CreateRotationZ(-bend);
            bones_[static_cast<int>(AvatarBone::Head)] = Matrix::CreateRotationX(MathHelper::ToRadians(10.0f * beat));
            expression_.setMouthProperty(beat > 0.0f ? AvatarMouth::Laughing : AvatarMouth::Happy);
            expression_.setLeftEyeProperty(AvatarEye::Happy);
            expression_.setRightEyeProperty(AvatarEye::Happy);
        }

        System::TimeSpan position_ = System::TimeSpan::Zero;
        std::vector<Matrix> bones_;
        AvatarExpression expression_;
    };

    // Pixels that differ between two captures of the screen.
    int Differences(const CnaDemos::BackBufferPixels& a, const CnaDemos::BackBufferPixels& b)
    {
        int count = 0;
        for (int y = 0; y < a.Height(); ++y)
            for (int x = 0; x < a.Width(); ++x)
                if (!CnaDemos::Near(a.At(x, y), b.At(x, y), 0))
                    ++count;
        return count;
    }

    class CustomAnimationDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            description_.emplace(AvatarDescription::CreateRandom(AvatarBodyType::Female));
            renderer_ = std::make_unique<AvatarRenderer>(&*description_, false);

            const float aspect = getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty();
            renderer_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 1.0f, 3.2f), Vector3(0.0f, 1.0f, 0.0f),
                                                            Vector3::Up));
            renderer_->setProjectionProperty(
                Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, aspect, 0.1f, 50.0f));
            renderer_->setWorldProperty(Matrix::CreateRotationY(MathHelper::Pi));
        }

        void UnloadContent() override { renderer_.reset(); }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            // As with the stock AvatarAnimation, the game advances the clip once the avatar is
            // Ready, so it starts from the beginning when the avatar appears.
            if (renderer_->getStateProperty() == AvatarRendererState::Ready)
            {
                cheer_.Update(gameTime.getElapsedGameTimeProperty(), true);
                ++readyUpdates_;
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            if (!checked_ && readyUpdates_ >= 3)
            {
                checked_ = true;
                Compare(device);
            }

            device.Clear(Background);
            renderer_->Draw(&cheer_); // the renderer asks the animation for its bones and expression
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "AvatarRenderer.Draw(IAvatarAnimation*)", Vector2(20.0f, 16.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "CheerAnimation: the game's own IAvatarAnimation", Vector2(20.0f, 44.0f),
                                     Color::LightGray);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                Context().Check(checked_, "the renderer reached Ready during the run");
            }
            DemoGame::EndRun();
        }

    private:
        // Draws the same frame three ways and compares the captures.
        void Compare(GraphicsDevice& device)
        {
            device.Clear(Background);
            renderer_->Draw(&cheer_);
            const CnaDemos::BackBufferPixels throughInterface(device);

            const auto transforms = cheer_.getBoneTransformsProperty();
            const std::vector<Matrix> bones(transforms.begin(), transforms.end());
            device.Clear(Background);
            renderer_->Draw(bones, cheer_.getExpressionProperty());
            const CnaDemos::BackBufferPixels throughBones(device);

            device.Clear(Background);
            renderer_->Draw(std::vector<Matrix>(AvatarRenderer::BoneCount, Matrix::getIdentityProperty()),
                            AvatarExpression());
            const CnaDemos::BackBufferPixels bindPose(device);

            const int sameFrame = Differences(throughInterface, throughBones);
            const int moved = Differences(throughInterface, bindPose);
            Context().Print("at {:.2f} s of {:.0f} s: {} pixels differ from the bones drawn directly, {} from the "
                            "bind pose",
                            cheer_.getCurrentPositionProperty().getTotalSecondsProperty(),
                            cheer_.getLengthProperty().getTotalSecondsProperty(), sameFrame, moved);
            Context().Check(sameFrame == 0,
                            "Draw(IAvatarAnimation*) draws the animation's BoneTransforms and Expression");
            Context().Check(moved > 1000, "the cheer moves the avatar away from its bind pose");
        }

        std::optional<AvatarDescription> description_;
        std::unique_ptr<AvatarRenderer> renderer_;
        CheerAnimation cheer_;
        int readyUpdates_ = 0;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(CustomAnimationDemo, {
    .id = "gamer_services.avatar.custom_animation",
    .title = "Your own avatar animation",
    .description = "Implements IAvatarAnimation -- BoneTransforms, Expression, Length, CurrentPosition and "
                   "Update(elapsed, loop) -- as a looping cheer computed in code and draws it with "
                   "AvatarRenderer::Draw(IAvatarAnimation*), which renders exactly what the bones and expression "
                   "say.",
    .apis = {"IAvatarAnimation", "IAvatarAnimation::BoneTransforms", "IAvatarAnimation::Expression",
             "IAvatarAnimation::Length", "IAvatarAnimation::CurrentPosition", "IAvatarAnimation::Update",
             "AvatarRenderer::Draw", "AvatarRenderer::World", "AvatarRenderer::View", "AvatarRenderer::Projection",
             "AvatarRenderer::State"},
    .tags = {"gamer-services", "avatar", "3d", "animation", "interface"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 40,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
