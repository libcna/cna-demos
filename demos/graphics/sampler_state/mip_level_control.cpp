// SPDX-License-Identifier: MS-PL
// SamplerState::MaxMipLevel and MipMapLevelOfDetailBias: steer which mip level a sampler reads.
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "CNA/RendererCapabilityProfile.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureFilter.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Size = 128;
    // One colour per level makes the chosen level visible.
    const std::array<Color, 8> LevelColors = {Color::Red,  Color::Orange, Color::Yellow, Color::Lime,
                                              Color::Cyan, Color::Blue,   Color::Purple, Color::White};

    SamplerState MakeSampler(float lodBias, int maxMipLevel)
    {
        SamplerState state;
        state.setFilterProperty(TextureFilter::Point);  // nearest level, nearest texel
        state.setMipMapLevelOfDetailBiasProperty(lodBias);
        state.setMaxMipLevelProperty(maxMipLevel);
        return state;
    }

    // Draws the texture at full, half and quarter size, once with the default sampler and once with
    // the sampler a derived demo is about, and reports which mip level each copy shows.
    class MipLevelDemo : public CnaDemos::DemoGame
    {
    protected:
        MipLevelDemo(SamplerState changed, const char* label, const char* expected, const char* expectation)
            : changed_(std::move(changed)), label_(label), expected_(expected), expectation_(expectation)
        {
        }

        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            texture_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), Size, Size, true, SurfaceFormat::Color);
            for (int level = 0; level < texture_->getLevelCountProperty(); ++level)
            {
                const int size = std::max(1, Size >> level);
                const std::vector<Color> pixels(static_cast<std::size_t>(size) * size,
                                                LevelColors[static_cast<std::size_t>(level)]);
                texture_->SetData(level, nullptr, pixels.data(), 0, static_cast<int>(pixels.size()));
            }
            default_ = MakeSampler(0.0f, 0);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            DrawRow(default_, 0);
            DrawRow(changed_, 1);

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels p(device);
                const auto row = [&](int r) {
                    std::string levels;
                    int x = 300;
                    for (int size = Size; size >= Size / 4; size /= 2)
                    {
                        const Color c = p.At(x + size / 2, 60 + r * 200 + size / 2);
                        const auto found = std::ranges::find(LevelColors, c);
                        levels += (levels.empty() ? "" : " ") +
                                  (found == LevelColors.end() ? std::string("?")
                                                              : std::to_string(found - LevelColors.begin()));
                        x += size + 20;
                    }
                    return levels;
                };
                Context().Print("levels used: default {}, {} {}", row(0), label_, row(1));
                Context().Check(row(0) == "0 1 2", "by default each halving moves one level down");
                Context().Check(row(1) == expected_, expectation_);
            }

            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "default", Vector2(20.0f, 110.0f), Color::White);
            spriteBatch_->DrawString(*font_, label_, Vector2(20.0f, 310.0f), Color::White);
            spriteBatch_->End();

            Game::Draw(gameTime);
        }

    private:
        void DrawRow(const SamplerState& sampler, int row)
        {
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &sampler, nullptr, nullptr);
            int x = 300;
            for (int size = Size; size >= Size / 4; size /= 2)
            {
                spriteBatch_->Draw(*texture_, Rectangle(x, 60 + row * 200, size, size), Color::White);
                x += size + 20;
            }
            spriteBatch_->End();
        }

        SamplerState changed_;
        const char* label_;
        const char* expected_;
        const char* expectation_;
        SamplerState default_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> texture_;
    };

    class MaxMipLevelDemo final : public MipLevelDemo
    {
    public:
        // MaxMipLevel is the largest (most detailed) level the sampler may read: with 2, levels 0
        // and 1 are never used.
        MaxMipLevelDemo()
            : MipLevelDemo(MakeSampler(0.0f, 2), "MaxMipLevel 2", "2 2 2",
                           "MaxMipLevel 2 never reads a level larger than level 2")
        {
        }
    };

    class LodBiasDemo final : public MipLevelDemo
    {
    public:
        // A positive bias selects smaller levels (blurrier), a negative one larger levels (sharper).
        LodBiasDemo()
            : MipLevelDemo(MakeSampler(1.0f, 0), "MipMapLevelOfDetailBias +1", "1 2 3",
                           "MipMapLevelOfDetailBias +1 picks one level smaller everywhere")
        {
        }

    protected:
        void LoadContent() override
        {
            // A renderer without a sampler LOD bias (OpenGL ES has none) accepts the state with no
            // effect, and says so (CNA-DEMOS-ISSUE-006).
            if (!getGraphicsDeviceProperty().SupportsRendererFeatureEXT(CNA::RendererFeature::SamplerLevelOfDetailBias))
                Context().Skip("this renderer does not apply a sampler LOD bias");
            MipLevelDemo::LoadContent();
        }
    };
}

CNA_DEMOS_GAME(MaxMipLevelDemo, {
    .id = "graphics.sampler_state.mip_level_control.max_mip_level",
    .title = "Limit the mip levels a sampler reads",
    .description = "Samples a texture whose mip levels have different colours with SamplerState::MaxMipLevel 2 and "
                   "reports which level each copy used.",
    .apis = {"SamplerState::MaxMipLevel", "TextureFilter::Point", "SamplerState::Filter"},
    .tags = {"sampler", "mipmap", "lod", "state"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})

CNA_DEMOS_GAME(LodBiasDemo, {
    .id = "graphics.sampler_state.mip_level_control.lod_bias",
    .title = "Mip level-of-detail bias",
    .description = "Samples a texture whose mip levels have different colours with SamplerState::"
                   "MipMapLevelOfDetailBias +1 and reports which level each copy used.",
    .apis = {"SamplerState::MipMapLevelOfDetailBias", "TextureFilter::Point", "SamplerState::Filter"},
    .tags = {"sampler", "mipmap", "lod", "bias", "state"},
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Skipped where the renderer reports no RendererFeature::SamplerLevelOfDetailBias (the OpenGL ES "
                    "renderers: GL ES sampler state has no LOD bias).",
})
