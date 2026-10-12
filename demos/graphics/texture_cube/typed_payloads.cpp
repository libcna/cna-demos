// SPDX-License-Identifier: MS-PL
// Packed objects and float vectors transfer their declared storage, not C++ object layouts.
#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/CubeMapFace.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgr565.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/TextureCube.hpp"
#include "System/ArgumentException.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using Microsoft::Xna::Framework::Graphics::PackedVector::Bgr565;

namespace
{
    class TypedCubeDemo final : public CnaDemos::DemoGame
    {
        void LoadContent() override
        {
            std::optional<TextureCube> packed, floating;
            try
            {
                packed.emplace(getGraphicsDeviceProperty(), 2, false, SurfaceFormat::Bgr565);
                floating.emplace(getGraphicsDeviceProperty(), 2, false, SurfaceFormat::Vector4);
            }
            catch (const System::NotSupportedException&)
            { Context().Skip("the live profile/device must create both Bgr565 and Vector4 cube formats"); }
            if (!packed || !floating) return;
            Context().Check(packed->getSizeProperty() == 2 && floating->getSizeProperty() == 2 &&
                            packed->getFormatProperty() == SurfaceFormat::Bgr565 &&
                            floating->getFormatProperty() == SurfaceFormat::Vector4,
                            "the two cubes retain their distinct packed and four-float storage formats");
            Bgr565 guard;
            guard.setPackedValueProperty(0xaaaa);
            const std::array<Bgr565, 6> source{guard, Bgr565(1, 0, 0), Bgr565(0, 1, 0),
                                             Bgr565(0, 0, 1), Bgr565(1, 1, 1), guard};
            packed->SetData(CubeMapFace::PositiveX, source.data(), 1, 4);
            std::array<Bgr565, 7> observed, expected;
            observed.fill(guard); expected = observed;
            std::copy(source.begin() + 1, source.begin() + 5, expected.begin() + 2);
            packed->GetData(CubeMapFace::PositiveX, observed.data(), 2, 4);
            Context().Check(observed == expected,
                            "four packed elements round-trip through a guarded destination window");
            std::array<std::uint8_t, 8> bytes{};
            packed->GetData(CubeMapFace::PositiveX, bytes.data(), 8);
            const std::array<std::uint8_t, 8> expectedBytes{0x00, 0xf8, 0xe0, 0x07, 0x1f, 0x00, 0xff, 0xff};
            Context().Check(bytes == expectedBytes,
                            "packed objects transfer four little-endian 16-bit RGB565 words rather than their C++ object representation");
            Context().Print("Bgr565: C++ object={} bytes; transferred PackedValue=2 bytes per texel.", sizeof(Bgr565));
            std::array<float, 4> wrongWidth;
            wrongWidth.fill(123.25f);
            const auto wrongWidthBefore = wrongWidth;
            bool widthRefused = false;
            // Two float elements have enough total bytes, but each exceeds this format's width.
            try { packed->GetData(CubeMapFace::PositiveX, wrongWidth.data(), 2); }
            catch (const System::ArgumentException&) { widthRefused = true; }
            Context().Check(widthRefused && wrongWidth == wrongWidthBefore,
                            "an oversized element type is refused before any caller float or guard changes");

            std::array<Vector4, 4> hdr{Vector4(2.5f, -.25f, .5f, 1), Vector4(4, 1.5f, -2, .25f),
                                       Vector4(0, .5f, 8, 1), Vector4(1, 1, 1, 1)};
            floating->SetData(CubeMapFace::NegativeZ, hdr.data(), 4);
            std::array<Vector4, 4> returned;
            floating->GetData(CubeMapFace::NegativeZ, returned.data(), 4);
            Context().Check(returned == hdr,
                            "Vector4 cube payloads retain negative and above-one channels without Color quantization");
            std::array<float, 18> scalarView;
            scalarView.fill(99.5f);
            const std::array<float, 18> expectedScalars{99.5f,
                2.5f, -.25f, .5f, 1, 4, 1.5f, -2, .25f, 0, .5f, 8, 1, 1, 1, 1, 1, 99.5f};
            floating->GetData(CubeMapFace::NegativeZ, scalarView.data(), 1, 16);
            Context().Check(scalarView == expectedScalars,
                            "a four-float format exposes sixteen ordered scalar elements for four texels while preserving guards");
            const Rectangle corner(1, 1, 1, 1);
            const Vector4 replacement(16, -4, .125f, .5f);
            floating->SetData(CubeMapFace::NegativeZ, 0, &corner, &replacement, 0, 1);
            hdr[3] = replacement;
            floating->GetData(CubeMapFace::NegativeZ, returned.data(), 4);
            Context().Check(returned == hdr,
                            "a single typed float-vector rectangle edit preserves every other texel and all four channels");
            scalarView.fill(99.5f);
            const auto beforeRefusal = scalarView;
            bool countRefused = false;
            try { floating->GetData(CubeMapFace::NegativeZ, scalarView.data(), 1, 15); }
            catch (const System::ArgumentException&) { countRefused = true; }
            Context().Check(countRefused && scalarView == beforeRefusal,
                            "a fifteen-scalar logical count is refused atomically when four texels require sixteen");
        }
    };
}

CNA_DEMOS_GAME(TypedCubeDemo, {
    .id = "graphics.texture_cube.typed_payloads",
    .title = "Packed and float cube payload widths",
    .description = "Transfers packed RGB565 values and unclamped Vector4 cube data, contrasts element counts with byte widths, "
                   "and verifies typed rectangle edits and refused-output preservation.",
    .apis = {"TextureCube::TextureCube", "TextureCube::SetData", "TextureCube::GetData", "Texture::Format",
             "Bgr565::Bgr565", "Bgr565::PackedValue", "SurfaceFormat::Bgr565", "SurfaceFormat::Vector4"},
    .tags = {"texture", "cube", "packed", "float", "array"},
    .needs = CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Requires Bgr565 and Vector4 cube creation; only constructor capability refusal skips. "
                    "Public typed/byte payload checks do not establish GPU readback, sampling, half-float conversion or every raw C++ value type.",
})
