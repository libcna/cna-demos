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
#include "Microsoft/Xna/Framework/GamerServices/AvatarRenderer.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

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

CNA_DEMOS_GAME(AnimationPresetsDemo, {
    .id = "gamer_services.avatar.animation_presets",
    .title = "Avatar animation presets",
    .description = "Six random avatars play six of the 31 standard AvatarAnimationPreset clips side by side -- clap, "
                   "celebrate, laugh, confused, an idle and a yawn -- each with its own facial expression.",
    .apis = {"AvatarAnimationPreset", "AvatarAnimation::AvatarAnimation", "AvatarAnimation::Update",
             "AvatarAnimation::Length", "AvatarAnimation::CurrentPosition", "AvatarRenderer::Draw",
             "AvatarDescription::CreateRandom"},
    .tags = {"gamer-services", "avatar", "3d", "animation", "gallery"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 12,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
