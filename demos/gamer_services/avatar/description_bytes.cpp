// SPDX-License-Identifier: MS-PL
// AvatarDescription::Description: the bytes a game saves or sends to recreate an avatar.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarAnimation.hpp"
#include "Microsoft/Xna/Framework/GamerServices/AvatarAnimationPreset.hpp"
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
#include "System/ArgumentException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class DescriptionBytesDemo final : public CnaDemos::GamerServicesDemo
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // The original, and a copy rebuilt only from its bytes -- as a saved game or another
            // player in a network session would receive them.
            original_.emplace(AvatarDescription::CreateRandom());
            const std::vector<SharpRuntime::bytecs> bytes = original_->getDescriptionProperty();
            copy_.emplace(bytes);
            ctx.Print("Description: {} bytes", bytes.size());
            ctx.Check(bytes.size() == 1021, "a description is 1021 bytes, as on XNA");
            ctx.Check(copy_->getIsValidProperty() && copy_->getDescriptionProperty() == bytes &&
                          copy_->getBodyTypeProperty() == original_->getBodyTypeProperty() &&
                          copy_->getHeightProperty() == original_->getHeightProperty(),
                      "the copy made from the bytes is the same avatar (body type, height, bytes)");

            // Bytes that are not an avatar make an invalid description, drawn as nothing.
            blank_.emplace(std::vector<SharpRuntime::bytecs>(1021, 0));
            ctx.Check(!blank_->getIsValidProperty(), "1021 zero bytes are not a valid description");
            bool refused = false;
            try
            {
                const AvatarDescription tooShort(std::vector<SharpRuntime::bytecs>(16, 1));
            }
            catch (const System::ArgumentException&)
            {
                refused = true;
            }
            ctx.Check(refused, "a buffer of another length throws ArgumentException");

            const float aspect = getGraphicsDeviceProperty().getViewportProperty().getAspectRatioProperty();
            const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 1.0f, 4.2f), Vector3(0.0f, 0.9f, 0.0f), Vector3::Up);
            const Matrix projection = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, aspect, 0.1f, 50.0f);
            const float x[] = {-1.1f, 0.0f, 1.1f};
            AvatarDescription* descriptions[] = {&*original_, &*copy_, &*blank_};
            for (int i = 0; i < 3; ++i)
            {
                renderers_[i] = std::make_unique<AvatarRenderer>(descriptions[i], false);
                renderers_[i]->setWorldProperty(Matrix::CreateTranslation(x[i], 0.0f, 0.0f));
                renderers_[i]->setViewProperty(view);
                renderers_[i]->setProjectionProperty(projection);
            }
            ctx.Check(renderers_[2]->getStateProperty() == AvatarRendererState::Unavailable,
                      "a renderer of an invalid description is Unavailable");
            animation_ = std::make_unique<AvatarAnimation>(AvatarAnimationPreset::Stand0);
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
            animation_->Update(gameTime.getElapsedGameTimeProperty(), true);
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(36, 52, 60));
            for (auto& renderer : renderers_)
            {
                renderer->Draw(animation_.get());
            }
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "AvatarDescription(Description bytes)", Vector2(20.0f, 16.0f), Color::White);
            spriteBatch_->DrawString(*font_, "original", Vector2(130.0f, 60.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "from bytes", Vector2(340.0f, 60.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, "invalid", Vector2(570.0f, 60.0f), Color::LightGray);
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::optional<AvatarDescription> original_;
        std::optional<AvatarDescription> copy_;
        std::optional<AvatarDescription> blank_;
        std::unique_ptr<AvatarRenderer> renderers_[3];
        std::unique_ptr<AvatarAnimation> animation_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(DescriptionBytesDemo, {
    .id = "gamer_services.avatar.description_bytes",
    .title = "Avatar description bytes",
    .description = "Saves a random avatar as its 1021 Description bytes and rebuilds an identical AvatarDescription "
                   "from them, drawn beside the original; zero bytes make an invalid description whose renderer is "
                   "Unavailable, and a buffer of the wrong size is refused.",
    .apis = {"AvatarDescription::Description", "AvatarDescription::AvatarDescription", "AvatarDescription::IsValid",
             "AvatarDescription::CreateRandom", "AvatarRenderer::State", "AvatarRendererState"},
    .tags = {"gamer-services", "avatar", "serialization", "network", "save"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 45,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
