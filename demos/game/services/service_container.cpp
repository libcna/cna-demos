// SPDX-License-Identifier: MS-PL
// Sharing objects between components through Game.Services (GameServiceContainer).
#include <memory>
#include <string>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"
#include "Microsoft/Xna/Framework/GameServiceContainer.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    // Any class can be a service; components look it up by type instead of holding a pointer
    // handed to them by whoever created them.
    class ScoreView
    {
    public:
        virtual ~ScoreView() = default;
        [[nodiscard]] virtual int Score() const = 0;
    };

    class ScoreService final : public ScoreView
    {
    public:
        int score = 0;
        [[nodiscard]] int Score() const override { return score; }
    };

    class ScoringComponent final : public GameComponent
    {
    public:
        explicit ScoringComponent(Game& game) : GameComponent(game) {}

        void Update(GameTime& gameTime) override
        {
            if (ScoreService* scores = getGameProperty().getServicesProperty().GetService<ScoreService>())
            {
                scores->score += 10;
            }
            GameComponent::Update(gameTime);
        }
    };

    class ServicesDemo final : public CnaDemos::DemoGame
    {
    public:
        ServicesDemo()
        {
            // Register the service; the container does not own it.
            getServicesProperty().AddService<ScoreService>(&scores_);
            getComponentsProperty().Add(std::make_shared<ScoringComponent>(*this));
        }

    protected:
        void EndRun() override
        {
            auto& ctx = Context();
            GameServiceContainer detached;
            ctx.Check(detached.GetService<ScoreService>() == nullptr && detached.GetService(typeid(ScoreService)) == nullptr,
                      "a directly constructed service container starts empty in typed and runtime lookups");

            ScoreService first;
            ScoreService replacement;
            detached.AddService<ScoreService>(&first);
            detached.AddService<ScoreView>(&first);
            ctx.Check(detached.GetService<ScoreService>() == &first &&
                          detached.GetService<ScoreView>() == static_cast<ScoreView*>(&first) &&
                          detached.GetService(typeid(ScoreService)) == &first,
                      "one provider can be registered and retrieved under distinct assignable type keys");

            bool duplicateRejected = false;
            try
            {
                detached.AddService<ScoreService>(&replacement);
            }
            catch (const System::ArgumentException&)
            {
                duplicateRejected = true;
            }
            ctx.Check(duplicateRejected && detached.GetService<ScoreService>() == &first,
                      "duplicate registration is refused with ArgumentException and preserves the original provider");

            bool nullRejected = false;
            try
            {
                detached.AddService<ScoreService>(nullptr);
            }
            catch (const System::ArgumentNullException&)
            {
                nullRejected = true;
            }
            detached.RemoveService<std::string>();
            ctx.Check(nullRejected && detached.GetService<ScoreService>() == &first,
                      "a null provider is refused with ArgumentNullException and removing a missing service is harmless");

            GameServiceContainer moved(std::move(detached));
            ctx.Check(moved.GetService<ScoreService>() == &first &&
                          moved.GetService<ScoreView>() == static_cast<ScoreView*>(&first),
                      "the CNA move constructor transfers every registered pointer and type key");
            moved.RemoveService<ScoreView>();
            moved.RemoveService<ScoreService>();
            ctx.Check(moved.GetService<ScoreView>() == nullptr && moved.GetService<ScoreService>() == nullptr,
                      "RemoveService deletes exactly the selected type keys");

            ctx.Print("score after the run: {}", scores_.score);
            ctx.Check(scores_.score > 0, "the component found the service and updated it");

            auto& services = getServicesProperty();
            ctx.Check(services.GetService<ScoreService>() == &scores_, "GetService<T> returns the registered object");
            services.RemoveService<ScoreService>();
            ctx.Check(services.GetService<ScoreService>() == nullptr, "after RemoveService it is gone");
        }

    private:
        ScoreService scores_;
    };
}

CNA_DEMOS_GAME(ServicesDemo, {
    .id = "game.services.service_container",
    .title = "Game services",
    .description = "Exercises typed and runtime GameServiceContainer registration, validation, lookup, removal and "
                   "CNA move semantics, then consumes a real Game.Services entry from a component.",
    .apis = {"Game::Services", "GameServiceContainer::GameServiceContainer",
             "GameServiceContainer::AddService", "GameServiceContainer::GetService",
             "GameServiceContainer::RemoveService"},
    .tags = {"services", "dependency-injection", "architecture"},
    .frames = 5,
})
