// SPDX-License-Identifier: MS-PL
// Texture3D: a volume of texels (slices), written whole and by sub-box, and read back slice by slice.
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsAdapter.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsProfile.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentationParameters.hpp"
#include "Microsoft/Xna/Framework/Graphics/PresentInterval.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture3D.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/NotSupportedException.hpp"

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
            std::array<Color, 130> white;
            white.fill(Color::Magenta);
            std::fill(white.begin() + 1, white.end() - 1, Color::White);
            const auto originalSource = white;
            volume.SetData(0, 12, 12, 20, 20, 1, 3, white.data(), 1, 128);
            Context().Check(white == originalSource,
                            "a nonzero source window uploads the two-slice box without changing its elements or guards");

            // Read just that box into a caller window, not a whole-volume staging array.
            std::array<Color, 132> boxed, expectedBox;
            boxed.fill(Color::Magenta); expectedBox = boxed;
            std::fill(expectedBox.begin() + 2, expectedBox.end() - 2, Color::White);
            volume.GetData(0, 12, 12, 20, 20, 1, 3, boxed.data(), 2, 128);
            Context().Check(boxed == expectedBox,
                            "a boxed read fills exactly the selected destination window and preserves both guard pairs");
            boxed.fill(Color::Gold);
            const auto beforeShortRead = boxed;
            bool shortReadRefused = false;
            // Physical storage includes all 128 elements plus guards; only the logical count is short.
            try { volume.GetData(0, 12, 12, 20, 20, 1, 3, boxed.data(), 2, 127); }
            catch (const System::ArgumentException&) { shortReadRefused = true; }
            Context().Check(shortReadRefused && boxed == beforeShortRead,
                            "a short logical boxed-read count is refused before any output or guard changes");

            // A non-cubic chain reaches depth one before its width reaches one.
            Texture3D mipVolume(device, 8, 4, 2, true, SurfaceFormat::Color);
            const std::array<int, 4> widths{8, 4, 2, 1}, heights{4, 2, 1, 1}, depths{2, 1, 1, 1};
            Context().Check(mipVolume.getWidthProperty() == 8 && mipVolume.getHeightProperty() == 4 &&
                            mipVolume.getDepthProperty() == 2 && mipVolume.getLevelCountProperty() == 4 &&
                            mipVolume.getFormatProperty() == SurfaceFormat::Color,
                            "the non-cubic Color volume allocates four levels while each axis bottoms out at one");
            std::array<std::vector<Color>, 4> levels;
            for (int level = 0; level < 4; ++level)
            {
                auto& data = levels[level];
                for (int z = 0; z < depths[level]; ++z)
                    for (int y = 0; y < heights[level]; ++y)
                        for (int x = 0; x < widths[level]; ++x)
                            data.emplace_back(30 + level * 50, 10 + x * 20, 20 + y * 40 + z * 10);
                mipVolume.SetData(level, 0, 0, widths[level], heights[level], 0, depths[level],
                                  data.data(), 0, static_cast<int>(data.size()));
            }
            const std::array<Color, 2> badDepthData{Color::Orange, Color::Gold};
            bool depthRefused = false;
            // Level 1 is 4x2x1: two physically allocated texels cannot make its depth two.
            try { mipVolume.SetData(1, 0, 0, 1, 1, 0, 2, badDepthData.data(), 0, 2); }
            catch (const System::ArgumentException&) { depthRefused = true; }
            Context().Check(depthRefused, "a box reaching past the mip's own Z boundary is refused");
            bool allLevelsMatch = true;
            for (int level = 0; level < 4; ++level)
            {
                std::vector<Color> data(levels[level].size());
                mipVolume.GetData(level, 0, 0, widths[level], heights[level], 0, depths[level],
                                  data.data(), 0, static_cast<int>(data.size()));
                allLevelsMatch = allLevelsMatch && data == levels[level];
            }
            Context().Check(allLevelsMatch,
                            "all four authored non-cubic mip payloads survive the refused Z-overflow upload");
            std::array<Color, 3> invalidLevelOutput{Color::Gold, Color::Gold, Color::Gold};
            const auto beforeInvalidLevel = invalidLevelOutput;
            bool levelRefused = false;
            try { mipVolume.GetData(4, 0, 0, 1, 1, 0, 1, invalidLevelOutput.data(), 1, 1); }
            catch (const System::InvalidOperationException&) { levelRefused = true; }
            Context().Check(levelRefused && invalidLevelOutput == beforeInvalidLevel,
                            "a read beyond LevelCount refuses the nonexistent mip without touching caller storage");

            std::vector<Color> back(texels.size());
            volume.GetData(back.data(), static_cast<int>(back.size()));
            const auto at = [&](int x, int y, int z) { return back[(z * Size + y) * Size + x]; };
            Context().Check(at(3, 5, 0) == texels[(0 * Size + 5) * Size + 3], "texels outside the box are unchanged");
            Context().Check(at(15, 15, 1) == Color::White && at(15, 15, 2) == Color::White &&
                                at(15, 15, 0) != Color::White && at(15, 15, 3) != Color::White,
                            "the box touched slices 1 and 2 only");

            CheckVectorField(device);
            CheckRawTransferContracts(device);
            CheckProfileBoundaries(device);
            CheckCrossDeviceMove(device);

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
        void CheckProfileBoundaries(GraphicsDevice& hiDef)
        {
            bool compressedRefused = false, extentRefused = false;
            try { Texture3D compressed(hiDef, 4, 4, 4, false, SurfaceFormat::Dxt1); }
            catch (const System::NotSupportedException&) { compressedRefused = true; }
            try { Texture3D oversized(hiDef, 257, 1, 1, false, SurfaceFormat::Color); }
            catch (const System::NotSupportedException&) { extentRefused = true; }
            Context().Check(compressedRefused,
                            "HiDef refuses a block-compressed format that is not valid for volume storage");
            Context().Check(extentRefused,
                            "HiDef refuses a volume whose width exceeds the 256-texel profile ceiling");

            PresentationParameters parameters;
            parameters.setBackBufferWidthProperty(32);
            parameters.setBackBufferHeightProperty(32);
            parameters.setDepthStencilFormatProperty(DepthFormat::None);
            parameters.setPresentationIntervalProperty(PresentInterval::Immediate);
            GraphicsDevice reach(GraphicsAdapter::getDefaultAdapterProperty(),
                                 GraphicsProfile::Reach, parameters);
            bool reachRefused = false;
            try { Texture3D unavailable(reach, 1, 1, 1, false, SurfaceFormat::Color); }
            catch (const System::NotSupportedException&) { reachRefused = true; }
            Context().Check(reachRefused,
                            "the Reach profile refuses volume textures even for a one-texel Color resource");
            reach.Dispose();
        }

        void CheckCrossDeviceMove(GraphicsDevice& primary)
        {
            PresentationParameters parameters;
            parameters.setBackBufferWidthProperty(32);
            parameters.setBackBufferHeightProperty(32);
            parameters.setDepthStencilFormatProperty(DepthFormat::None);
            parameters.setPresentationIntervalProperty(PresentInterval::Immediate);
            GraphicsDevice secondary(GraphicsAdapter::getDefaultAdapterProperty(),
                                     GraphicsProfile::HiDef, parameters);

            Texture3D destination(primary, 1, 1, 1, false, SurfaceFormat::Color);
            const Color placeholder = Color::Green;
            destination.SetData(&placeholder, 1);
            primary.getTexturesProperty()(3, &destination);

            Texture3D source(secondary, 2, 1, 2, false, SurfaceFormat::Color);
            const std::array<Color, 4> payload{
                Color::Red, Color::Green, Color::Blue, Color::Yellow};
            source.SetData(payload.data(), 4);
            secondary.getTexturesProperty()(3, &source);

            destination = std::move(source);
            std::array<Color, 4> observed;
            observed.fill(Color::Black);
            bool transferredBindingRefused = false;
            try { destination.SetData(payload.data(), 4); }
            catch (const System::InvalidOperationException&) { transferredBindingRefused = true; }
            secondary.getTexturesProperty()(3, nullptr);
            destination.GetData(observed.data(), 4);
            Context().Check(destination.getGraphicsDeviceProperty() == &secondary &&
                                destination.getWidthProperty() == 2 && destination.getHeightProperty() == 1 &&
                                destination.getDepthProperty() == 2 && observed == payload,
                            "cross-device move assignment replaces an occupied volume and transfers ownership and payload");
            Context().Check(transferredBindingRefused,
                            "cross-device move assignment transfers the source device's active texture binding");

            // Assignment detached destination from the primary slot; clearing it is harmless and
            // keeps this workflow explicit before both resources and the secondary device leave scope.
            primary.getTexturesProperty()(3, nullptr);
            destination.Dispose();
            secondary.Dispose();
        }

        void CheckRawTransferContracts(GraphicsDevice& device)
        {
            struct RawWord
            {
                std::uint16_t low;
                std::uint16_t high;
                bool operator==(const RawWord&) const = default;
            };
            static_assert(sizeof(RawWord) == 4 && std::is_trivially_copyable_v<RawWord>);

            // Texture3D's generic overload deliberately accepts arbitrary trivially-copyable
            // elements whose width divides the voxel format. Use non-Color words so this check
            // cannot accidentally pass through Color's packed-value specialization.
            Texture3D rawVolume(device, 2, 1, 2, false, SurfaceFormat::Color);
            const RawWord guard{0xBEEFu, 0xCAFEu};
            const std::array<RawWord, 6> source{{
                guard, {0x1122u, 0x3344u}, {0x5566u, 0x7788u},
                {0x99AAu, 0xBBCCu}, {0xDDEEu, 0x0F10u}, guard}};
            rawVolume.SetData(source.data(), 1, 4);
            std::array<RawWord, 7> observed;
            observed.fill(guard);
            rawVolume.GetData(observed.data(), 2, 4);
            Context().Check(observed[0] == guard && observed[1] == guard && observed[6] == guard &&
                                std::equal(source.begin() + 1, source.end() - 1, observed.begin() + 2),
                            "four-byte raw values round-trip in X-fastest, slice-major order inside guarded caller windows");

            const std::uint64_t tooWide = UINT64_C(0x0123456789ABCDEF);
            std::array<std::uint64_t, 2> wideOutput{tooWide, tooWide};
            const auto originalWideOutput = wideOutput;
            bool wideWriteRefused = false, wideReadRefused = false;
            try { rawVolume.SetData(&tooWide, 1); }
            catch (const System::ArgumentException&) { wideWriteRefused = true; }
            try { rawVolume.GetData(wideOutput.data(), 1); }
            catch (const System::ArgumentException&) { wideReadRefused = true; }
            observed.fill(guard);
            rawVolume.GetData(observed.data(), 2, 4);
            Context().Check(wideWriteRefused && wideReadRefused && wideOutput == originalWideOutput &&
                                std::equal(source.begin() + 1, source.end() - 1, observed.begin() + 2),
                            "an element wider than the four-byte format is refused atomically in both transfer directions");

            const std::array<RawWord, 4> replacement{{
                {1u, 2u}, {3u, 4u}, {5u, 6u}, {7u, 8u}}};
            device.getTexturesProperty()(2, &rawVolume);
            bool boundWriteRefused = false;
            try { rawVolume.SetData(replacement.data(), 4); }
            catch (const System::InvalidOperationException&) { boundWriteRefused = true; }
            Context().Check(boundWriteRefused,
                            "a bound volume rejects upload before changing its existing payload");

            Texture3D movedBound(std::move(rawVolume));
            bool movedBoundWriteRefused = false;
            try { movedBound.SetData(replacement.data(), 4); }
            catch (const System::InvalidOperationException&) { movedBoundWriteRefused = true; }
            device.getTexturesProperty()(2, nullptr); // the binding followed movedBound
            observed.fill(guard);
            movedBound.GetData(observed.data(), 2, 4);
            Context().Check(movedBoundWriteRefused && movedBound.getWidthProperty() == 2 &&
                                movedBound.getHeightProperty() == 1 && movedBound.getDepthProperty() == 2 &&
                                std::equal(source.begin() + 1, source.end() - 1, observed.begin() + 2),
                            "an active binding follows move construction and retains its payload after explicit unbinding");
        }

        void CheckVectorField(GraphicsDevice& device)
        {
            // A 4x3x2 displacement field uses signed XYZ and a fourth scalar channel, not Color.
            // Keep the existing Color atlas: this section demonstrates transfers, not GPU sampling.
            std::optional<Texture3D> fieldVolume;
            try { fieldVolume.emplace(device, 4, 3, 2, false, SurfaceFormat::Vector4); }
            catch (const System::NotSupportedException&)
            { Context().Skip("the live device/profile must create a Vector4 volume for the displacement field"); }
            if (!fieldVolume) return;
            const Vector4 guard(99, 98, 97, 96);
            std::array<Vector4, 26> source;
            source.fill(guard);
            for (int z = 0; z < 2; ++z)
                for (int y = 0; y < 3; ++y)
                    for (int x = 0; x < 4; ++x)
                    {
                        const int index = (z * 3 + y) * 4 + x;
                        source[index + 1] = Vector4(static_cast<float>(x) - .5f,
                            static_cast<float>(y) * 2 - .25f, static_cast<float>(z) * 4 + .5f,
                            static_cast<float>(index + 1) * .125f);
                    }
            const auto originalSource = source;
            fieldVolume->SetData(source.data(), 1, 24);
            std::array<Vector4, 28> observed, expected;
            observed.fill(guard); expected = observed;
            std::copy(source.begin() + 1, source.end() - 1, expected.begin() + 2);
            fieldVolume->GetData(observed.data(), 2, 24);
            Context().Check(fieldVolume->getFormatProperty() == SurfaceFormat::Vector4 &&
                            source == originalSource && observed == expected,
                            "a guarded Vector4 volume preserves every signed displacement and above-one channel without Color quantization");

            std::array<float, 100> scalarView, expectedScalars;
            scalarView.fill(-123.5f); expectedScalars = scalarView;
            for (int index = 0; index < 24; ++index)
            {
                const Vector4& value = source[index + 1];
                expectedScalars[index * 4 + 2] = value.X;
                expectedScalars[index * 4 + 3] = value.Y;
                expectedScalars[index * 4 + 4] = value.Z;
                expectedScalars[index * 4 + 5] = value.W;
            }
            fieldVolume->GetData(scalarView.data(), 2, 96);
            Context().Check(scalarView == expectedScalars,
                            "twenty-four vector voxels expose ninety-six ordered float elements with both guard pairs intact");

            // Edit x=1..2, y=1 in both slices. The compact payload has no full-volume row pitch.
            const std::array<Vector4, 6> patch{guard, Vector4(-8, 2, .25f, 1), Vector4(4, -3, 8, .5f),
                                              Vector4(16, .125f, -4, 2), Vector4(.5f, 32, -2, .25f), guard};
            fieldVolume->SetData(0, 1, 1, 3, 2, 0, 2, patch.data(), 1, 4);
            expected[2 + 5] = patch[1]; expected[2 + 6] = patch[2];
            expected[2 + 17] = patch[3]; expected[2 + 18] = patch[4];
            fieldVolume->GetData(observed.data(), 2, 24);
            Context().Check(observed == expected,
                            "a compact two-slice typed box changes exactly four field voxels and preserves every neighbor and guard");
            std::array<float, 18> box;
            box.fill(-123.5f);
            const std::array<float, 18> expectedBox{-123.5f, -8,2,.25f,1, 4,-3,8,.5f,
                                                  16,.125f,-4,2, .5f,32,-2,.25f, -123.5f};
            fieldVolume->GetData(0, 1, 1, 3, 2, 0, 2, box.data(), 1, 16);
            Context().Check(box == expectedBox,
                            "a boxed scalar read packs the two slice rows consecutively while preserving caller guards");
            box.fill(-123.5f);
            const auto beforeRefusal = box;
            bool readRefused = false;
            // Physical storage fits all sixteen scalars plus guards; only the declared count is short.
            try { fieldVolume->GetData(0, 1, 1, 3, 2, 0, 2, box.data(), 1, 15); }
            catch (const System::ArgumentException&) { readRefused = true; }
            Context().Check(readRefused && box == beforeRefusal,
                            "a short scalar box read is refused before changing any output or guard");
            bool writeRefused = false;
            try { fieldVolume->SetData(0, 1, 1, 3, 2, 0, 2, box.data(), 1, 15); }
            catch (const System::ArgumentException&) { writeRefused = true; }
            fieldVolume->GetData(observed.data(), 2, 24);
            Context().Check(writeRefused && observed == expected && box == beforeRefusal,
                            "a short scalar box upload preserves the complete existing field and caller storage");
        }

        std::vector<Texture2D> slices_;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(VolumeDataDemo, {
    .id = "graphics.texture3d.volume_data",
    .title = "3D textures",
    .description = "Creates a 32x32x4 Texture3D, fills it slice by slice with SetData, overwrites a sub-box spanning "
                   "two slices, reads guarded boxes and shows the base slices as 2D textures. A non-cubic authored mip chain "
                   "checks per-level Z bounds. A signed Vector4 displacement field contrasts vector/scalar counts, compact "
                   "two-slice boxes, guarded windows and refused-transfer preservation. Raw four-byte values additionally "
                   "show generic caller windows, element-width rejection and active-binding moves within and across devices. "
                   "Constructor checks establish volume format, extent and Reach-profile boundaries.",
    .apis = {"Texture3D::Texture3D", "Texture3D::SetData", "Texture3D::GetData", "Texture3D::Width",
             "Texture3D::Height", "Texture3D::Depth", "Texture::LevelCount", "Texture::Format", "SurfaceFormat::Vector4",
             "GraphicsDevice::GraphicsDevice", "GraphicsDevice::Textures", "GraphicsResource::GraphicsDevice",
             "GraphicsResource::Dispose"},
    .tags = {"texture", "volume", "3d-texture", "lookup-table"},
    .needs = CnaDemos::Need::Texture3D,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Requires Color and Vector4 volume creation; only constructor capability refusal skips the typed fixture. "
                    "Public transfer checks and the Color readback atlas do not establish direct GPU readback or non-Color "
                    "sampling. Packed and half-float format conversion is already covered by the Texture2D format workflows; "
                    "device loss and renderer-specific volume limits remain outside this scene.",
})
