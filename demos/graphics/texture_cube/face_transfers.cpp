// SPDX-License-Identifier: MS-PL
// Cube faces are separate images with separate mip levels; array offsets are caller coordinates.
#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/PixelChecks.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
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
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;

namespace
{
    class CubeTransfersDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& device = getGraphicsDeviceProperty(); auto& ctx = Context();
            batch_ = std::make_unique<SpriteBatch>(device);
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
            TextureCube cube(device, 4, true, SurfaceFormat::Color);
            ctx.Check(cube.getSizeProperty() == 4 && cube.getLevelCountProperty() == 3 &&
                      cube.getFormatProperty() == SurfaceFormat::Color,
                      "a four-texel Color cube exposes 4x4, 2x2 and 1x1 levels");
            const std::array<Color, 6> colors{Color::Red, Color::Cyan, Color::Blue,
                                             Color::Yellow, Color::Lime, Color::Magenta};
            std::array<std::array<Color, 16>, 6> expected;
            for (int face = 0; face < 6; ++face)
            {
                expected[face].fill(colors[face]);
                cube.SetData(static_cast<CubeMapFace>(face), expected[face].data(), 16);
            }
            // Update only the central 2x2 of +Y from an allocated source window.
            const Rectangle region(1, 1, 2, 2);
            const std::array<Color, 6> source{Color::Orange, Color::Red, Color::Lime,
                                             Color::White, Color::Black, Color::Orange};
            cube.SetData(CubeMapFace::PositiveY, 0, &region, source.data(), 1, 4);
            expected[2][5] = Color::Red; expected[2][6] = Color::Lime;
            expected[2][9] = Color::White; expected[2][10] = Color::Black;
            std::array<Color, 8> window; window.fill(Color::Orange);
            cube.GetData(CubeMapFace::PositiveY, 0, &region, window.data(), 2, 4);
            ctx.Check(std::equal(source.begin() + 1, source.begin() + 5, window.begin() + 2) &&
                      window[0] == Color::Orange && window[1] == Color::Orange &&
                      window[6] == Color::Orange && window[7] == Color::Orange,
                      "a rectangular copy honors independent source and destination offsets and leaves output guards intact");

            // Explicitly authored mips have their own colors, not a downsample of level zero.
            const std::array<Color, 4> middle{Color::Cyan, Color::Cyan, Color::Cyan, Color::Cyan};
            const Color last = Color::Magenta;
            cube.SetData(CubeMapFace::PositiveY, 1, nullptr, middle.data(), 0, 4);
            cube.SetData(CubeMapFace::PositiveY, 2, nullptr, &last, 0, 1);
            std::array<Color, 4> readMiddle; Color readLast;
            cube.GetData(CubeMapFace::PositiveY, 1, nullptr, readMiddle.data(), 0, 4);
            cube.GetData(CubeMapFace::PositiveY, 2, nullptr, &readLast, 0, 1);
            ctx.Check(readMiddle == middle && readLast == last,
                      "null rectangles address the selected 2x2 and 1x1 mip dimensions with independent authored content");

            std::array<Color, 4> rejected; rejected.fill(Color::Orange); // would visibly change the current patch
            bool refused = false;
            try { cube.SetData(CubeMapFace::PositiveY, 0, &region, rejected.data(), 0, 3); }
            catch (const System::ArgumentException&) { refused = true; }
            ctx.Check(refused, "a transfer count smaller than the complete selected rectangle is refused");
            window.fill(Color::Orange);
            const auto saved = window;
            const Rectangle outside(3, 3, 2, 2);
            refused = false;
            try { cube.GetData(CubeMapFace::PositiveY, 0, &outside, window.data(), 2, 4); }
            catch (const System::ArgumentException&) { refused = true; }
            ctx.Check(refused && window == saved, "a refused out-of-bounds read leaves the entire caller buffer untouched");

