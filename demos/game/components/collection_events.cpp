// SPDX-License-Identifier: MS-PL
// Reacting to components being added or removed: ComponentAdded and ComponentRemoved.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"
#include "Microsoft/Xna/Framework/GameComponentCollectionEventArgs.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class CollectionEventsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            auto& components = getComponentsProperty();
            components.ComponentAdded += [this](System::Object*, const GameComponentCollectionEventArgs& e) {
                ++added_;
                lastAdded_ = e.getGameComponentProperty();
            };
            components.ComponentRemoved += [this](System::Object*, const GameComponentCollectionEventArgs&) {
                ++removed_;
            };

            auto first = std::make_shared<GameComponent>(*this);
            auto second = std::make_shared<GameComponent>(*this);
            components.Add(first);
            components.Add(second);
            Context().Check(lastAdded_ == second.get(), "the event carries the component that was added");
            Context().Check(components.Contains(first.get()) && components.IndexOf(second.get()) == 1,
                            "Contains and IndexOf find the components");
            Context().Check(components.Remove(first.get()), "Remove succeeds for a member");
            Context().Print("added {}, removed {}", added_, removed_);
            Context().Check(added_ == 2 && removed_ == 1, "ComponentAdded fired twice and ComponentRemoved once");

            DemoGame::Initialize();
        }

    private:
        int added_ = 0;
        int removed_ = 0;
        IGameComponent* lastAdded_ = nullptr;
    };
}

CNA_DEMOS_GAME(CollectionEventsDemo, {
    .id = "game.components.collection_events",
    .title = "Component collection events",
    .description = "Subscribes to GameComponentCollection's ComponentAdded and ComponentRemoved events and uses "
                   "Contains, IndexOf and Remove.",
    .apis = {"GameComponentCollection::ComponentAdded", "GameComponentCollection::ComponentRemoved",
             "GameComponentCollection::Contains", "GameComponentCollection::IndexOf", "GameComponentCollection::Remove",
             "GameComponentCollectionEventArgs::GameComponent",
             "IGameComponent"},
    .tags = {"component", "event"},
    .frames = 3,
})
