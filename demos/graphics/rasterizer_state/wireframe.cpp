// SPDX-License-Identifier: MS-PL
// RasterizerState::FillMode WireFrame: draw only the edges of triangles -- here, of sprites.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/CullMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/FillMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class WireframeDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");

            wireframe_ = std::make_unique<RasterizerState>();
            wireframe_->setFillModeProperty(FillMode::WireFrame);
            wireframe_->setCullModeProperty(CullMode::None);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            // Solid: the usual filled quad.
            spriteBatch_->Begin();
            spriteBatch_->Draw(checker_, Rectangle(40, 80, 320, 320), Color::White);
            spriteBatch_->End();

            // Wireframe: each sprite is two triangles, so the outline and one diagonal are drawn.
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, nullptr, nullptr, wireframe_.get());
            spriteBatch_->Draw(checker_, Rectangle(440, 80, 320, 320), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                int solid = 0;
                int wire = 0;
                for (int y = 80; y < 400; ++y)
                {
                    for (int x = 0; x < 320; ++x)
                    {
                        solid += pixels.At(40 + x, y) != Color::Black ? 1 : 0;
                        wire += pixels.At(440 + x, y) != Color::Black ? 1 : 0;
                    }
                }
                Context().Print("pixels drawn: solid {}, wireframe {}", solid, wire);
                Context().Check(solid == 320 * 320, "the solid sprite covers its rectangle");
                Context().Check(wire > 0 && wire < 320 * 320 / 50, "the wireframe sprite draws only lines");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
        std::unique_ptr<RasterizerState> wireframe_;
    };
}

CNA_DEMOS_GAME(WireframeDemo, {
    .id = "graphics.rasterizer_state.wireframe",
    .title = "Wireframe rendering",
    .description = "Draws the same sprite solid and through a RasterizerState with FillMode::WireFrame, which shows "
                   "only the edges of the sprite's two triangles.",
    .apis = {"RasterizerState::FillMode", "FillMode::WireFrame", "FillMode::Solid", "RasterizerState::CullMode",
             "SpriteBatch::Begin"},
    .tags = {"rasterizer", "wireframe", "debug", "state"},
    .needs = CnaDemos::Need::WireFrame,
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})
