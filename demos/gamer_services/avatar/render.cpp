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
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Color Background(40, 56, 80);
    const Vector3 KeyColor(0.82f, 0.76f, 0.68f);
    const Vector3 KeyDirection(-0.4f, -0.5f, -0.75f);
    const Vector3 AmbientColor(0.24f, 0.27f, 0.34f);

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
            renderer_->setLightColorProperty(KeyColor);
            renderer_->setLightDirectionProperty(KeyDirection);
            renderer_->setAmbientLightColorProperty(AmbientColor);
            Context().Check(renderer_->getLightColorProperty() == KeyColor &&
                                renderer_->getLightDirectionProperty() == KeyDirection &&
                                renderer_->getAmbientLightColorProperty() == AmbientColor,
                            "the directional and ambient light properties round-trip exactly");
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
            if (renderer_->getStateProperty() == AvatarRendererState::Ready)
            {
                ++readyUpdates_;
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Background);
            // Drawing while Loading draws the loading effect (the renderer was created with it).
            renderer_->Draw(animation_.get());

            if (readyUpdates_ > 0 && !checked_)
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
                CheckLighting(pixels);
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
            if (renderer_)
            {
                const bool startedLive = !renderer_->getIsDisposedProperty();
                renderer_->Dispose();
                renderer_->Dispose();
                bool stateRefused = false;
                bool drawRefused = false;
                try
                {
                    (void)renderer_->getStateProperty();
                }
                catch (const System::ObjectDisposedException&)
                {
                    stateRefused = true;
                }
                try
                {
                    renderer_->Draw(animation_.get());
                }
                catch (const System::ObjectDisposedException&)
                {
                    drawRefused = true;
                }
                Context().Check(startedLive && renderer_->getIsDisposedProperty() && stateRefused && drawRefused,
                                "Dispose is idempotent, sets IsDisposed, and guards State and Draw");
            }
            DemoGame::EndRun();
        }

    private:
        static int Differences(const CnaDemos::BackBufferPixels& a, const CnaDemos::BackBufferPixels& b)
        {
            int count = 0;
            for (int y = 0; y < a.Height(); y += 2)
            {
                for (int x = 0; x < a.Width(); x += 2)
                {
                    count += CnaDemos::Near(a.At(x, y), b.At(x, y), 2) ? 0 : 1;
                }
            }
            return count;
        }

        void CheckLighting(const CnaDemos::BackBufferPixels& lit)
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            renderer_->setLightColorProperty(Vector3::Zero);
            renderer_->setAmbientLightColorProperty(Vector3::Zero);
            device.Clear(Background);
            renderer_->Draw(animation_.get());
            const CnaDemos::BackBufferPixels unlit(device);

            renderer_->setLightColorProperty(KeyColor);
            renderer_->setAmbientLightColorProperty(AmbientColor);
            renderer_->setLightDirectionProperty(Vector3(-KeyDirection.X, -KeyDirection.Y, -KeyDirection.Z));
            device.Clear(Background);
            renderer_->Draw(animation_.get());
            const CnaDemos::BackBufferPixels reversed(device);

            const int colorChanges = Differences(lit, unlit);
            const int directionChanges = Differences(lit, reversed);
            Context().Print("lighting changes {} sampled pixels; reversing its direction changes {}", colorChanges,
                            directionChanges);
            Context().Check(colorChanges > 250, "LightColor and AmbientLightColor affect the rendered avatar");
            Context().Check(directionChanges > 100, "LightDirection changes the avatar's shading");

            renderer_->setLightDirectionProperty(KeyDirection);
            device.Clear(Background);
            renderer_->Draw(animation_.get());
        }

        std::optional<AvatarDescription> description_;
        std::unique_ptr<AvatarRenderer> renderer_;
        std::unique_ptr<AvatarAnimation> animation_;
        float yaw_ = 0.0f;
        int readyUpdates_ = 0;
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
             "AvatarRenderer::LightColor", "AvatarRenderer::LightDirection", "AvatarRenderer::AmbientLightColor",
             "AvatarRenderer::Dispose", "AvatarRenderer::IsDisposed",
             "AvatarAnimation::AvatarAnimation", "AvatarAnimation::Update", "AvatarAnimation::Length",
             "AvatarAnimationPreset", "AvatarRendererState"},
    .tags = {"gamer-services", "avatar", "3d", "animation", "character"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 90,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
