// SPDX-License-Identifier: MS-PL
// AvatarRenderer and AvatarAnimation: drawing a standard avatar playing a preset animation.
#include <memory>
#include <optional>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarAnimation.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarAnimationPreset.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarBodyType.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarDescription.hpp"
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
    const Color Background(40, 56, 80);

    std::string StateName(AvatarRendererState state)
    {
        switch (state)
        {
            case AvatarRendererState::Loading: return "Loading";
            case AvatarRendererState::Ready: return "Ready";
            case AvatarRendererState::Unavailable: return "Unavailable";
        }
        return "?";
    }

    class AvatarRenderDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // A random avatar from CNA's built-in catalog (the bytes in Description are what a
            // game would save or send to other players), drawn by the standard renderer.
            description_.emplace(AvatarDescription::CreateRandom(AvatarBodyType::Female));
            Context().Check(description_->getIsValidProperty() &&
                                description_->getBodyTypeProperty() == AvatarBodyType::Female,
                            "CreateRandom made a valid female avatar description");
            renderer_ = std::make_unique<AvatarRenderer>(&*description_, true);
            animation_ = std::make_unique<AvatarAnimation>(AvatarAnimationPreset::Wave);
            Context().Print("height {:.2f} m, Wave lasts {:.2f} s, renderer {}", description_->getHeightProperty(),
                            animation_->getLengthProperty().getTotalSecondsProperty(),
                            StateName(renderer_->getStateProperty()));

            const float aspect = getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty();
            renderer_->setViewProperty(Matrix::CreateLookAt(Vector3(0.0f, 1.0f, 3.2f), Vector3(0.0f, 0.9f, 0.0f),
                                                            Vector3::Up));
            renderer_->setProjectionProperty(
                Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, aspect, 0.1f, 50.0f));
        }

        void UnloadContent() override
        {
            animation_.reset();
            renderer_.reset();
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            // The animation advances with game time and loops.
            animation_->Update(gameTime.getElapsedGameTimeProperty(), true);
            // Turn slowly so every side is seen.
            yaw_ += 0.4f * static_cast<float>(gameTime.getElapsedGameTimeProperty().getTotalSecondsProperty());
            renderer_->setWorldProperty(Matrix::CreateRotationY(yaw_));
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Background);
            // Drawing while Loading draws the loading effect (the renderer was created with it).
            renderer_->Draw(animation_.get());

            if (renderer_->getStateProperty() == AvatarRendererState::Ready && !checked_)
            {
                checked_ = true;
                const CnaDemos::BackBufferPixels pixels(getGraphicsDeviceProperty());
                int covered = 0;
                for (int y = pixels.Height() / 4; y < pixels.Height(); y += 4)
                {
                    const Color c = pixels.At(pixels.Width() / 2, y);
                    covered += CnaDemos::Near(c, Background, 4) ? 0 : 1;
                }
                Context().Check(covered > 10, "the avatar covers the centre of the screen once Ready");
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "AvatarRenderer.Draw(AvatarAnimation Wave)", Vector2(20.0f, 16.0f),
                                     Color::White);
            spriteBatch_->DrawString(*font_, "State: " + StateName(renderer_->getStateProperty()), Vector2(20.0f, 44.0f),
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
        std::optional<AvatarDescription> description_;
        std::unique_ptr<AvatarRenderer> renderer_;
        std::unique_ptr<AvatarAnimation> animation_;
        float yaw_ = 0.0f;
        bool checked_ = false;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(AvatarRenderDemo, {
    .id = "gamer_services.avatar.render",
    .title = "Avatar with an animation",
    .description = "Creates a random female avatar with AvatarDescription::CreateRandom and draws it with "
                   "AvatarRenderer playing the Wave preset (AvatarAnimation), turning slowly; the renderer shows a "
                   "loading effect until its State is Ready.",
    .apis = {"AvatarDescription::CreateRandom", "AvatarDescription::IsValid", "AvatarDescription::BodyType",
             "AvatarDescription::Height", "AvatarRenderer::AvatarRenderer", "AvatarRenderer::Draw",
             "AvatarRenderer::State", "AvatarRenderer::World", "AvatarRenderer::View", "AvatarRenderer::Projection",
             "AvatarAnimation::AvatarAnimation", "AvatarAnimation::Update", "AvatarAnimation::Length",
             "AvatarAnimationPreset", "AvatarRendererState"},
    .tags = {"gamer-services", "avatar", "3d", "animation", "character"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 90,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
