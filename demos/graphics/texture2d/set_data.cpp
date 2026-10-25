// SPDX-License-Identifier: MS-PL
// Texture2D::SetData: build a texture from pixels computed in code.
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Size = 128;

    class SetDataDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());

            // One Color per pixel, row by row from the top-left corner.
            std::vector<Color> pixels(static_cast<std::size_t>(Size) * Size);
            std::vector<std::uint8_t> rgba(static_cast<std::size_t>(Size) * Size * 4);
            for (int y = 0; y < Size; ++y)
            {
                for (int x = 0; x < Size; ++x)
                {
                    // An XOR pattern: a classic procedural texture.
                    const int value = (x ^ y) * 2;
                    pixels[static_cast<std::size_t>(y) * Size + x] = Color(value, 255 - value, 160);
                    const std::size_t byte = (static_cast<std::size_t>(y) * Size + x) * 4;
                    rgba[byte] = static_cast<std::uint8_t>(value);
                    rgba[byte + 1] = static_cast<std::uint8_t>(255 - value);
                    rgba[byte + 2] = 160;
                    rgba[byte + 3] = 255;
                }
            }

            // A Color texture of the right size, then its whole top mip level replaced at once.
            texture_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), Size, Size);
            texture_->SetData(pixels.data(), static_cast<int>(pixels.size()));

            // GetData returns exactly what SetData stored.
            std::vector<Color> readBack(pixels.size());
            texture_->GetData(readBack.data(), static_cast<int>(readBack.size()));
            Context().Check(readBack == pixels, "GetData returns the pixels SetData stored");

            // CNA's convenience factory consumes tightly packed RGBA8 bytes. XNA-compatible
            // code uses the constructor + SetData route above; these two textures should agree.
            fromPixels_ = std::make_unique<Texture2D>(
                Texture2D::CreateFromPixels(getGraphicsDeviceProperty(), Size, Size, rgba));
            std::vector<Color> factoryReadBack(pixels.size());
            fromPixels_->GetData(factoryReadBack.data(), static_cast<int>(factoryReadBack.size()));
            Context().Check(factoryReadBack == pixels,
                            "CreateFromPixels stores the same RGBA8 texels as constructor plus SetData");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*texture_, Rectangle(72, 80, 288, 288), Color::White);
            spriteBatch_->Draw(*fromPixels_, Rectangle(440, 80, 288, 288), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> texture_;
        std::unique_ptr<Texture2D> fromPixels_;
    };
}

CNA_DEMOS_GAME(SetDataDemo, {
    .id = "graphics.texture2d.set_data",
    .title = "Procedural texture",
    .description = "Builds the same procedural XOR texture through XNA's Texture2D constructor plus SetData and "
                   "CNA's CreateFromPixels RGBA8 factory, then reads both back and compares their exact texels.",
    .apis = {"Texture2D::Texture2D", "Texture2D::SetData", "Texture2D::CreateFromPixels", "Texture2D::GetData"},
    .tags = {"texture", "procedural", "pixels", "rgba"},
    .frames = 3,
    .deterministic = true,
})
