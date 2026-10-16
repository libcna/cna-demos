// SPDX-License-Identifier: MS-PL
// A GameComponent: reusable per-frame logic that the Game updates for you.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    // Counts frames; any game can add it.
    class FrameCounter final : public GameComponent
    {
    public:
        explicit FrameCounter(Game& game) : GameComponent(game) {}

        int initialized = 0;
        int updates = 0;

        void Initialize() override
        {
            ++initialized;
            GameComponent::Initialize();
        }

        void Update(GameTime& gameTime) override
        {
            ++updates;
            GameComponent::Update(gameTime);
        }
    };

    class GameComponentDemo final : public CnaDemos::DemoGame
    {
    public:
        GameComponentDemo()
        {
            // The collection keeps the component alive (shared_ptr overload, CNA's ownership
            // model for what C# leaves to the garbage collector).
            counter_ = std::make_shared<FrameCounter>(*this);
            counter_->Disposed += [this](System::Object*, const System::EventArgs&) { ++disposedEvents_; };
            getComponentsProperty().Add(counter_);
        }

    protected:
        void Update(GameTime& gameTime) override
        {
            Game::Update(gameTime);   // updates every enabled component

            // After five frames, disable it: Game::Update skips disabled components from now on.
            if (++updates_ == 5)
            {
                counter_->setEnabledProperty(false);
            }
        }

        void EndRun() override
        {
            Context().Print("component initialized {}, updated {} times before disable",
                            counter_->initialized, counter_->updates);
            Context().Check(counter_->initialized == 1, "Game::Initialize initialized the component");
            Context().Check(counter_->updates == 5, "the component updated until it was disabled");
            Context().Check(!counter_->getEnabledProperty(), "Enabled reads back false");
            Context().Check(&counter_->getGameProperty() == this, "a component knows its Game");
            counter_->Dispose();
            counter_->Dispose();
            Context().Check(disposedEvents_ == 2,
                            "genuine XNA raises Disposed for each explicit Dispose call");
        }

    private:
        std::shared_ptr<FrameCounter> counter_;
        int updates_ = 0;
        int disposedEvents_ = 0;
    };
}

CNA_DEMOS_GAME(GameComponentDemo, {
    .id = "game.components.game_component",
    .title = "A game component",
    .description = "Adds a GameComponent to Game.Components so the game initializes and updates it, then disables "
                   "and repeatedly disposes it while observing XNA's public event contract.",
    .apis = {"GameComponent::GameComponent", "GameComponent::Initialize", "GameComponent::Update",
             "GameComponent::Dispose", "GameComponent::Disposed", "GameComponent::Enabled", "GameComponent::Game",
             "IGameComponent::Initialize", "IUpdateable::Update", "Game::Components", "GameComponentCollection::Add"},
    .tags = {"component", "architecture"},
    .frames = 10,
    .restrictions = "CNA-DEMOS-ISSUE-141 records repeated Dispose suppressing XNA's repeated Disposed event.",
    .knownIssue = "CNA-DEMOS-ISSUE-141",
})
