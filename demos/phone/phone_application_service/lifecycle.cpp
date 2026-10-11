// SPDX-License-Identifier: MS-PL
// PhoneApplicationService: Launching, Deactivated, Activated and Closing, and the State dictionary.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Phone/Shell/ActivatedEventArgs.hpp"
#include "Microsoft/Phone/Shell/ClosingEventArgs.hpp"
#include "Microsoft/Phone/Shell/DeactivatedEventArgs.hpp"
#include "Microsoft/Phone/Shell/LaunchingEventArgs.hpp"
#include "Microsoft/Phone/Shell/PhoneApplicationService.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Object.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Phone::Shell;

namespace
{
    // What the game keeps across a trip to the background.
    struct Progress : System::Object
    {
        int level = 0;
        int score = 0;

        [[nodiscard]] const std::string& GetTypeName() const override
        {
            static const std::string name = "Progress";
            return name;
        }
    };

    class LifecycleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            PhoneApplicationService& service = PhoneApplicationService::getCurrentProperty();
            launching_ = service.Launching.Add([this](System::Object*, const LaunchingEventArgs&) { log_.push_back("Launching"); });
            deactivated_ = service.Deactivated.Add([this, &service](System::Object*, const DeactivatedEventArgs&) {
                // The last moment that is sure to come: put away what must survive.
                auto progress = std::make_shared<Progress>();
                progress->level = 3;
                progress->score = score_;
                service.getStateProperty()["progress"] = progress;
                log_.push_back("Deactivated: progress saved in State");
            });
            activated_ = service.Activated.Add([this, &service](System::Object*, const ActivatedEventArgs& args) {
                std::string line = std::string("Activated (instance preserved: ") +
                    (args.getIsApplicationInstancePreservedProperty() ? "yes" : "no") + ")";
                if (service.getStateProperty().ContainsKey("progress"))
                {
                    line += ", progress restored";
                }
                log_.push_back(line);
            });
            closing_ = service.Closing.Add([this](System::Object*, const ClosingEventArgs&) { log_.push_back("Closing"); ++closings_; });

            // CNA has no phone shell: the game's own Activated/Deactivated/Exiting drive the
            // service once it is attached, and attaching reports this fresh start (Launching).
            service.AttachEXT(*this);
            Context().Check(!log_.empty() && log_.front() == "Launching", "attaching raised Launching");
            DemoGame::Initialize();
        }

        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override
        {
            // The service is process-wide: detach and unsubscribe.
            PhoneApplicationService& service = PhoneApplicationService::getCurrentProperty();
            if (Context().Automated())
            {
                Context().Check(closings_ == 1, "leaving the game raised Closing once");
            }
            service.DetachEXT();
            service.Launching.Remove(launching_);
            service.Deactivated.Remove(deactivated_);
            service.Activated.Remove(activated_);
            service.Closing.Remove(closing_);
            service.getStateProperty().Remove("progress");
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            score_ += 10;
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(24, 36, 56));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "PhoneApplicationService (switch to another window and back)",
                                     Vector2(20.0f, 20.0f), Color::White);
            float y = 70.0f;
            for (const std::string& line : log_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(20.0f, y), Color::LightGreen);
                y += 28.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        System::EventHandler<LaunchingEventArgs>::Token launching_{};
        System::EventHandler<DeactivatedEventArgs>::Token deactivated_{};
        System::EventHandler<ActivatedEventArgs>::Token activated_{};
        System::EventHandler<ClosingEventArgs>::Token closing_{};
        std::vector<std::string> log_;
        int score_ = 0;
        int closings_ = 0;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(LifecycleDemo, {
    .id = "phone.phone_application_service.lifecycle",
    .title = "Phone application lifecycle",
    .description = "Attaches PhoneApplicationService::Current to the game with the CNAEXT AttachEXT: Launching "
                   "reports the fresh start, losing focus raises Deactivated (the game saves progress in State), "
                   "regaining it Activated, and leaving the game Closing.",
    .apis = {"PhoneApplicationService::Current", "PhoneApplicationService::Launching",
             "PhoneApplicationService::Deactivated", "PhoneApplicationService::Activated",
             "PhoneApplicationService::Closing", "PhoneApplicationService::State", "PhoneApplicationService::AttachEXT",
             "PhoneApplicationService::DetachEXT", "ActivatedEventArgs::IsApplicationInstancePreserved"},
    .tags = {"phone", "lifecycle", "tombstoning", "save", "focus"},
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
})
