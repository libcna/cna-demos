// SPDX-License-Identifier: MS-PL
#pragma once

// Helpers for checking rendered pixels in Game demos. They only read and compare; whatever the
// demo is about is still drawn with CNA calls in the demo itself. Reading the back buffer is
// GraphicsDevice::GetBackBufferData, which XNA allows under the HiDef profile only (the profile
// cna-demos runs in); graphics.device.get_back_buffer_data shows that API on its own.

#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

namespace CnaDemos
{
    /** A copy of the current back buffer, taken when constructed (call it before Game::Draw ends). */
    class BackBufferPixels
    {
    public:
        explicit BackBufferPixels(Microsoft::Xna::Framework::Graphics::GraphicsDevice& device)
            : width_(device.getPresentationParametersProperty().getBackBufferWidthProperty()),
              height_(device.getPresentationParametersProperty().getBackBufferHeightProperty()),
              pixels_(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_))
        {
            device.GetBackBufferData(pixels_.data(), static_cast<int>(pixels_.size()));
        }

        [[nodiscard]] Microsoft::Xna::Framework::Color At(int x, int y) const
        {
            return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
        }

        [[nodiscard]] int Width() const { return width_; }
        [[nodiscard]] int Height() const { return height_; }

    private:
        int width_;
        int height_;
        std::vector<Microsoft::Xna::Framework::Color> pixels_;
    };

    /** True when every channel of a and b differs by at most tolerance (renderers round differently). */
    [[nodiscard]] inline bool Near(const Microsoft::Xna::Framework::Color& a, const Microsoft::Xna::Framework::Color& b,
                                   int tolerance)
    {
        return std::abs(a.getRProperty() - b.getRProperty()) <= tolerance &&
               std::abs(a.getGProperty() - b.getGProperty()) <= tolerance &&
               std::abs(a.getBProperty() - b.getBProperty()) <= tolerance &&
               std::abs(a.getAProperty() - b.getAProperty()) <= tolerance;
    }
}
