// SPDX-License-Identifier: MS-PL
// SpriteSortMode::Immediate: each Draw is drawn at once, so device state changed between Draws applies.
#include <cstddef>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerStateCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    const Rectangle Left(20, 40, 370, 370);
    const Rectangle Right(410, 40, 370, 370);

    std::vector<Color> ReadBackBuffer(GraphicsDevice& device)
    {
        const auto& parameters = device.getPresentationParametersProperty();
        std::vector<Color> pixels(static_cast<std::size_t>(parameters.getBackBufferWidthProperty()) *
                                  parameters.getBackBufferHeightProperty());
        device.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));
        return pixels;
    }

    // --- Blend state changed between two Draws -------------------------------------------------
    class ImmediateBlendStateDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color(0, 0, 128));

            // Begin applies its states to the device right away in Immediate mode...
            spriteBatch_->Begin(SpriteSortMode::Immediate, &BlendState::Opaque, &SamplerState::PointClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(checker_, Left, Color::White);

            // ...and each Draw is sent immediately, so a device state changed in between applies
            // to the sprites drawn after the change only. (In Deferred mode End would apply the
            // batch's own states and the change would be lost.)
            device.setBlendStateProperty(BlendState::Additive);
            spriteBatch_->Draw(checker_, Right, Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const std::vector<Color> pixels = ReadBackBuffer(device);
                const int width = device.getPresentationParametersProperty().getBackBufferWidthProperty();
                // The middle of the second checker cell (a dark one, (72, 61, 139)); added to the dark
                // blue background its blue channel saturates.
                const int cell = Left.Width / 8;
                const Color opaque = pixels[static_cast<std::size_t>(Left.Y + cell / 2) * width + Left.X + cell * 3 / 2];
                const Color added = pixels[static_cast<std::size_t>(Right.Y + cell / 2) * width + Right.X + cell * 3 / 2];
                Context().Print("first sprite {}, second sprite {}", opaque.ToString(), added.ToString());
                Context().Check(opaque.getBProperty() == 139, "the first sprite replaced the background (Opaque)");
                Context().Check(added.getBProperty() == 255, "the second sprite was added to it (Additive)");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
    };

    // --- Sampler state changed between two Draws -----------------------------------------------
    class ImmediateSamplerStateDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            checker_ = getContentProperty().Load<Texture2D>("textures/checker");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);

            spriteBatch_->Begin(SpriteSortMode::Immediate, &BlendState::Opaque, &SamplerState::PointClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(checker_, Left, Color::White);
            // Same idea with the sampler: the second sprite should be filtered linearly.
            device.getSamplerStatesProperty()(0, SamplerState::LinearClamp);
            spriteBatch_->Draw(checker_, Right, Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const std::vector<Color> pixels = ReadBackBuffer(device);
                const int width = device.getPresentationParametersProperty().getBackBufferWidthProperty();
                // Point sampling produces only the checker's two colours along a row; linear
                // filtering also produces the blends between them.
                const auto distinct = [&](const Rectangle& area) {
                    std::vector<Color> seen;
                    for (int x = area.X; x < area.X + area.Width; ++x)
                    {
                        const Color c = pixels[static_cast<std::size_t>(200) * width + x];
                        bool known = false;
                        for (const Color& s : seen)
                            known = known || s == c;
                        if (!known)
                            seen.push_back(c);
                    }
                    return seen.size();
                };
                Context().Print("distinct colours in one row: first sprite {}, second sprite {}", distinct(Left),
                                distinct(Right));
                Context().Check(distinct(Left) == 2, "the first sprite used Begin's PointClamp");
                Context().Check(distinct(Right) > 2, "the second sprite used the LinearClamp set between the Draws");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        Texture2D checker_;
    };
}

CNA_DEMOS_GAME(ImmediateBlendStateDemo, {
    .id = "graphics.sprite_batch.immediate_mode.blend_state",
    .title = "Immediate mode: change blending between sprites",
    .description = "Uses SpriteSortMode::Immediate and sets GraphicsDevice::BlendState between two Draw calls: the "
                   "first sprite is drawn opaque, the second additively.",
    .apis = {"SpriteBatch::Begin", "SpriteSortMode::Immediate", "GraphicsDevice::BlendState", "BlendState::Additive",
             "SpriteBatch::Draw"},
    .tags = {"sprite", "immediate", "state", "blend"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
})

CNA_DEMOS_GAME(ImmediateSamplerStateDemo, {
    .id = "graphics.sprite_batch.immediate_mode.sampler_state",
    .title = "Immediate mode: change sampling between sprites",
    .description = "Uses SpriteSortMode::Immediate and sets GraphicsDevice::SamplerStates[0] between two Draw calls: "
                   "the first sprite should be point-sampled, the second linearly filtered.",
    .apis = {"SpriteBatch::Begin", "SpriteSortMode::Immediate", "GraphicsDevice::SamplerStates",
             "SamplerStateCollection::Item", "SpriteBatch::Draw"},
    .tags = {"sprite", "immediate", "state", "sampler"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/checker.png"},
    .knownIssue = "CNA-DEMOS-ISSUE-004",
})