            // Exercise the second device before allocating the 2D atlas resources. This mirrors
            // the Texture3D lifecycle workflow and makes the later pixels test whether the primary
            // device can create resources and draw a frame after secondary-device teardown.
            CheckLifecycle(device);

            bool allFacesMatch = true;
            for (int face = 0; face < 6; ++face)
            {
                std::array<Color, 16> pixels;
                cube.GetData(static_cast<CubeMapFace>(face), pixels.data(), 16);
                allFacesMatch = allFacesMatch && pixels == expected[face];
                faces_.emplace_back(device, 4, 4);
                faces_.back().SetData(pixels.data(), 16);
            }
            ctx.Check(allFacesMatch, "mip edits and refused writes preserve the patched base face, its borders and all five neighboring faces");
            mips_.emplace_back(device, 2, 2); mips_.back().SetData(readMiddle.data(), 4);
            mips_.emplace_back(device, 1, 1); mips_.back().SetData(&readLast, 1);
            ctx.Print("Face atlas: +X, -X, +Y (patched), -Y, +Z, -Z; lower row: +Y mips 1 and 2");
        }

        void Draw(const GameTime& time) override
        {
            auto& device = getGraphicsDeviceProperty(); device.Clear(Color(24, 28, 36));
            batch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            batch_->DrawString(*font_, "TextureCube: read-back faces and authored mips", Vector2(32, 32), Color::White);
            const std::array<const char*, 6> names{"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
            for (int face = 0; face < 6; ++face)
            {
                const int x = 40 + 120 * face;
                batch_->DrawString(*font_, names[face], Vector2(static_cast<float>(x), 88), Color::White);
                batch_->Draw(faces_[face], Rectangle(x, 120, 96, 96), Color::White);
            }
            batch_->DrawString(*font_, "+Y mip 1       +Y mip 2", Vector2(280, 268), Color::White);
            batch_->Draw(mips_[0], Rectangle(280, 300, 96, 96), Color::White);
            batch_->Draw(mips_[1], Rectangle(400, 300, 96, 96), Color::White);
            batch_->End();
            if (FramesDrawn() == 0)
            {
                const CnaDemos::BackBufferPixels pixels(device);
                Context().Check(CnaDemos::Near(pixels.At(316, 156), Color::Red, 0) &&
                                CnaDemos::Near(pixels.At(340, 156), Color::Lime, 0) &&
                                CnaDemos::Near(pixels.At(316, 180), Color::White, 0) &&
                                CnaDemos::Near(pixels.At(340, 180), Color::Black, 0) &&
                                CnaDemos::Near(pixels.At(292, 132), Color::Blue, 0),
                                "the primary device draws the read-back +Y atlas after secondary-device teardown");
                Context().Check(CnaDemos::Near(pixels.At(328, 348), Color::Cyan, 0) &&
                                CnaDemos::Near(pixels.At(448, 348), Color::Magenta, 0),
                                "the primary device draws authored mip tiles after secondary-device teardown");
            }
            DemoGame::Draw(time);
        }

    private:
        void CheckLifecycle(GraphicsDevice& primary)
        {
            PresentationParameters parameters;
            parameters.setBackBufferWidthProperty(32);
            parameters.setBackBufferHeightProperty(32);
            parameters.setDepthStencilFormatProperty(DepthFormat::None);
            parameters.setPresentationIntervalProperty(PresentInterval::Immediate);
            GraphicsDevice secondary(GraphicsAdapter::getDefaultAdapterProperty(),
                                     GraphicsProfile::HiDef, parameters);

            TextureCube destination(primary, 1, false, SurfaceFormat::Color);
            const Color placeholder = Color::Green;
            destination.SetData(CubeMapFace::PositiveX, &placeholder, 1);
            primary.getTexturesProperty()(3, &destination);

            TextureCube source(secondary, 2, true, SurfaceFormat::Color);
            const std::array<Color, 4> payload{
                Color::Red, Color::Green, Color::Blue, Color::Yellow};
            source.SetData(CubeMapFace::NegativeZ, payload.data(), 4);
            destination = source;
            std::array<Color, 4> observed;
            observed.fill(Color::Black);
            destination.GetData(CubeMapFace::NegativeZ, observed.data(), 4);
            Context().Check(destination.getGraphicsDeviceProperty() == &secondary &&
                                destination.getSizeProperty() == 2 &&
                                destination.getLevelCountProperty() == 2 && observed == payload &&
                                primary.getTexturesProperty()[3] == nullptr,
                            "cross-device copy assignment replaces the target, changes its owner and detaches its old binding");

            const std::array<Color, 4> sharedEdit{
                Color::Orange, Color::Cyan, Color::Magenta, Color::White};
            source.SetData(CubeMapFace::NegativeZ, sharedEdit.data(), 4);
            destination.GetData(CubeMapFace::NegativeZ, observed.data(), 4);
            Context().Check(observed == sharedEdit,
                            "a cross-device-assigned cube shares later face edits with its source wrapper");

            secondary.getTexturesProperty()(3, &source);
            TextureCube moved(std::move(source));
            bool boundWriteRefused = false;
            try { moved.SetData(CubeMapFace::NegativeZ, payload.data(), 4); }
            catch (const System::InvalidOperationException&) { boundWriteRefused = true; }
            const bool bindingFollowed = secondary.getTexturesProperty()[3] == &moved;
            secondary.getTexturesProperty()(3, nullptr);
            moved.GetData(CubeMapFace::NegativeZ, observed.data(), 4);
            Context().Check(bindingFollowed && boundWriteRefused && observed == sharedEdit,
                            "move construction transfers an active cube binding and preserves its shared payload");

            secondary.Dispose();
            observed.fill(Color::Gold);
            const auto beforeDisposedRead = observed;
            bool disposedReadRefused = false;
            try { destination.GetData(CubeMapFace::NegativeZ, observed.data(), 4); }
            catch (const System::ObjectDisposedException&) { disposedReadRefused = true; }
            Context().Check(destination.getIsDisposedProperty() && moved.getIsDisposedProperty() &&
                                disposedReadRefused && observed == beforeDisposedRead,
                            "device disposal invalidates every registered cube wrapper before a read can change output");
        }

        std::unique_ptr<SpriteBatch> batch_;
        std::optional<SpriteFont> font_;
        std::vector<Texture2D> faces_, mips_;
    };
}

