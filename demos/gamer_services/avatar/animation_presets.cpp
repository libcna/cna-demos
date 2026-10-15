// SPDX-License-Identifier: MS-PL
// AvatarAnimationPreset: the standard animations, several avatars at once.
#include <array>
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarAnimation.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarAnimationPreset.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarBodyType.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarDescription.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarEye.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarEyebrow.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarMouth.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarRenderer.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

#define CNA_DEMOS_AVATAR_ANIMATION_PRESETS(APPLY) \
    APPLY(Stand0) APPLY(Stand1) APPLY(Stand2) APPLY(Stand3) APPLY(Stand4) APPLY(Stand5) APPLY(Stand6) APPLY(Stand7) \
    APPLY(Clap) APPLY(Wave) APPLY(Celebrate) \
    APPLY(FemaleIdleCheckNails) APPLY(FemaleIdleLookAround) APPLY(FemaleIdleShiftWeight) APPLY(FemaleIdleFixShoe) \
    APPLY(FemaleAngry) APPLY(FemaleConfused) APPLY(FemaleLaugh) APPLY(FemaleCry) APPLY(FemaleShocked) APPLY(FemaleYawn) \
    APPLY(MaleIdleLookAround) APPLY(MaleIdleStretch) APPLY(MaleIdleShiftWeight) APPLY(MaleIdleCheckHand) \
    APPLY(MaleAngry) APPLY(MaleConfused) APPLY(MaleLaugh) APPLY(MaleCry) APPLY(MaleSurprised) APPLY(MaleYawn)

namespace
{
    struct Slot
    {
        AvatarAnimationPreset preset;
        AvatarBodyType body;
        const char* label;
    };

    constexpr std::array<Slot, 6> Slots = {{
        {AvatarAnimationPreset::Clap, AvatarBodyType::Female, "Clap"},
        {AvatarAnimationPreset::Celebrate, AvatarBodyType::Male, "Celebrate"},
        {AvatarAnimationPreset::FemaleLaugh, AvatarBodyType::Female, "FemaleLaugh"},
        {AvatarAnimationPreset::MaleConfused, AvatarBodyType::Male, "MaleConfused"},
        {AvatarAnimationPreset::FemaleIdleLookAround, AvatarBodyType::Female, "FemaleIdleLookAround"},
        {AvatarAnimationPreset::MaleYawn, AvatarBodyType::Male, "MaleYawn"},
    }};

#define CNA_DEMOS_AVATAR_ANIMATION_VALUE(name) AvatarAnimationPreset::name,
    constexpr std::array<AvatarAnimationPreset, 31> AllPresets = {{
        CNA_DEMOS_AVATAR_ANIMATION_PRESETS(CNA_DEMOS_AVATAR_ANIMATION_VALUE)
    }};
#undef CNA_DEMOS_AVATAR_ANIMATION_VALUE

