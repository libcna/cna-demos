// SPDX-License-Identifier: MS-PL
// Ordering components: UpdateOrder and DrawOrder.
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/DrawableGameComponent.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    class Recorder final : public DrawableGameComponent
    {
    public:
        Recorder(Game& game, char name, std::string& updateLog, std::string& drawLog)
            : DrawableGameComponent(game), name_(name), updateLog_(updateLog), drawLog_(drawLog)
        {
        }

        void Update(GameTime& gameTime) override
        {
            updateLog_ += name_;
            DrawableGameComponent::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            drawLog_ += name_;
            DrawableGameComponent::Draw(gameTime);
        }

    private:
        char name_;
        std::string& updateLog_;
        std::string& drawLog_;
    };

    class OrderDemo final : public CnaDemos::DemoGame
    {
    public:
        OrderDemo()
        {
            auto input = std::make_shared<Recorder>(*this, 'I', updateLog_, drawLog_);
            auto physics = std::make_shared<Recorder>(*this, 'P', updateLog_, drawLog_);
            auto hud = std::make_shared<Recorder>(*this, 'H', updateLog_, drawLog_);

            // Lower values run first. Added in the "wrong" order on purpose.
            hud->setUpdateOrderProperty(30);
            physics->setUpdateOrderProperty(20);
            input->setUpdateOrderProperty(10);
            // The HUD must be drawn last, on top.
            hud->setDrawOrderProperty(100);
            physics->setDrawOrderProperty(0);
            input->setDrawOrderProperty(50);

            getComponentsProperty().Add(hud);
            getComponentsProperty().Add(physics);
            getComponentsProperty().Add(input);
        }

    protected:
        void Update(GameTime& gameTime) override
        {
            updateLog_.clear();
            Game::Update(gameTime);
        }

        void Draw(const GameTime& gameTime) override
        {
            drawLog_.clear();
            Game::Draw(gameTime);
        }

        void EndRun() override
        {
            Context().Print("update order: {}   draw order: {}", updateLog_, drawLog_);
            Context().Check(updateLog_ == "IPH", "components update by ascending UpdateOrder");
            Context().Check(drawLog_ == "PIH", "and draw by ascending DrawOrder");
        }

    private:
        std::string updateLog_;
        std::string drawLog_;
    };
}

CNA_DEMOS_GAME(OrderDemo, {
    .id = "game.components.update_and_draw_order",
    .title = "Component update and draw order",
    .description = "Adds three components in arbitrary order and sequences them with UpdateOrder and DrawOrder.",
    .apis = {"GameComponent::UpdateOrder", "DrawableGameComponent::DrawOrder"},
    .tags = {"component", "ordering"},
    .frames = 5,
})
