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
    .description = "Pauses and reorders two components and logs GameComponent::EnabledChanged/UpdateOrderChanged and "
                   "DrawableGameComponent::VisibleChanged/DrawOrderChanged; the component itself reacts through the "
                   "OnEnabledChanged hook.",
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
})
