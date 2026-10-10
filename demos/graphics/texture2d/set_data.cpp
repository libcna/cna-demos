// SPDX-License-Identifier: MS-PL
// Texture2D::SetData: build a texture from pixels computed in code.
#include <cstddef>
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
            for (int y = 0; y < Size; ++y)
            {
                for (int x = 0; x < Size; ++x)
                {
                    // An XOR pattern: a classic procedural texture.
                    const int value = (x ^ y) * 2;
                    pixels[static_cast<std::size_t>(y) * Size + x] = Color(value, 255 - value, 160);
                }
            }

            // A Color texture of the right size, then its whole top mip level replaced at once.
            texture_ = std::make_unique<Texture2D>(getGraphicsDeviceProperty(), Size, Size);
            texture_->SetData(pixels.data(), static_cast<int>(pixels.size()));

            // GetData returns exactly what SetData stored.
            std::vector<Color> readBack(pixels.size());
            texture_->GetData(readBack.data(), static_cast<int>(readBack.size()));
            Context().Check(readBack == pixels, "GetData returns the pixels SetData stored");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->Draw(*texture_, Rectangle(208, 48, 384, 384), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::unique_ptr<Texture2D> texture_;
    };
}

CNA_DEMOS_GAME(SetDataDemo, {
    .id = "graphics.texture2d.set_data",
    .title = "Procedural texture",
    .description = "Creates an empty Texture2D, fills it with an XOR pattern computed in code through SetData and "
                   "reads it back with GetData.",
    .apis = {"Texture2D::Texture2D", "Texture2D::SetData", "Texture2D::GetData"},
    .tags = {"texture", "procedural", "pixels"},
    .frames = 3,
    .deterministic = true,
})
