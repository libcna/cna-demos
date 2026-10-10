// SPDX-License-Identifier: MS-PL
// Render into a RenderTarget2D and read the result on the CPU with GetData.
#include <cstddef>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ReadBackDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            arrow_ = getContentProperty().Load<Texture2D>("textures/arrow");
            target_ = std::make_unique<RenderTarget2D>(device, 64, 64);

            device.SetRenderTarget(target_.get());
            device.Clear(Color::Transparent);
            spriteBatch_->Begin();
            spriteBatch_->Draw(arrow_, Vector2::Zero, Color::White);
            spriteBatch_->End();

            // While the target is bound it cannot be read: switch back to the back buffer first.
            bool refused = false;
            try
            {
                std::vector<Color> early(64 * 64);
                target_->GetData(early.data(), static_cast<int>(early.size()));
            }
            catch (const System::InvalidOperationException& e)
            {
                refused = true;
                Context().Print("GetData while bound: {}", e.what());
            }
            Context().Check(refused, "GetData on the bound render target throws");
            device.SetRenderTarget(nullptr);

            // Now the rendered pixels can be read like any texture's -- e.g. to save a screenshot of
            // an off-screen view or to analyse it on the CPU.
            std::vector<Color> pixels(64 * 64);
            target_->GetData(pixels.data(), static_cast<int>(pixels.size()));
            std::vector<Color> source(64 * 64);
            arrow_.GetData(source.data(), static_cast<int>(source.size()));
            int opaque = 0;
            for (const Color& c : pixels)
                opaque += c.getAProperty() == 255 ? 1 : 0;
            Context().Print("{} of {} rendered pixels are opaque", opaque, pixels.size());
            Context().Check(pixels[static_cast<std::size_t>(32) * 64 + 20] == source[static_cast<std::size_t>(32) * 64 + 20],
                            "an opaque arrow pixel was rendered unchanged");
            Context().Check(pixels[static_cast<std::size_t>(60) * 64 + 60] == Color::Transparent,
                            "an empty corner keeps the transparent clear colour");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::DarkSlateBlue);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*target_, Rectangle(272, 112, 256, 256), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D arrow_;
        std::unique_ptr<RenderTarget2D> target_;
    };
}

CNA_DEMOS_GAME(ReadBackDemo, {
    .id = "graphics.render_target.read_back",
    .title = "Read a render target",
    .description = "Renders a sprite into a RenderTarget2D and reads the pixels back with GetData after unbinding it "
                   "(reading while it is bound throws).",
    .apis = {"Texture2D::GetData", "RenderTarget2D::RenderTarget2D", "GraphicsDevice::SetRenderTarget"},
    .tags = {"render-target", "readback", "offscreen", "pixels"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/arrow.png"},
})
