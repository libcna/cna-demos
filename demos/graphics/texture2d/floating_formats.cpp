// SPDX-License-Identifier: MS-PL
// Binary32 Texture2D formats: exact CPU data and their sampled channel interpretation.
#include <memory>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "System/ArgumentException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class FloatingFormatsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            single_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Single);
            vector2_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Vector2);
            vector4_ = std::make_unique<Texture2D>(device, 1, 1, false, SurfaceFormat::Vector4);

            const float singleValue = 0.25f;
            const Vector2 vector2Value(0.25f, 0.5f);
            const Vector4 vector4Value(2.0f, 0.5f, -1.0f, 1.0f);
            single_->SetData(&singleValue, 1);
            vector2_->SetData(&vector2Value, 1);
            vector4_->SetData(&vector4Value, 1);

            Context().Check(single_->getFormatProperty() == SurfaceFormat::Single &&
                                vector2_->getFormatProperty() == SurfaceFormat::Vector2 &&
                                vector4_->getFormatProperty() == SurfaceFormat::Vector4,
                            "the three textures retain their binary32 channel formats");

            float singleRead = 0.0f;
            Vector2 vector2Read;
            Vector4 vector4Read;
            single_->GetData(&singleRead, 1);
            vector2_->GetData(&vector2Read, 1);
            vector4_->GetData(&vector4Read, 1);
            Context().Check(singleRead == singleValue, "Single round-trips its exact float value");
            Context().Check(vector2Read == vector2Value, "Vector2 round-trips both exact float channels");
            Context().Check(vector4Read == vector4Value,
                            "Vector4 round-trips HDR and negative channels without clamping");

            bool wrongWidthRejected = false;
            try
            {
                single_->SetData(&vector2Value, 1);
            }
            catch (const System::ArgumentException&)
            {
                wrongWidthRejected = true;
            }
            single_->GetData(&singleRead, 1);
            Context().Check(wrongWidthRejected && singleRead == singleValue,
                            "an element wider than Single is rejected without changing its value");
        }

        void Draw(const GameTime& gameTime) override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            device.Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, &BlendState::Opaque,
                                &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*single_, Rectangle(64, 120, 192, 240), Color::White);
            spriteBatch_->Draw(*vector2_, Rectangle(304, 120, 192, 240), Color::White);
            spriteBatch_->Draw(*vector4_, Rectangle(544, 120, 192, 240), Color::White);
            spriteBatch_->End();

            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                const Color singlePixel = pixels.At(160, 240);
                const Color vector2Pixel = pixels.At(400, 240);
                const Color vector4Pixel = pixels.At(640, 240);
                Context().Print("sampled Single {}, Vector2 {}, Vector4 {}", singlePixel.ToString(),
                                vector2Pixel.ToString(), vector4Pixel.ToString());
                Context().Check(CnaDemos::Near(singlePixel, Color(64, 255, 255, 255), 2),
                                "sampling Single expands missing G, B and A channels to one");
                Context().Check(CnaDemos::Near(vector2Pixel, Color(64, 128, 255, 255), 2),
                                "sampling Vector2 expands missing B and A channels to one");
                Context().Check(CnaDemos::Near(vector4Pixel, Color(255, 128, 0, 255), 2),
                                "the Color back buffer clamps sampled Vector4 HDR and negative channels");
            }

            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> single_;
        std::unique_ptr<Texture2D> vector2_;
        std::unique_ptr<Texture2D> vector4_;
    };
}

CNA_DEMOS_GAME(FloatingFormatsDemo, {
    .id = "graphics.texture2d.floating_formats",
    .title = "Floating-point texture formats",
    .description = "Stores exact binary32 values in Single, Vector2 and Vector4 textures, rejects a mismatched element width, then point-samples their channel expansion and final Color-buffer clamping.",
    .apis = {"Texture2D::Texture2D", "Texture2D::SetData", "Texture2D::GetData", "Texture::Format",
             "SurfaceFormat::Single", "SurfaceFormat::Vector2", "SurfaceFormat::Vector4",
             "SpriteBatch::Draw"},
    .tags = {"texture", "format", "float", "hdr", "sampling"},
    .frames = 3,
    .deterministic = true,
    .restrictions = "HiDef binary32 texture formats. Exact raw data and sampled back-buffer pixels are checked; no screenshot or manual visual inspection is claimed.",
})
