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
            input_ = std::make_shared<Recorder>(*this, 'I', updateLog_, drawLog_);
            physics_ = std::make_shared<Recorder>(*this, 'P', updateLog_, drawLog_);
            hud_ = std::make_shared<Recorder>(*this, 'H', updateLog_, drawLog_);

            // Lower values run first. Added in the "wrong" order on purpose.
            hud_->setUpdateOrderProperty(30);
            physics_->setUpdateOrderProperty(20);
            input_->setUpdateOrderProperty(10);
            // The HUD must be drawn last, on top.
            hud_->setDrawOrderProperty(100);
            physics_->setDrawOrderProperty(0);
            input_->setDrawOrderProperty(50);

            getComponentsProperty().Add(hud_);
            getComponentsProperty().Add(physics_);
            getComponentsProperty().Add(input_);
        }

    protected:
        void Update(GameTime& gameTime) override
        {
            ++updatePass_;
            updateLog_.clear();
            Game::Update(gameTime);
            if (updatePass_ == 1)
            {
                initialUpdateLog_ = updateLog_;
                hud_->setUpdateOrderProperty(5);
            }
            else if (updatePass_ == 2)
            {
                changedUpdateLog_ = updateLog_;
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            ++drawPass_;
            drawLog_.clear();
            Game::Draw(gameTime);
            if (drawPass_ == 1)
            {
                initialDrawLog_ = drawLog_;
                hud_->setDrawOrderProperty(-10);
            }
            else if (drawPass_ == 2)
            {
                changedDrawLog_ = drawLog_;
            }
        }

        void EndRun() override
        {
            Context().Print("initial update/draw: {}/{}; changed: {}/{}",
                            initialUpdateLog_, initialDrawLog_, changedUpdateLog_, changedDrawLog_);
            Context().Check(initialUpdateLog_ == "IPH" && initialDrawLog_ == "PIH",
                            "components update and draw by ascending order rather than insertion order");
            Context().Check(changedUpdateLog_ == "HIP" && changedDrawLog_ == "HPI",
                            "changing either order after registration re-sorts the next component pass");

            input_->setUpdateOrderProperty(-2);
            physics_->setUpdateOrderProperty(3);
            const int forward = input_->CompareTo(*physics_);
            const int reverse = physics_->CompareTo(*input_);
            physics_->setUpdateOrderProperty(-2);
            Context().Check(forward != 0 && forward == -reverse && input_->CompareTo(*physics_) == 0,
                            "the CNA public component comparator is antisymmetric and groups equal update orders");
        }

    private:
        std::shared_ptr<Recorder> input_;
        std::shared_ptr<Recorder> physics_;
        std::shared_ptr<Recorder> hud_;
        std::string updateLog_;
        std::string drawLog_;
        std::string initialUpdateLog_;
        std::string initialDrawLog_;
        std::string changedUpdateLog_;
        std::string changedDrawLog_;
        int updatePass_ = 0;
        int drawPass_ = 0;
    };
}

CNA_DEMOS_GAME(OrderDemo, {
    .id = "game.components.update_and_draw_order",
    .title = "Component update and draw order",
    .description = "Adds three components in arbitrary order, sequences them with UpdateOrder and DrawOrder, then "
                   "changes both properties to demonstrate live re-sorting and CNA's public comparator.",
    .apis = {"GameComponent::UpdateOrder", "GameComponent::CompareTo", "GameComponent::UpdateOrderChanged",
             "DrawableGameComponent::DrawOrder", "DrawableGameComponent::DrawOrderChanged"},
    .tags = {"component", "ordering"},
    .frames = 5,
})
