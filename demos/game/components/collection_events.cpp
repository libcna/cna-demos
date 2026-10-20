// SPDX-License-Identifier: MS-PL
// GameComponentCollection insertion, removal, events, boundaries and CNA ownership adaptation.
#include <memory>
#include <stdexcept>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"
#include "Microsoft/Xna/Framework/GameComponentCollectionEventArgs.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class LifetimeComponent final : public GameComponent
    {
    public:
        LifetimeComponent(Game& game, int& destructions) : GameComponent(game), destructions_(destructions) {}
        ~LifetimeComponent() override { ++destructions_; }

    private:
        int& destructions_;
    };

    class CollectionEventsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void Initialize() override
        {
            auto& ctx = Context();
            GameComponentCollection detached;
            ctx.Check(detached.getCountProperty() == 0,
                      "a directly constructed component collection starts empty");

            auto first = std::make_shared<GameComponent>(*this);
            auto second = std::make_shared<GameComponent>(*this);
            auto third = std::make_shared<GameComponent>(*this);
            const GameComponentCollectionEventArgs explicitArgs(first.get());
            const GameComponentCollectionEventArgs nullArgs(nullptr);
            ctx.Check(explicitArgs.getGameComponentProperty() == first.get() &&
                          nullArgs.getGameComponentProperty() == nullptr,
                      "explicit collection event args retain either their component or null");

            std::vector<IGameComponent*> added;
            std::vector<IGameComponent*> removed;
            bool addedState = true;
            bool removedState = true;
            bool canonicalSenders = true;
            int nullAdded = 0;
            int nullRemoved = 0;
            detached.ComponentAdded += [&](System::Object* sender, const GameComponentCollectionEventArgs& e) {
                canonicalSenders = canonicalSenders && sender == &detached;
                IGameComponent* component = e.getGameComponentProperty();
                if (component == nullptr)
                {
                    ++nullAdded;
                }
                else
                {
                    added.push_back(component);
                    addedState = addedState && detached.Contains(component);
                }
            };
            detached.ComponentRemoved += [&](System::Object* sender, const GameComponentCollectionEventArgs& e) {
                canonicalSenders = canonicalSenders && sender == &detached;
                IGameComponent* component = e.getGameComponentProperty();
                if (component == nullptr)
                {
                    ++nullRemoved;
                }
                else
                {
                    removed.push_back(component);
                    removedState = removedState && !detached.Contains(component);
                }
            };

            detached.Add(first.get());
            detached.Insert(0, second.get());
            const GameComponentCollection& constDetached = detached;
            std::vector<IGameComponent*> mutableRange(detached.begin(), detached.end());
            std::vector<IGameComponent*> constRange(constDetached.begin(), constDetached.end());
            ctx.Check(detached.getCountProperty() == 2 && detached[0] == second.get() && detached[1] == first.get() &&
                          detached.IndexOf(first.get()) == 1 && detached.IndexOf(second.get()) == 0 &&
                          mutableRange == constRange && mutableRange == std::vector<IGameComponent*>({second.get(), first.get()}) &&
                          added == std::vector<IGameComponent*>({first.get(), second.get()}) && addedState && canonicalSenders,
                      "Add, Insert, Item, lookup and both CNA ranges preserve exact collection and event order");

            bool duplicateRejected = false;
            try
            {
                detached.Add(first.get());
            }
            catch (const System::ArgumentException&)
            {
                duplicateRejected = true;
            }
            ctx.Check(duplicateRejected && detached.getCountProperty() == 2,
                      "adding the same component twice is refused without mutation");

            bool invalidUniqueUsesRange = false;
            bool invalidDuplicateUsesRange = false;
            try
            {
                detached.Insert(3, third.get());
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                invalidUniqueUsesRange = true;
            }
            try
            {
                detached.Insert(3, first.get());
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                invalidDuplicateUsesRange = true;
            }
            catch (const System::ArgumentException&)
            {
            }
            ctx.Check(invalidUniqueUsesRange && invalidDuplicateUsesRange && detached.getCountProperty() == 2,
                      "an invalid Insert index takes precedence over duplicate-item validation");

            detached.RemoveAt(0);
            bool missingRemove = detached.Remove(second.get());
            bool invalidItemRejected = false;
            try
            {
                (void)detached[1];
            }
            catch (const System::ArgumentOutOfRangeException&)
            {
                invalidItemRejected = true;
            }
            ctx.Check(detached.getCountProperty() == 1 && detached[0] == first.get() && !missingRemove &&
                          removed == std::vector<IGameComponent*>({second.get()}) && removedState && invalidItemRejected,
                      "RemoveAt erases before its event, missing Remove is harmless and Item validates its index");

            removed.clear();
            removedState = true;
            bool clearSawCompleteCollection = true;
            const auto clearToken = detached.ComponentRemoved.Add(
                [&](System::Object*, const GameComponentCollectionEventArgs&) {
                    clearSawCompleteCollection = clearSawCompleteCollection && detached.getCountProperty() == 2;
                });
            detached.Add(second.get());
            detached.Clear();
            detached.ComponentRemoved.Remove(clearToken);
            ctx.Check(detached.getCountProperty() == 0 &&
                          removed == std::vector<IGameComponent*>({first.get(), second.get()}) &&
                          clearSawCompleteCollection,
                      "Clear raises removals in forward order while the original collection remains observable");

            detached.Add(nullptr);
            const bool removedNull = detached.Remove(nullptr);
            const bool ordinaryNullSilent = nullAdded == 0 && nullRemoved == 0;
            detached.Add(nullptr);
            detached.Clear();
            ctx.Check(removedNull && ordinaryNullSilent && nullRemoved == 1 && detached.getCountProperty() == 0,
                      "XNA keeps null entries silent for Add/Remove but raises their removal during Clear");

            GameComponentCollection throwing;
            auto throwingItem = std::make_shared<GameComponent>(*this);
            const auto throwingToken = throwing.ComponentAdded.Add(
                [](System::Object*, const GameComponentCollectionEventArgs&) {
                    throw std::runtime_error("component-added probe");
                });
            bool handlerFailurePropagated = false;
            try
            {
                throwing.Add(throwingItem.get());
            }
            catch (const std::runtime_error&)
            {
                handlerFailurePropagated = true;
            }
            throwing.ComponentAdded.Remove(throwingToken);
            ctx.Check(handlerFailurePropagated && throwing.getCountProperty() == 1 &&
                          throwing.Contains(throwingItem.get()),
                      "a throwing ComponentAdded handler propagates after the insertion remains visible");
            throwing.Clear();

            int destructions = 0;
            bool liveDuringRemoval = false;
            GameComponentCollection owning;
            auto owned = std::make_shared<LifetimeComponent>(*this, destructions);
            const std::weak_ptr<LifetimeComponent> lifetime = owned;
            IGameComponent* const ownedRaw = owned.get();
            owning.ComponentRemoved += [&](System::Object*, const GameComponentCollectionEventArgs& e) {
                if (e.getGameComponentProperty() == ownedRaw)
                {
                    liveDuringRemoval = !lifetime.expired();
                }
            };
            owning.Add(owned);
            owned.reset();
            const bool retainedByCollection = !lifetime.expired() && owning.Contains(ownedRaw);
            const bool removedOwned = owning.Remove(ownedRaw);
            ctx.Check(retainedByCollection && removedOwned && liveDuringRemoval && lifetime.expired() && destructions == 1,
                      "the CNA shared ownership overload retains a component through its removal event, then releases it");

            auto& components = getComponentsProperty();
            int integratedAdded = 0;
            int integratedRemoved = 0;
            IGameComponent* lastAdded = nullptr;
            components.ComponentAdded += [&](System::Object*, const GameComponentCollectionEventArgs& e) {
                ++integratedAdded;
                lastAdded = e.getGameComponentProperty();
            };
            components.ComponentRemoved += [&](System::Object*, const GameComponentCollectionEventArgs&) {
                ++integratedRemoved;
            };
            components.Add(first);
            components.Add(second);
            const bool removedFirst = components.Remove(first.get());
            ctx.Print("integrated added {}, removed {}", integratedAdded, integratedRemoved);
            ctx.Check(lastAdded == second.get() && removedFirst && integratedAdded == 2 && integratedRemoved == 1,
                      "Game.Components exposes the same add/remove events and payloads");

            DemoGame::Initialize();
        }
    };
}

CNA_DEMOS_GAME(CollectionEventsDemo, {
    .id = "game.components.collection_events",
    .title = "Component collection events",
    .description = "Exercises GameComponentCollection insertion, lookup, ranges, removal and event boundaries, plus "
                   "the CNA shared-ownership adaptation used by Game.Components.",
    .apis = {"GameComponentCollection::GameComponentCollection", "GameComponentCollection::Count",
             "GameComponentCollection::ComponentAdded", "GameComponentCollection::ComponentRemoved",
             "GameComponentCollection::Add", "GameComponentCollection::Insert",
             "GameComponentCollection::Contains", "GameComponentCollection::IndexOf",
             "GameComponentCollection::Item", "GameComponentCollection::Remove",
             "GameComponentCollection::RemoveAt", "GameComponentCollection::Clear",
             "GameComponentCollection::begin", "GameComponentCollection::end",
             "GameComponentCollectionEventArgs::GameComponentCollectionEventArgs",
             "GameComponentCollectionEventArgs::GameComponent", "IGameComponent"},
    .tags = {"component", "collection", "event", "ownership", "validation"},
    .frames = 3,
})
