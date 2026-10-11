// SPDX-License-Identifier: MS-PL
// A floating-point render target (SurfaceFormat::HdrBlendable): light adds up past 1.0 instead of
// saturating, as an HDR renderer needs before tone mapping.
#include <cmath>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/HalfVector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Xna::Framework::Graphics::PackedVector::HalfVector4;

namespace
{
    constexpr int Size = 64;
    // Three overlapping lights of this colour land on the centre of each target.
    const Color Light(0.8f, 0.5f, 0.2f);

    std::string Format(const Vector4& v)
    {
        char text[64];
        std::snprintf(text, sizeof(text), "(%.2f, %.2f, %.2f)", v.X, v.Y, v.Z);
        return text;
    }

    class HdrBlendableDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            white_ = std::make_unique<Texture2D>(device, 1, 1);
            const Color white = Color::White;
            white_->SetData(&white, 1);

            // The same scene into two targets: 8 bits per channel, and 16-bit floats.
            colorTarget_ = std::make_unique<RenderTarget2D>(device, Size, Size, false, SurfaceFormat::Color,
                                                            DepthFormat::None);
            hdrTarget_ = std::make_unique<RenderTarget2D>(device, Size, Size, false, SurfaceFormat::HdrBlendable,
                                                          DepthFormat::None);
            Context().Check(hdrTarget_->getFormatProperty() == SurfaceFormat::HdrBlendable,
                            "the target keeps its HdrBlendable format");
        }

        // Three lights, added together (BlendState::Additive), overlapping in the middle.
        void DrawLights(RenderTarget2D& target)
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.SetRenderTarget(&target);
            device.Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Additive, &SamplerState::PointClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(*white_, Rectangle(8, 8, 40, 40), Light);
            spriteBatch_->Draw(*white_, Rectangle(16, 16, 40, 40), Light);
            spriteBatch_->Draw(*white_, Rectangle(24, 24, 32, 32), Light);
            spriteBatch_->End();
            device.SetRenderTarget(nullptr);
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            DrawLights(*colorTarget_);
            DrawLights(*hdrTarget_);

            if (FramesDrawn() == 0)
            {
                // The centre pixel, where all three lights overlap.
                const int centre = (Size / 2) * Size + Size / 2;
                std::vector<Color> clamped(Size * Size);
                colorTarget_->GetData(clamped.data(), Size * Size);
                std::vector<HalfVector4> hdr(Size * Size);
                hdrTarget_->GetData(hdr.data(), Size * Size);
                const Vector4 inColor = clamped[centre].ToVector4();
                const Vector4 inHdr = hdr[centre].ToVector4();
                colorText_ = "Color target:        " + Format(inColor);
                hdrText_ = "HdrBlendable target: " + Format(inHdr);
                Context().Print("{}\n{}", colorText_, hdrText_);
                // 3 x (0.8, 0.5, 0.2) = (2.4, 1.5, 0.6): the 8-bit target saturates at 1.0.
                Context().Check(clamped[centre] == Color(255, 255, 153), "the Color target saturates at 1.0");
                Context().Check(std::abs(inHdr.X - 2.4f) < 0.02f && std::abs(inHdr.Y - 1.51f) < 0.02f &&
                                    std::abs(inHdr.Z - 0.6f) < 0.02f,
                                "the HdrBlendable target holds the sum, (2.4, 1.5, 0.6)");
            }

            // A display shows 0..1, so an HDR renderer maps the float target into that range (tone
            // mapping). The simplest such mapping is an exposure: both targets drawn at 0.4. The
            // float target's levels above 1 reappear; the 8-bit target has lost them.
            device.Clear(Color(24, 24, 32));
            const Color exposure(0.4f, 0.4f, 0.4f);
            // Opaque: the targets' alpha summed up too (to 4 in the middle) and is not a coverage.
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque, &SamplerState::PointClamp, nullptr,
                                nullptr);
            spriteBatch_->Draw(*colorTarget_, Rectangle(80, 120, 256, 256), exposure);
            spriteBatch_->Draw(*hdrTarget_, Rectangle(464, 120, 256, 256), exposure);
            spriteBatch_->End();
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Color, exposure 0.4", Vector2(80.0f, 80.0f), Color::White);
            spriteBatch_->DrawString(*font_, "HdrBlendable, exposure 0.4", Vector2(464.0f, 80.0f), Color::White);
            spriteBatch_->DrawString(*font_, colorText_, Vector2(80.0f, 400.0f), Color::LightGray);
            spriteBatch_->DrawString(*font_, hdrText_, Vector2(80.0f, 430.0f), Color::Gold);
            spriteBatch_->End();
            if (FramesDrawn() == 0)
            {
                // Two lights overlap at target pixel (20, 20), three at (40, 40); 4 screen pixels each.
                const CnaDemos::BackBufferPixels screen(device);
                const int twoColor = screen.At(80 + 82, 120 + 82).getRProperty();
                const int threeColor = screen.At(80 + 162, 120 + 162).getRProperty();
                const int twoHdr = screen.At(464 + 82, 120 + 82).getRProperty();
                const int threeHdr = screen.At(464 + 162, 120 + 162).getRProperty();
                Context().Print("red at exposure 0.4, two lights / three lights: Color {} / {}, HdrBlendable {} / {}",
                                twoColor, threeColor, twoHdr, threeHdr);
                Context().Check(twoColor == threeColor && threeHdr - twoHdr > 50,
                                "drawn darker, the float target still tells two lights from three");
            }
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
        std::unique_ptr<Texture2D> white_;
        std::unique_ptr<RenderTarget2D> colorTarget_;
        std::unique_ptr<RenderTarget2D> hdrTarget_;
        std::string colorText_;
        std::string hdrText_;
    };
}

CNA_DEMOS_GAME(HdrBlendableDemo, {
    .id = "graphics.render_target.hdr_blendable",
    .title = "Floating-point render target (HDR)",
    .description = "Adds three overlapping lights into a SurfaceFormat::Color target and an HdrBlendable one: the "
                   "8-bit target saturates at 1.0, the floating-point target keeps the sum (2.4, 1.5, 0.6), read "
                   "back as HalfVector4; drawn at an exposure of 0.4, only the float target still shows the "
                   "brighter levels.",
    .apis = {"SurfaceFormat::HdrBlendable", "RenderTarget2D::RenderTarget2D", "Texture2D::GetData", "HalfVector4",
             "HalfVector4::ToVector4", "BlendState::Additive", "BlendState::Opaque", "Texture::Format"},
    .tags = {"render-target", "hdr", "float", "blending"},
    .needs = CnaDemos::Need::HalfFloatRenderTargets,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