    class AnimationPresetsDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            const float aspect = getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty();
            const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 1.0f, 6.0f), Vector3(0.0f, 0.85f, 0.0f), Vector3::Up);
            const Matrix projection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, aspect, 0.1f, 50.0f);
            for (std::size_t i = 0; i < Slots.size(); ++i)
            {
                descriptions_[i].emplace(AvatarDescription::CreateRandom(Slots[i].body));
                Context().Check(descriptions_[i]->getBodyTypeProperty() == Slots[i].body,
                                std::string(Slots[i].label) + " preserves its requested body type");
                renderers_[i] = std::make_unique<AvatarRenderer>(&*descriptions_[i], true);
                // An avatar faces -Z: turned by pi, it faces the camera.
                renderers_[i]->setWorldProperty(Matrix::CreateRotationY(MathHelper::Pi) *
                                                Matrix::CreateTranslation(-3.4f + 1.36f * static_cast<float>(i), 0.0f, 0.0f));
                renderers_[i]->setViewProperty(view);
                renderers_[i]->setProjectionProperty(projection);
                animations_[i] = std::make_unique<AvatarAnimation>(Slots[i].preset);
                const double seconds = animations_[i]->getLengthProperty().getTotalSecondsProperty();
                Context().Print("{}: {:.2f} s", Slots[i].label, seconds);
                Context().Check(seconds > 0.0, std::string(Slots[i].label) + " has a length");
                // Start each clip mid-gesture rather than in its neutral first pose: drawing six
                // avatars is slow on the SOFTWARE renderer, so an automated run is only a few frames.
                animations_[i]->setCurrentPositionProperty(System::TimeSpan::FromSeconds(seconds * 0.4));
            }

            // Each public preset selects a distinct stock clip. Sample every one at its midpoint
            // and validate the complete renderer-facing payload instead of merely naming the enum.
            bool allPresetsSampled = true;
            long long totalLengthTicks = 0;
            for (const AvatarAnimationPreset preset : AllPresets)
            {
                AvatarAnimation clip(preset);
                const long long lengthTicks = clip.getLengthProperty().getTicksProperty();
                const auto midpoint = System::TimeSpan::FromTicks(lengthTicks / 2);
                clip.setCurrentPositionProperty(midpoint);
                const auto expression = clip.getExpressionProperty();
                allPresetsSampled = allPresetsSampled && lengthTicks > 0 &&
                    clip.getCurrentPositionProperty().getTicksProperty() == midpoint.getTicksProperty() &&
                    clip.getBoneTransformsProperty().getCountProperty() == AvatarRenderer::BoneCount &&
                    static_cast<int>(expression.getMouthProperty()) >= static_cast<int>(AvatarMouth::Neutral) &&
                    static_cast<int>(expression.getMouthProperty()) <= static_cast<int>(AvatarMouth::PhoneticDth) &&
                    static_cast<int>(expression.getLeftEyeProperty()) >= static_cast<int>(AvatarEye::Neutral) &&
                    static_cast<int>(expression.getLeftEyeProperty()) <= static_cast<int>(AvatarEye::Blink) &&
                    static_cast<int>(expression.getRightEyeProperty()) >= static_cast<int>(AvatarEye::Neutral) &&
                    static_cast<int>(expression.getRightEyeProperty()) <= static_cast<int>(AvatarEye::Blink) &&
                    static_cast<int>(expression.getLeftEyebrowProperty()) >= static_cast<int>(AvatarEyebrow::Neutral) &&
                    static_cast<int>(expression.getLeftEyebrowProperty()) <= static_cast<int>(AvatarEyebrow::Raised) &&
                    static_cast<int>(expression.getRightEyebrowProperty()) >= static_cast<int>(AvatarEyebrow::Neutral) &&
                    static_cast<int>(expression.getRightEyebrowProperty()) <= static_cast<int>(AvatarEyebrow::Raised);
                totalLengthTicks += lengthTicks;
            }
            Context().Check(allPresetsSampled && totalLengthTicks > 0,
                            "all 31 stock presets provide timed skeleton and expression samples");

            // Inspect one independent stock clip at a documented keyed frame. BoneTransforms and
            // Expression are the values AvatarRenderer consumes; this instance is kept separate so
            // its lifetime checks cannot disturb the six animations still being drawn.
            AvatarAnimation sampled(AvatarAnimationPreset::Celebrate);
            sampled.setCurrentPositionProperty(System::TimeSpan::FromSeconds(1.0));
            const auto bones = sampled.getBoneTransformsProperty();
            bool hasAnimatedBone = false;
            for (int bone = 0; bone < bones.getCountProperty(); ++bone)
            {
                hasAnimatedBone = hasAnimatedBone || bones[bone] != Matrix::getIdentityProperty();
            }
            Context().Check(bones.getCountProperty() == AvatarRenderer::BoneCount && hasAnimatedBone,
                            "BoneTransforms exposes all 71 sampled local transforms");

            const auto expression = sampled.getExpressionProperty();
            Context().Check(expression.getMouthProperty() == AvatarMouth::Laughing &&
                                expression.getLeftEyeProperty() == AvatarEye::Laughing &&
                                expression.getRightEyeProperty() == AvatarEye::Laughing &&
                                expression.getLeftEyebrowProperty() == AvatarEyebrow::Raised &&
                                expression.getRightEyebrowProperty() == AvatarEyebrow::Raised,
                            "Expression follows Celebrate's keyed laughing frame");

            const bool startedLive = !sampled.getIsDisposedProperty();
            sampled.Dispose();
            sampled.Dispose();
            bool disposedUpdateRefused = false;
            try
            {
                sampled.Update(System::TimeSpan::Zero, false);
            }
            catch (const System::ObjectDisposedException&)
            {
                disposedUpdateRefused = true;
            }
            Context().Check(startedLive && sampled.getIsDisposedProperty() && disposedUpdateRefused,
                            "Dispose is idempotent, sets IsDisposed, and guards Update");
        }

        void UnloadContent() override
        {
            for (auto& renderer : renderers_)
            {
                renderer.reset();
            }
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            for (auto& animation : animations_)
            {
                animation->Update(gameTime.getElapsedGameTimeProperty(), true);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(44, 48, 58));
            for (std::size_t i = 0; i < Slots.size(); ++i)
            {
                renderers_[i]->Draw(animations_[i].get());
            }
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "AvatarAnimationPreset", Vector2(20.0f, 16.0f), Color::White);
            const float width = static_cast<float>(getGraphicsDeviceProperty().getViewportProperty().getWidthProperty());
            for (std::size_t i = 0; i < Slots.size(); ++i)
            {
                const std::string label = Slots[i].label;
                const float column = width * (static_cast<float>(i) + 0.5f) / static_cast<float>(Slots.size());
                const float y = 380.0f + 24.0f * static_cast<float>(i % 2);
                spriteBatch_->DrawString(*font_, label, Vector2(column - font_->MeasureString(label).X * 0.5f, y),
                                         Color::LightGray);
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::array<std::optional<AvatarDescription>, Slots.size()> descriptions_;
        std::array<std::unique_ptr<AvatarRenderer>, Slots.size()> renderers_;
        std::array<std::unique_ptr<AvatarAnimation>, Slots.size()> animations_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

#define CNA_DEMOS_AVATAR_ANIMATION_API(name) "AvatarAnimationPreset::" #name,

CNA_DEMOS_GAME(AnimationPresetsDemo, {
    .id = "gamer_services.avatar.animation_presets",
    .title = "Avatar animation presets",
    .description = "Six random avatars play six of the 31 standard AvatarAnimationPreset clips side by side -- clap, "
                   "celebrate, laugh, confused, an idle and a yawn -- each with its own facial expression.",
    .apis = {"AvatarAnimationPreset",
             CNA_DEMOS_AVATAR_ANIMATION_PRESETS(CNA_DEMOS_AVATAR_ANIMATION_API)
             "AvatarAnimation::AvatarAnimation", "AvatarAnimation::Update",
             "AvatarAnimation::Length", "AvatarAnimation::CurrentPosition", "AvatarAnimation::BoneTransforms",
             "AvatarAnimation::Expression", "AvatarAnimation::Dispose", "AvatarAnimation::IsDisposed",
             "AvatarRenderer::Draw",
             "AvatarDescription::CreateRandom",
             "AvatarDescription::BodyType", "AvatarBodyType", "AvatarBodyType::Female", "AvatarBodyType::Male",
             "AvatarRenderer::World", "AvatarRenderer::View", "AvatarRenderer::Projection"},
    .tags = {"gamer-services", "avatar", "3d", "animation", "gallery"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 12,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})

#undef CNA_DEMOS_AVATAR_ANIMATION_API
#undef CNA_DEMOS_AVATAR_ANIMATION_PRESETS
