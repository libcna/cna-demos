// SPDX-License-Identifier: MS-PL
// Texture3D: a volume of texels (slices), written whole and by sub-box, and read back slice by slice.
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture3D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    constexpr int Size = 32;
    constexpr int Depth = 4;

    class VolumeDataDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));

            // A 32x32x4 volume, e.g. a colour lookup table or a 3D noise texture for a shader.
            Texture3D volume(device, Size, Size, Depth, false, SurfaceFormat::Color);
            Context().Check(volume.getWidthProperty() == Size && volume.getHeightProperty() == Size &&
                                volume.getDepthProperty() == Depth,
                            "Width, Height and Depth describe the volume");

            // Whole volume: slice by slice, each slice row by row. Slice z gets its own tint.
            std::vector<Color> texels(Size * Size * Depth);
            for (int z = 0; z < Depth; ++z)
            {
                for (int y = 0; y < Size; ++y)
                {
                    for (int x = 0; x < Size; ++x)
                    {
                        texels[(z * Size + y) * Size + x] =
                            Color(x * 8, y * 8, 64 * z + 63, 255);
                    }
                }
            }
            volume.SetData(texels.data(), static_cast<int>(texels.size()));

            // A sub-box: an 8x8 white square in slices 1 and 2 only (front 1, back 3 exclusive).
            const std::vector<Color> white(8 * 8 * 2, Color::White);
            volume.SetData(0, 12, 12, 20, 20, 1, 3, white.data(), 0, static_cast<int>(white.size()));

            std::vector<Color> back(texels.size());
            volume.GetData(back.data(), static_cast<int>(back.size()));
            const auto at = [&](int x, int y, int z) { return back[(z * Size + y) * Size + x]; };
            Context().Check(at(3, 5, 0) == texels[(0 * Size + 5) * Size + 3], "texels outside the box are unchanged");
            Context().Check(at(15, 15, 1) == Color::White && at(15, 15, 2) == Color::White &&
                                at(15, 15, 0) != Color::White && at(15, 15, 3) != Color::White,
                            "the box touched slices 1 and 2 only");

            // Show each slice as an ordinary 2D texture made from the read-back data.
            for (int z = 0; z < Depth; ++z)
            {
                slices_.emplace_back(device, Size, Size);
                slices_.back().SetData(back.data() + z * Size * Size, 0, Size * Size);
            }
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(30, 30, 30));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "Texture3D 32x32x4, read back slice by slice", Vector2(20.0f, 20.0f), Color::White);
            for (int z = 0; z < Depth; ++z)
            {
                const int x = 30 + z * 190;
                spriteBatch_->Draw(slices_[static_cast<std::size_t>(z)], Rectangle(x, 100, 160, 160), Color::White);
                spriteBatch_->DrawString(*font_, "slice " + std::to_string(z), Vector2(static_cast<float>(x), 270.0f),
                                         Color::LightGray);
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::vector<Texture2D> slices_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(VolumeDataDemo, {
    .id = "graphics.texture3d.volume_data",
    .title = "3D textures",
    .description = "Creates a 32x32x4 Texture3D, fills it slice by slice with SetData, overwrites a sub-box spanning "
                   "two slices, reads the volume back with GetData and shows each slice as a 2D texture.",
    .apis = {"Texture3D::Texture3D", "Texture3D::SetData", "Texture3D::GetData", "Texture3D::Width",
             "Texture3D::Height", "Texture3D::Depth"},
    .tags = {"texture", "volume", "3d-texture", "lookup-table"},
    .needs = CnaDemos::Need::Texture3D,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
})
