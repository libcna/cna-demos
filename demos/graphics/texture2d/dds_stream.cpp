// SPDX-License-Identifier: MS-PL
// Explicit DDS stream loading preserves authored mips; resizing decodes level zero to Color.
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GraphicsNames.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "System/FormatException.hpp"
#include "System/IO/MemoryStream.hpp"
#include "System/IO/SeekOrigin.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class ChunkedStream final : public System::IO::MemoryStream
    {
    public:
        using System::IO::MemoryStream::MemoryStream;
        SharpRuntime::intcs Read(SharpRuntime::bytecs bytes[], SharpRuntime::intcs offset, SharpRuntime::intcs count) override
        {
            ++reads;
            return System::IO::MemoryStream::Read(bytes, offset, std::min<SharpRuntime::intcs>(count, 7));
        }
        int reads = 0;
    };

    class DdsStreamDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty(); auto& ctx = Context();
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            const auto file = TitleContainer::OpenStream("assets/textures/striped_mips.dds");
            if (!ctx.Check(file->getLengthProperty() == 232, "the original DDS contains its header and complete five-level block payload")) return;
            std::vector<std::uint8_t> bytes(232);
            int offset = 0;
            while (offset < static_cast<int>(bytes.size()))
            {
                const int count = file->Read(bytes.data(), offset, static_cast<int>(bytes.size()) - offset);
                if (count <= 0) throw std::runtime_error("DDS fixture ended early");
                offset += count;
            }
            // Model an image entry inside a pack: start after a prefix and allow partial reads.
            std::vector<std::uint8_t> packed{'P', 'A', 'C', 'K'};
            packed.insert(packed.end(), bytes.begin(), bytes.end());
            ChunkedStream stream(packed.data(), static_cast<int>(packed.size()), false);
            stream.Seek(4, System::IO::SeekOrigin::Begin);
            original_ = Texture2D::DDSFromStreamEXT(device, stream);
            ctx.Check(stream.reads > 1 && stream.getPositionProperty() == stream.getLengthProperty(),
                      "DDS loading consumes repeated short reads from the caller's current stream position");
            if (!ctx.Check(original_.getWidthProperty() == 16 && original_.getHeightProperty() == 8 && original_.getLevelCountProperty() == 5,
                           "DDSFromStreamEXT retains the image size and every authored mip level")) return;
            const bool decoded = original_.getFormatProperty() == SurfaceFormat::Color;
            if (!ctx.Check(decoded || original_.getFormatProperty() == SurfaceFormat::Dxt1, "DDS storage is decoded Color or supported native DXT1 blocks")) return;
            ctx.Print("Loaded DDS storage: {}", CnaDemos::Name(original_.getFormatProperty()));
            const std::array<Color, 4> mipColors = {Color::Magenta, Color::Cyan, Color::White, Color::Red};
            const std::array<int, 4> byteCounts = {16, 8, 8, 8};
            int byteOffset = 128 + 64;
            for (int level = 1; level <= 4; ++level)
            {
                bool correct = false;
                if (decoded)
                {
                    std::vector<Color> colors(std::max(1, 16 >> level) * std::max(1, 8 >> level));
                    original_.GetData(level, nullptr, colors.data(), 0, static_cast<int>(colors.size()));
                    correct = std::all_of(colors.begin(), colors.end(), [&](Color c) { return c == mipColors[level - 1]; });
                }
                else
                {
                    std::vector<std::uint8_t> blocks(byteCounts[level - 1]);
                    original_.GetData(level, nullptr, blocks.data(), 0, static_cast<int>(blocks.size()));
                    correct = std::equal(blocks.begin(), blocks.end(), bytes.begin() + byteOffset);
                }
                ctx.Check(correct, "the loaded mip contains its independently authored color, not a generated downsample");
                byteOffset += byteCounts[level - 1];
            }
            System::IO::MemoryStream fitInput(bytes.data(), static_cast<int>(bytes.size()), false);
            fitted_ = Texture2D::DDSFromStreamEXT(device, fitInput, 32, 32, false);
            System::IO::MemoryStream cropInput(bytes.data(), static_cast<int>(bytes.size()), false);
            cropped_ = Texture2D::DDSFromStreamEXT(device, cropInput, 32, 32, true);
            ctx.Check(fitted_.getWidthProperty() == 32 && fitted_.getHeightProperty() == 16 && cropped_.getWidthProperty() == 32 && cropped_.getHeightProperty() == 32,
                      "fit preserves the two-to-one aspect ratio while zoom crops to the square box");
            ctx.Check(fitted_.getFormatProperty() == SurfaceFormat::Color && cropped_.getFormatProperty() == SurfaceFormat::Color &&
                      fitted_.getLevelCountProperty() == 1 && cropped_.getLevelCountProperty() == 1,
                      "resizing decodes level zero to single-level Color textures");
            auto bad = bytes;
            for (bool truncate : {true, false})
            {
                bad = bytes;
                if (truncate) bad.pop_back(); else bad[28] = 4; // incomplete chain: maximum for 16x8 is five
                System::IO::MemoryStream malformed(bad.data(), static_cast<int>(bad.size()), false);
                bool refused = false;
                try { (void)Texture2D::DDSFromStreamEXT(device, malformed); }
                catch (const System::FormatException& error) { refused = true; ctx.Print("DDS refused: {}", error.what()); }
                ctx.Check(refused, truncate ? "a truncated final compressed mip is refused" : "an incomplete declared mip chain is refused");
            }
            ready_ = true;
        }
        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty(); device.Clear(Color(24, 28, 36));
            if (ready_)
            {
                batch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
                batch_->DrawString(*font_, "DDS stream: authored mipmaps, fit and crop", Vector2(32, 32), Color::White);
                batch_->DrawString(*font_, "Original 16x8", Vector2(32, 96), Color::White);
                batch_->DrawString(*font_, "Fit 32x16", Vector2(400, 96), Color::White);
                batch_->DrawString(*font_, "Crop 32x32", Vector2(600, 96), Color::White);
                batch_->Draw(original_, Rectangle(32, 128, 320, 160), Color::White);
                batch_->Draw(fitted_, Rectangle(400, 128, 160, 80), Color::White);
                batch_->Draw(cropped_, Rectangle(600, 128, 160, 160), Color::White);
                batch_->End();
                if (FramesDrawn() == 0)
                {
                    const CnaDemos::BackBufferPixels pixels(device);
                    const std::array<Color, 4> stripes = {Color::Red, Color::Yellow, Color::Lime, Color::Blue};
                    bool originalOk = true, fitOk = true;
                    for (int i = 0; i < 4; ++i)
                    {
                        originalOk &= CnaDemos::Near(pixels.At(72 + i * 80, 180), stripes[i], 2);
                        fitOk &= CnaDemos::Near(pixels.At(420 + i * 40, 160), stripes[i], 2);
                    }
                    Context().Check(originalOk && fitOk, "the original and fitted images render all four level-zero stripes");
                    Context().Check(CnaDemos::Near(pixels.At(630, 200), Color::Yellow, 2) && CnaDemos::Near(pixels.At(710, 200), Color::Lime, 2),
                                    "the centered crop renders the middle stripes rather than squeezing the full image");
                }
            }
            DemoGame::Draw(time);
        }
    private:
        Texture2D original_, fitted_, cropped_;
        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        bool ready_ = false;
    };
}

CNA_DEMOS_GAME(DdsStreamDemo, {
    .id = "graphics.texture2d.dds_stream",
    .title = "Load DDS streams with authored mipmaps",
    .description = "Loads an original DXT1 stream with independent mip colors, handles short reads and an embedded offset, "
                   "checks malformed mip chains, and renders aspect-preserving and cropped Color conversions.",
    .apis = {"Microsoft::Xna::Framework::Graphics::Texture2D::DDSFromStreamEXT", "Texture2D::GetData",
             "Texture::Format", "Texture::LevelCount", "Texture2D::Width", "Texture2D::Height"},
    .tags = {"texture", "dds", "compression", "mipmap", "stream", "cnaext"},
    .frames = 3,
    .deterministic = true,
    .assets = {"textures/striped_mips.dds", "fonts/pixel_font.cnb"},
})
