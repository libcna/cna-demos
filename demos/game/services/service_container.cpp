// SPDX-License-Identifier: MS-PL
// Sharing objects between components through Game.Services (GameServiceContainer).
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"
#include "Microsoft/Xna/Framework/GameServiceContainer.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    // Any class can be a service; components look it up by type instead of holding a pointer
    // handed to them by whoever created them.
    class ScoreService
    {
    public:
        int score = 0;
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
            Context().Print("score after the run: {}", scores_.score);
            Context().Check(scores_.score > 0, "the component found the service and updated it");

            auto& services = getServicesProperty();
            Context().Check(services.GetService<ScoreService>() == &scores_, "GetService<T> returns the registered object");
            services.RemoveService<ScoreService>();
            Context().Check(services.GetService<ScoreService>() == nullptr, "after RemoveService it is gone");
        }

    private:
        ScoreService scores_;
    };
}

CNA_DEMOS_GAME(ServicesDemo, {
    .id = "game.services.service_container",
    .title = "Game services",
    .description = "Registers an object with Game.Services, finds it from a component with GetService and removes "
                   "it with RemoveService.",
    .apis = {"Game::Services", "GameServiceContainer::AddService", "GameServiceContainer::GetService",
             "GameServiceContainer::RemoveService"},
    .tags = {"services", "dependency-injection", "architecture"},
    .frames = 5,
})
