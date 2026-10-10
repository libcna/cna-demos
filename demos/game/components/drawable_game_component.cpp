// SPDX-License-Identifier: MS-PL
// A DrawableGameComponent: a component with its own LoadContent and Draw.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/DrawableGameComponent.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    // A coloured bar drawn by the component itself.
    class Bar final : public DrawableGameComponent
    {
    public:
        Bar(Game& game, Rectangle area, Color color) : DrawableGameComponent(game), area_(area), color_(color) {}

        int loads = 0;
        int draws = 0;

    protected:
        void LoadContent() override
        {
            ++loads;
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            pixel_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), 1, 1);
            const Color white = Color::White;
            pixel_->SetData(&white, 1);
        }

    public:
        void Draw(const GameTime& gameTime) override
        {
            ++draws;
            spriteBatch_->Begin();
            spriteBatch_->Draw(*pixel_, area_, color_);
            spriteBatch_->End();
            DrawableGameComponent::Draw(gameTime);
        }

    private:
        Rectangle area_;
        Color color_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> pixel_;
    };

    class DrawableComponentDemo final : public CnaDemos::DemoGame
    {
    public:
        DrawableComponentDemo()
        {
            top_ = std::make_shared<Bar>(*this, Rectangle(100, 100, 600, 80), Color::Gold);
            bottom_ = std::make_shared<Bar>(*this, Rectangle(100, 300, 600, 80), Color::Crimson);
            getComponentsProperty().Add(top_);
            getComponentsProperty().Add(bottom_);
            bottom_->setVisibleProperty(false);   // invisible components are not drawn
        }

    protected:
        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateGray);
            Game::Draw(gameTime);   // draws every visible DrawableGameComponent
        }

        void EndRun() override
        {
            Context().Print("top bar: {} load(s), {} draw(s); hidden bar: {} draw(s)", top_->loads, top_->draws, bottom_->draws);
            Context().Check(top_->loads == 1 && bottom_->loads == 1, "each component loaded its content once");
            Context().Check(top_->draws > 0, "the visible component drew every frame");
            Context().Check(bottom_->draws == 0, "the invisible component was never drawn");
        }

    private:
        std::shared_ptr<Bar> top_;
        std::shared_ptr<Bar> bottom_;
    };
}

CNA_DEMOS_GAME(DrawableComponentDemo, {
    .id = "game.components.drawable_game_component",
    .title = "A drawable game component",
    .description = "Builds DrawableGameComponents that load their own content and draw themselves when the game "
                   "draws; one is hidden with the Visible property.",
    .apis = {"DrawableGameComponent::DrawableGameComponent", "DrawableGameComponent::LoadContent",
             "DrawableGameComponent::Draw", "DrawableGameComponent::Visible", "DrawableGameComponent::GraphicsDevice"},
    .tags = {"component", "drawing"},
    .frames = 10,
    .deterministic = true,
})
