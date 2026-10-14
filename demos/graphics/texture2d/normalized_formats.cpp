// SPDX-License-Identifier: MS-PL
// Normalized integer Texture2D formats: exact packed storage and sampled channel interpretation.
#include <array>
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Alpha8.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/NormalizedByte2.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/NormalizedByte4.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Rg32.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Rgba1010102.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Rgba64.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Graphics::PackedVector;

namespace
{
    class NormalizedFormatsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            textures_[0] = std::make_unique<Texture2D>(device, 1, 1, false,
                                                       SurfaceFormat::NormalizedByte2);
            textures_[1] = std::make_unique<Texture2D>(device, 1, 1, false,
                                                       SurfaceFormat::NormalizedByte4);
            textures_[2] = std::make_unique<Texture2D>(device, 1, 1, false,
                                                       SurfaceFormat::Rgba1010102);
            textures_[3] = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Rg32);
            textures_[4] = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Rgba64);
            textures_[5] = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Alpha8);

            const NormalizedByte2 signed2(-0.5f, 0.25f);
            const NormalizedByte4 signed4(-0.5f, 0.25f, 1.0f, 0.5f);
            const Rgba1010102 rgb10(0.5f, 0.25f, 1.0f, 0.5f);
            const Rg32 rg16(0.5f, 0.25f);
            const Rgba64 rgba16(0.5f, 0.25f, 1.0f, 0.5f);
            const Alpha8 alpha(0.5f);
            textures_[0]->SetData(&signed2, 1);
            textures_[1]->SetData(&signed4, 1);
            textures_[2]->SetData(&rgb10, 1);
            textures_[3]->SetData(&rg16, 1);
            textures_[4]->SetData(&rgba16, 1);
            textures_[5]->SetData(&alpha, 1);

            Context().Check(textures_[0]->getFormatProperty() == SurfaceFormat::NormalizedByte2 &&
                                textures_[1]->getFormatProperty() == SurfaceFormat::NormalizedByte4 &&
                                textures_[2]->getFormatProperty() == SurfaceFormat::Rgba1010102 &&
                                textures_[3]->getFormatProperty() == SurfaceFormat::Rg32 &&
                                textures_[4]->getFormatProperty() == SurfaceFormat::Rgba64 &&
                                textures_[5]->getFormatProperty() == SurfaceFormat::Alpha8,
                            "the six textures retain their distinct normalized channel layouts");

            NormalizedByte2 signed2Read;
            NormalizedByte4 signed4Read;
            Rgba1010102 rgb10Read;
            Rg32 rg16Read;
            Rgba64 rgba16Read;
            Alpha8 alphaRead;
            textures_[0]->GetData(&signed2Read, 1);
            textures_[1]->GetData(&signed4Read, 1);
            textures_[2]->GetData(&rgb10Read, 1);
            textures_[3]->GetData(&rg16Read, 1);
            textures_[4]->GetData(&rgba16Read, 1);
            textures_[5]->GetData(&alphaRead, 1);
            Context().Check(signed2Read == signed2 && signed4Read == signed4,
                            "signed-normalized textures round-trip their exact packed bytes");
            Context().Check(rgb10Read == rgb10 && rg16Read == rg16 && rgba16Read == rgba16,
                            "wide unsigned-normalized textures round-trip their exact packed words");
            Context().Check(alphaRead == alpha,
                            "the alpha-only texture round-trips its exact packed byte");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque,
                                &SamplerState::PointClamp, nullptr, nullptr);
            for (std::size_t i = 0; i < textures_.size(); ++i)
                spriteBatch_->Draw(*textures_[i], Rectangle(32 + static_cast<int>(i) * 128,
                                                            120, 96, 240), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                std::array<Color, 6> sampled;
                for (std::size_t i = 0; i < sampled.size(); ++i)
                    sampled[i] = pixels.At(80 + static_cast<int>(i) * 128, 240);

                Context().Print("sampled signed2 {}, signed4 {}, rgb10 {}", sampled[0].ToString(),
                                sampled[1].ToString(), sampled[2].ToString());
                Context().Print("sampled rg16 {}, rgba16 {}, alpha {}", sampled[3].ToString(),
                                sampled[4].ToString(), sampled[5].ToString());
                Context().Check(CnaDemos::Near(sampled[0], Color(0, 64, 255, 255), 2),
                                "NormalizedByte2 clamps negative red only at Color output and expands B/A to one");
                Context().Check(CnaDemos::Near(sampled[1], Color(0, 64, 255, 129), 2),
                                "NormalizedByte4 retains signed-normalized G/B/A while Color clamps negative red");
                Context().Check(CnaDemos::Near(sampled[2], Color(128, 64, 255, 170), 2),
                                "Rgba1010102 exposes its coarse two-bit alpha quantization when sampled");
                Context().Check(CnaDemos::Near(sampled[3], Color(128, 64, 255, 255), 2),
                                "Rg32 expands missing blue and alpha channels to one");
                Context().Check(CnaDemos::Near(sampled[4], Color(128, 64, 255, 128), 2),
                                "Rgba64 samples all four unsigned-normalized channels");
                Context().Check(CnaDemos::Near(sampled[5], Color(0, 0, 0, 128), 2),
                                "Alpha8 expands missing color channels to zero and keeps its alpha");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::array<std::unique_ptr<Texture2D>, 6> textures_;
    };
}

CNA_DEMOS_GAME(NormalizedFormatsDemo, {
    .id = "graphics.texture2d.normalized_formats",
    .title = "Normalized integer texture formats",
    .description = "Round-trips six packed normalized Texture2D layouts, then point-samples signed clamping, missing-channel expansion, wide UNorm precision and alpha-only storage.",
    .apis = {"Texture2D::Texture2D", "Texture2D::SetData", "Texture2D::GetData", "Texture::Format",
             "SurfaceFormat::NormalizedByte2", "SurfaceFormat::NormalizedByte4",
             "SurfaceFormat::Rgba1010102", "SurfaceFormat::Rg32", "SurfaceFormat::Rgba64",
             "SurfaceFormat::Alpha8", "NormalizedByte2", "NormalizedByte4", "Rgba1010102",
             "Rg32", "Rgba64", "Alpha8", "SpriteBatch::Draw"},
    .tags = {"texture", "format", "normalized", "packed", "sampling"},
    .frames = 3,
    .deterministic = true,
    .restrictions = "HiDef normalized integer texture formats. Exact packed data and sampled back-buffer pixels are checked; no screenshot or manual visual inspection is claimed.",
})
