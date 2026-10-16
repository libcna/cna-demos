// SPDX-License-Identifier: MS-PL
// Component change events: EnabledChanged, UpdateOrderChanged, VisibleChanged, DrawOrderChanged --
// how other code (and the component itself) learns that a component was switched or reordered.
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/DrawableGameComponent.hpp"
#include "Microsoft/Xna/Framework/GameComponent.hpp"
#include "System/EventArgs.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    // Spawns enemies on a timer. When it is switched back on it restarts its timer: the
    // component reacts to its own change through the OnEnabledChanged hook.
    class Spawner final : public GameComponent
    {
    public:
        explicit Spawner(Game& game) : GameComponent(game) {}

        int timerRestarts = 0;
        int updateOrderChanges = 0;

    protected:
        void OnEnabledChanged(System::Object* sender, const System::EventArgs& args) override
        {
            if (getEnabledProperty())
            {
                ++timerRestarts;
            }
            GameComponent::OnEnabledChanged(sender, args);
        }

        void OnUpdateOrderChanged(System::Object* sender, const System::EventArgs& args) override
        {
            ++updateOrderChanges;
            GameComponent::OnUpdateOrderChanged(sender, args);
        }
    };

    // Draws the score.
    class Hud final : public DrawableGameComponent
    {
    public:
        explicit Hud(Game& game) : DrawableGameComponent(game) {}

        int visibleChanges = 0;
        int drawOrderChanges = 0;

    protected:
        void OnVisibleChanged(System::Object* sender, const System::EventArgs& args) override
        {
            ++visibleChanges;
            DrawableGameComponent::OnVisibleChanged(sender, args);
        }

        void OnDrawOrderChanged(System::Object* sender, const System::EventArgs& args) override
        {
            ++drawOrderChanges;
            DrawableGameComponent::OnDrawOrderChanged(sender, args);
        }
    };

    class ComponentDispatchProbe final : public GameComponent
    {
    public:
        ComponentDispatchProbe(Game& game, std::vector<std::string>& trace, bool forward)
            : GameComponent(game), trace_(trace), forward_(forward)
        {
        }

        bool canonicalArguments = true;

    protected:
        void OnEnabledChanged(System::Object* sender, const System::EventArgs& args) override
        {
            trace_.push_back("enabled hook");
            canonicalArguments = canonicalArguments && sender == this && &args == &System::EventArgs::Empty;
            if (forward_)
            {
                GameComponent::OnEnabledChanged(sender, args);
            }
        }

        void OnUpdateOrderChanged(System::Object* sender, const System::EventArgs& args) override
        {
            trace_.push_back("order hook");
            canonicalArguments = canonicalArguments && sender == this && &args == &System::EventArgs::Empty;
            if (forward_)
            {
                GameComponent::OnUpdateOrderChanged(sender, args);
            }
        }

    private:
        std::vector<std::string>& trace_;
        bool forward_;
    };

    class DrawableDispatchProbe final : public DrawableGameComponent
    {
    public:
        DrawableDispatchProbe(Game& game, std::vector<std::string>& trace, bool forward)
            : DrawableGameComponent(game), trace_(trace), forward_(forward)
        {
        }

        bool canonicalArguments = true;

    protected:
        void OnVisibleChanged(System::Object* sender, const System::EventArgs& args) override
        {
            trace_.push_back("visible hook");
            canonicalArguments = canonicalArguments && sender == this && &args == &System::EventArgs::Empty;
            if (forward_)
            {
                DrawableGameComponent::OnVisibleChanged(sender, args);
            }
        }

        void OnDrawOrderChanged(System::Object* sender, const System::EventArgs& args) override
        {
            trace_.push_back("draw hook");
            canonicalArguments = canonicalArguments && sender == this && &args == &System::EventArgs::Empty;
            if (forward_)
            {
                DrawableGameComponent::OnDrawOrderChanged(sender, args);
            }
        }

    private:
        std::vector<std::string>& trace_;
        bool forward_;
    };

    class ChangeEventsDemo final : public CnaDemos::DemoGame
    {
    public:
        ChangeEventsDemo()
        {
            spawner_ = std::make_shared<Spawner>(*this);
            hud_ = std::make_shared<Hud>(*this);
            getComponentsProperty().Add(spawner_);
            getComponentsProperty().Add(hud_);

            // Other code -- here a pause menu's status line -- listens instead of polling.
            spawner_->EnabledChanged += [this](System::Object*, const System::EventArgs&) {
                log_.push_back(std::string("EnabledChanged -> ") + (spawner_->getEnabledProperty() ? "true" : "false"));
            };
            spawner_->UpdateOrderChanged += [this](System::Object*, const System::EventArgs&) {
                log_.push_back("UpdateOrderChanged -> " + std::to_string(spawner_->getUpdateOrderProperty()));
            };
            hud_->VisibleChanged += [this](System::Object*, const System::EventArgs&) {
                log_.push_back(std::string("VisibleChanged -> ") + (hud_->getVisibleProperty() ? "true" : "false"));
            };
            hud_->DrawOrderChanged += [this](System::Object*, const System::EventArgs&) {
                log_.push_back("DrawOrderChanged -> " + std::to_string(hud_->getDrawOrderProperty()));
            };
        }

    protected:
        void Update(GameTime& gameTime) override
        {
            Game::Update(gameTime);
            if (++updates_ != 2)
            {
                return;
            }
            auto& ctx = Context();
            // The game pauses: spawner off, HUD hidden. Setting the same value again changes
            // nothing and raises nothing.
            spawner_->setEnabledProperty(false);
            spawner_->setEnabledProperty(false);
            hud_->setVisibleProperty(false);
            // The order changes, e.g. the HUD must draw above a newly added overlay.
            spawner_->setUpdateOrderProperty(5);
            hud_->setDrawOrderProperty(10);
            // Unpause.
            spawner_->setEnabledProperty(true);
            hud_->setVisibleProperty(true);

            for (const std::string& line : log_)
            {
                ctx.Print("{}", line);
            }
            const std::vector<std::string> expected = {
                "EnabledChanged -> false", "VisibleChanged -> false", "UpdateOrderChanged -> 5",
                "DrawOrderChanged -> 10",  "EnabledChanged -> true",  "VisibleChanged -> true",
            };
            ctx.Check(log_ == expected, "each real change raised its event once, in order; a repeated value none");
            ctx.Check(spawner_->timerRestarts == 1, "the spawner's OnEnabledChanged saw it switched back on");
            ctx.Check(spawner_->updateOrderChanges == 1 && hud_->drawOrderChanges == 1 && hud_->visibleChanges == 2,
                      "protected order and visibility callbacks observe each corresponding real change");

            std::vector<std::string> dispatchTrace;
            ComponentDispatchProbe component(*this, dispatchTrace, true);
            DrawableDispatchProbe drawable(*this, dispatchTrace, true);
            bool canonicalEventArguments = true;
            component.EnabledChanged += [&component, &dispatchTrace, &canonicalEventArguments](
                                            System::Object* sender, const System::EventArgs& args) {
                dispatchTrace.push_back("enabled event");
                canonicalEventArguments = canonicalEventArguments && sender == &component &&
                    &args == &System::EventArgs::Empty;
            };
            component.UpdateOrderChanged += [&component, &dispatchTrace, &canonicalEventArguments](
                                                System::Object* sender, const System::EventArgs& args) {
                dispatchTrace.push_back("order event");
                canonicalEventArguments = canonicalEventArguments && sender == &component &&
                    &args == &System::EventArgs::Empty;
            };
            drawable.VisibleChanged += [&drawable, &dispatchTrace, &canonicalEventArguments](
                                           System::Object* sender, const System::EventArgs& args) {
                dispatchTrace.push_back("visible event");
                canonicalEventArguments = canonicalEventArguments && sender == &drawable &&
                    &args == &System::EventArgs::Empty;
            };
            drawable.DrawOrderChanged += [&drawable, &dispatchTrace, &canonicalEventArguments](
                                             System::Object* sender, const System::EventArgs& args) {
                dispatchTrace.push_back("draw event");
                canonicalEventArguments = canonicalEventArguments && sender == &drawable &&
                    &args == &System::EventArgs::Empty;
            };

            ctx.Check(component.getEnabledProperty() && component.getUpdateOrderProperty() == 0 &&
                          drawable.getVisibleProperty() && drawable.getDrawOrderProperty() == 0,
                      "component change properties start at XNA's enabled, visible and zero-order defaults");
            component.setEnabledProperty(false);
            component.setEnabledProperty(false);
            component.setUpdateOrderProperty(-7);
            component.setUpdateOrderProperty(-7);
            drawable.setVisibleProperty(false);
            drawable.setVisibleProperty(false);
            drawable.setDrawOrderProperty(9);
            drawable.setDrawOrderProperty(9);
            const std::vector<std::string> xnaDispatch = {
                "enabled hook", "enabled event", "order hook", "order event",
                "visible hook", "visible event", "draw hook", "draw event"};
            ctx.Check(dispatchTrace == xnaDispatch,
                      "XNA change hooks run before the public events they dispatch");
            ctx.Check(canonicalEventArguments && component.canonicalArguments && drawable.canonicalArguments,
                      "change hooks and events receive the component sender and EventArgs::Empty");
            ctx.Check(&component.getEnabledChangedEvent() == &component.EnabledChanged &&
                          &component.getUpdateOrderChangedEvent() == &component.UpdateOrderChanged &&
                          &drawable.getVisibleChangedEvent() == &drawable.VisibleChanged &&
                          &drawable.getDrawOrderChangedEvent() == &drawable.DrawOrderChanged,
                      "the CNA interface event accessors expose the same public event objects");

            std::vector<std::string> suppressedTrace;
            ComponentDispatchProbe suppressingComponent(*this, suppressedTrace, false);
            DrawableDispatchProbe suppressingDrawable(*this, suppressedTrace, false);
            int suppressedEvents = 0;
            suppressingComponent.EnabledChanged += [&suppressedEvents](System::Object*, const System::EventArgs&) {
                ++suppressedEvents;
            };
            suppressingComponent.UpdateOrderChanged += [&suppressedEvents](System::Object*, const System::EventArgs&) {
                ++suppressedEvents;
            };
            suppressingDrawable.VisibleChanged += [&suppressedEvents](System::Object*, const System::EventArgs&) {
                ++suppressedEvents;
            };
            suppressingDrawable.DrawOrderChanged += [&suppressedEvents](System::Object*, const System::EventArgs&) {
                ++suppressedEvents;
            };
            suppressingComponent.setEnabledProperty(false);
            suppressingComponent.setUpdateOrderProperty(1);
            suppressingDrawable.setVisibleProperty(false);
            suppressingDrawable.setDrawOrderProperty(1);
            ctx.Check(suppressedEvents == 0 && suppressedTrace.size() == 4,
                      "an override that omits its XNA base hook suppresses the corresponding public event");
        }

    private:
        std::shared_ptr<Spawner> spawner_;
        std::shared_ptr<Hud> hud_;
        std::vector<std::string> log_;
        int updates_ = 0;
    };
}

