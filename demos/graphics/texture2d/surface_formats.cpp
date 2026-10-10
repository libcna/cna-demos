// SPDX-License-Identifier: MS-PL
// Texture2D surface formats: the same gradient stored as Color, Bgr565, Bgra4444 and Bgra5551.
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgr565.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgra4444.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgra5551.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Graphics::PackedVector;

namespace
{
    constexpr int Width = 128;
    constexpr int Height = 16;

    // A smooth ramp from black to a light blue: banding shows how many bits a format keeps.
    Vector4 Ramp(int x)
    {
        const float t = static_cast<float>(x) / (Width - 1);
        return Vector4(0.6f * t, 0.8f * t, t, 1.0f);
    }

    // Bgr565 has no alpha and takes a Vector3; the others take a Vector4.
    template <typename T>
    T Pack(const Vector4& value)
    {
        if constexpr (std::is_same_v<T, Bgr565>)
            return T(Vector3(value.X, value.Y, value.Z));
        else
            return T(value);
    }

    // Fills a texture of the packed type T, row by row.
    template <typename T>
    std::unique_ptr<Texture2D> MakeRamp(GraphicsDevice& device, SurfaceFormat format)
    {
        std::vector<T> pixels(static_cast<std::size_t>(Width) * Height);
        for (int y = 0; y < Height; ++y)
            for (int x = 0; x < Width; ++x)
                pixels[static_cast<std::size_t>(y) * Width + x] = Pack<T>(Ramp(x));

        // The element type passed to SetData must match the texture's format in size.
        auto texture = std::make_unique<Texture2D>(device, Width, Height, false, format);
        texture->SetData(pixels.data(), static_cast<int>(pixels.size()));
        return texture;
    }

    class SurfaceFormatsDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            GraphicsDevice& device = getGraphicsDeviceProperty();
            spriteBatch_ = std::make_unique<SpriteBatch>(device);

            std::vector<Color> colors(static_cast<std::size_t>(Width) * Height);
            for (int y = 0; y < Height; ++y)
                for (int x = 0; x < Width; ++x)
                    colors[static_cast<std::size_t>(y) * Width + x] = Color(Ramp(x));
            textures_.push_back(std::make_unique<Texture2D>(device, Width, Height, false, SurfaceFormat::Color));
            textures_.back()->SetData(colors.data(), static_cast<int>(colors.size()));

            textures_.push_back(MakeRamp<Bgr565>(device, SurfaceFormat::Bgr565));
            textures_.push_back(MakeRamp<Bgra4444>(device, SurfaceFormat::Bgra4444));
            textures_.push_back(MakeRamp<Bgra5551>(device, SurfaceFormat::Bgra5551));

            for (const auto& texture : textures_)
                Context().Print("{:<9} {}x{}, {} bytes per pixel", CnaDemos::Name(texture->getFormatProperty()),
                                texture->getWidthProperty(), texture->getHeightProperty(),
                                Texture::GetFormatSizeEXT(texture->getFormatProperty()));
            Context().Check(textures_[1]->getFormatProperty() == SurfaceFormat::Bgr565,
                            "the texture keeps the format it was created with");

            // Reading back in the same packed type returns the exact stored values.
            std::vector<Bgr565> packed(static_cast<std::size_t>(Width) * Height);
            textures_[1]->GetData(packed.data(), static_cast<int>(packed.size()));
            Context().Check(packed[Width - 1].getPackedValueProperty() == Pack<Bgr565>(Ramp(Width - 1)).getPackedValueProperty(),
                            "GetData with Bgr565 elements returns the stored 16-bit values");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color::Black);
            // Stretched 6x horizontally, with point sampling so the format's steps stay sharp.
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            for (std::size_t i = 0; i < textures_.size(); ++i)
                spriteBatch_->Draw(*textures_[i], Rectangle(16, 40 + static_cast<int>(i) * 104, 768, 80), Color::White);
            spriteBatch_->End();
            Game::Draw(gameTime);
        }

    private:
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::vector<std::unique_ptr<Texture2D>> textures_;
    };
}

CNA_DEMOS_GAME(SurfaceFormatsDemo, {
    .id = "graphics.texture2d.surface_formats",
    .title = "Texture surface formats",
    .description = "Stores one gradient as Color, Bgr565, Bgra4444 and Bgra5551 textures (top to bottom); the "
                   "16-bit formats show visible banding.",
    .apis = {"Texture2D::Texture2D", "Texture::Format", "Texture2D::SetData", "Texture2D::GetData",
             "SurfaceFormat::Color", "SurfaceFormat::Bgr565", "SurfaceFormat::Bgra4444", "SurfaceFormat::Bgra5551"},
    .tags = {"texture", "format", "16-bit", "banding", "packed"},
    .frames = 3,
    .deterministic = true,
})