CNA_DEMOS_GAME(CubeTransfersDemo, {
    .id = "graphics.texture_cube.face_transfers",
    .title = "Cube face windows and mip transfers",
    .description = "Uploads independent cube faces, edits a rectangle through an array window, authors non-base mips, "
                   "checks refused transfers and cube wrapper lifecycle, then displays read-back faces and levels as an atlas.",
    .apis = {"TextureCube::TextureCube", "TextureCube::Size", "TextureCube::SetData", "TextureCube::GetData",
             "Texture::Format", "Texture::LevelCount", "GraphicsResource::GraphicsDevice",
             "GraphicsResource::IsDisposed", "GraphicsDevice::Textures", "GraphicsDevice::Dispose",
             "CubeMapFace::PositiveX", "CubeMapFace::NegativeX", "CubeMapFace::PositiveY",
             "CubeMapFace::NegativeY", "CubeMapFace::PositiveZ", "CubeMapFace::NegativeZ"},
    .tags = {"texture", "cubemap", "mipmap", "readback", "array", "ownership", "lifetime"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 3,
    .deterministic = true,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "CNA-DEMOS-ISSUE-086: destroying a secondary EasyGL device leaves the primary device's later frame black. Color transfers and public read-back do not establish direct GPU readback.",
    .knownIssue = "CNA-DEMOS-ISSUE-086",
    .knownIssueRenderers = {"OPENGLES3"},
})