CNA_DEMOS_GAME(ChangeEventsDemo, {
    .id = "game.components.change_events",
    .title = "Component change events",
    .description = "Pauses and reorders components, checks default state and exact event arguments, and demonstrates "
                   "the ordering and override-dispatch contract of all four protected change hooks.",
    .apis = {"GameComponent::EnabledChanged", "GameComponent::UpdateOrderChanged", "GameComponent::OnEnabledChanged",
             "GameComponent::OnUpdateOrderChanged", "DrawableGameComponent::VisibleChanged",
             "DrawableGameComponent::DrawOrderChanged", "DrawableGameComponent::OnVisibleChanged",
             "DrawableGameComponent::OnDrawOrderChanged",
             "GameComponent::Enabled", "GameComponent::UpdateOrder", "DrawableGameComponent::Visible",
             "DrawableGameComponent::DrawOrder", "IUpdateable::Enabled", "IUpdateable::EnabledChanged",
             "IUpdateable::UpdateOrder", "IUpdateable::UpdateOrderChanged", "IDrawable::Visible",
             "IDrawable::VisibleChanged", "IDrawable::DrawOrder", "IDrawable::DrawOrderChanged"},
    .tags = {"component", "event", "pause"},
    .frames = 3,
    .restrictions = "CNA-DEMOS-ISSUE-142 records public events firing before and independently of their XNA hooks.",
    .knownIssue = "CNA-DEMOS-ISSUE-142",
})
