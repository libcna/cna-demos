// SPDX-License-Identifier: MS-PL
// Audit CNA's public deterministic BC1/BC2/BC3 build-time encoder.
#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "CNA/Content/Pipeline/BlockCompression.hpp"

namespace
{
    namespace Pipeline = CNA::Content::Pipeline;
    using Pipeline::BlockCompressionFormat;

    template<typename Function>
    bool RefusesInvalidArgument(Function&& function)
    {
        try
        {
            function();
        }
        catch (const std::invalid_argument&)
        {
            return true;
        }
        return false;
    }

    std::vector<std::uint8_t> MakeSolid(std::uint32_t width, std::uint32_t height,
                                        const std::array<std::uint8_t, 4>& color)
    {
        std::vector<std::uint8_t> pixels;
        pixels.reserve(static_cast<std::size_t>(width) * height * 4u);
        for (std::size_t texel = 0; texel < static_cast<std::size_t>(width) * height; ++texel)
            pixels.insert(pixels.end(), color.begin(), color.end());
        return pixels;
    }

    std::uint16_t ReadU16(const std::span<const std::uint8_t> bytes, std::size_t offset)
    {
        return static_cast<std::uint16_t>(bytes[offset])
            | static_cast<std::uint16_t>(bytes[offset + 1u]) << 8u;
    }

    int Run()
    {
        bool passed = true;
        const auto check = [&](bool condition, std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };

        const Pipeline::BlockCompressionOptions defaults;
        check(defaults.refinementRounds == 4u && defaults.alphaCutoff == 128u,
              "encoder options default to four refinement rounds and alpha cutoff 128");

        check(static_cast<int>(BlockCompressionFormat::Bc1) == 0
                  && static_cast<int>(BlockCompressionFormat::Bc2) == 1
                  && static_cast<int>(BlockCompressionFormat::Bc3) == 2
                  && Pipeline::BlockCompressionFormatName(BlockCompressionFormat::Bc1) == "Dxt1"
                  && Pipeline::BlockCompressionFormatName(BlockCompressionFormat::Bc2) == "Dxt3"
                  && Pipeline::BlockCompressionFormatName(BlockCompressionFormat::Bc3) == "Dxt5",
              "BC1, BC2 and BC3 have stable identities and XNA-facing DXT names");

        check(Pipeline::BlockCompressedBlockByteCount(BlockCompressionFormat::Bc1) == 8u
                  && Pipeline::BlockCompressedBlockByteCount(BlockCompressionFormat::Bc2) == 16u
                  && Pipeline::BlockCompressedBlockByteCount(BlockCompressionFormat::Bc3) == 16u,
              "one BC1 block occupies 8 bytes while BC2 and BC3 occupy 16");

        check(Pipeline::BlockCompressedByteCount(BlockCompressionFormat::Bc1, 1u, 1u) == 8u
                  && Pipeline::BlockCompressedByteCount(BlockCompressionFormat::Bc1, 4u, 4u) == 8u
                  && Pipeline::BlockCompressedByteCount(BlockCompressionFormat::Bc1, 5u, 5u) == 32u
                  && Pipeline::BlockCompressedByteCount(BlockCompressionFormat::Bc2, 13u, 7u) == 128u
                  && Pipeline::BlockCompressedByteCount(BlockCompressionFormat::Bc3, 16u, 16u) == 256u,
              "image sizing rounds each dimension up to a whole 4x4 block grid");

        const auto opaque = MakeSolid(4u, 4u, {10u, 20u, 30u, 255u});
        auto cutout = opaque;
        cutout[3] = 0u;
        auto faded = opaque;
        faded[3] = 128u;
        check(!Pipeline::ImageHasTransparency(opaque, 4u, 4u)
                  && !Pipeline::ImageHasPartialTransparency(opaque, 4u, 4u)
                  && Pipeline::ImageHasTransparency(cutout, 4u, 4u)
                  && !Pipeline::ImageHasPartialTransparency(cutout, 4u, 4u)
                  && Pipeline::ImageHasTransparency(faded, 4u, 4u)
                  && Pipeline::ImageHasPartialTransparency(faded, 4u, 4u),
              "alpha classification separates opaque, binary-cutout and partial transparency");

        std::vector<std::uint8_t> image(13u * 7u * 4u);
        for (std::uint32_t y = 0; y < 7u; ++y)
        {
            for (std::uint32_t x = 0; x < 13u; ++x)
            {
                const std::size_t at = (static_cast<std::size_t>(y) * 13u + x) * 4u;
                image[at] = static_cast<std::uint8_t>(x * 17u);
                image[at + 1u] = static_cast<std::uint8_t>(y * 31u);
                image[at + 2u] = static_cast<std::uint8_t>((x + y) * 11u);
                image[at + 3u] = static_cast<std::uint8_t>(255u - x * 9u);
            }
        }
        bool sizesAndDeterminism = true;
        for (const BlockCompressionFormat format :
             {BlockCompressionFormat::Bc1, BlockCompressionFormat::Bc2,
              BlockCompressionFormat::Bc3})
        {
            const auto first = Pipeline::EncodeBlockCompressedImage(format, image, 13u, 7u);
            const auto second = Pipeline::EncodeBlockCompressedImage(format, image, 13u, 7u);
            sizesAndDeterminism = sizesAndDeterminism && first == second
                && first.size() == Pipeline::BlockCompressedByteCount(format, 13u, 7u);
        }
        check(sizesAndDeterminism,
              "every format produces its declared byte count deterministically on a partial grid");

        bool edgeReplication = true;
        for (const BlockCompressionFormat format :
             {BlockCompressionFormat::Bc1, BlockCompressionFormat::Bc2,
              BlockCompressionFormat::Bc3})
        {
            const auto one = MakeSolid(1u, 1u, {255u, 0u, 0u, 255u});
            const auto full = MakeSolid(4u, 4u, {255u, 0u, 0u, 255u});
            edgeReplication = edgeReplication
                && Pipeline::EncodeBlockCompressedImage(format, one, 1u, 1u)
                    == Pipeline::EncodeBlockCompressedImage(format, full, 4u, 4u);
        }
        check(edgeReplication,
              "a partial 1x1 block repeats its edge texel exactly like a full 4x4 solid block");

        auto thresholdImage = MakeSolid(4u, 4u, {255u, 0u, 0u, 255u});
        thresholdImage[3] = 127u;
        const auto cutoutBlock = Pipeline::EncodeBlockCompressedImage(
            BlockCompressionFormat::Bc1, thresholdImage, 4u, 4u);
        Pipeline::BlockCompressionOptions lowerCutoff;
        lowerCutoff.alphaCutoff = 127u;
        const auto opaqueBlock = Pipeline::EncodeBlockCompressedImage(
            BlockCompressionFormat::Bc1, thresholdImage, 4u, 4u, lowerCutoff);
        check(ReadU16(cutoutBlock, 0u) <= ReadU16(cutoutBlock, 2u)
                  && (cutoutBlock[4] & 0x03u) == 0x03u
                  && ReadU16(opaqueBlock, 0u) > ReadU16(opaqueBlock, 2u)
                  && (opaqueBlock[4] & 0x03u) != 0x03u && cutoutBlock != opaqueBlock,
              "BC1 alpha cutoff selects transparent mode below the threshold and opaque mode at it");

        const auto transparentBlock = Pipeline::EncodeBlockCompressedImage(
            BlockCompressionFormat::Bc1, MakeSolid(4u, 4u, {90u, 120u, 200u, 0u}), 4u, 4u);
        check(ReadU16(transparentBlock, 0u) == 0u && ReadU16(transparentBlock, 2u) == 0u
                  && transparentBlock[4] == 0xFFu && transparentBlock[5] == 0xFFu
                  && transparentBlock[6] == 0xFFu && transparentBlock[7] == 0xFFu,
              "a fully transparent BC1 block uses transparent index three for every texel");

        auto alphaRamp = MakeSolid(4u, 4u, {128u, 128u, 128u, 0u});
        for (std::size_t texel = 0; texel < 16u; ++texel)
            alphaRamp[texel * 4u + 3u] = static_cast<std::uint8_t>(texel * 17u);
        const auto bc2 = Pipeline::EncodeBlockCompressedImage(
            BlockCompressionFormat::Bc2, alphaRamp, 4u, 4u);
        const std::array<std::uint8_t, 8> expectedNibbles{
            0x10u, 0x32u, 0x54u, 0x76u, 0x98u, 0xBAu, 0xDCu, 0xFEu};
        check(std::equal(expectedNibbles.begin(), expectedNibbles.end(), bc2.begin()),
              "BC2 stores the sixteen exact alpha nibbles in row-major texel order");

        Pipeline::BlockCompressionOptions noRefinement;
        noRefinement.refinementRounds = 0u;
        Pipeline::BlockCompressionOptions extraRefinement;
        extraRefinement.refinementRounds = 8u;
        const auto coarse = Pipeline::EncodeBlockCompressedImage(
            BlockCompressionFormat::Bc3, image, 13u, 7u, noRefinement);
        const auto refined = Pipeline::EncodeBlockCompressedImage(
            BlockCompressionFormat::Bc3, image, 13u, 7u, extraRefinement);
        check(coarse.size() == refined.size()
                  && coarse.size() == Pipeline::BlockCompressedByteCount(
                      BlockCompressionFormat::Bc3, 13u, 7u)
                  && refined == Pipeline::EncodeBlockCompressedImage(
                      BlockCompressionFormat::Bc3, image, 13u, 7u, extraRefinement),
              "caller-selected refinement counts preserve layout and deterministic output");

        check(RefusesInvalidArgument([&] {
                  (void) Pipeline::EncodeBlockCompressedImage(
                      BlockCompressionFormat::Bc1, opaque, 0u, 4u);
              })
                  && RefusesInvalidArgument([&] {
                      (void) Pipeline::EncodeBlockCompressedImage(
                          BlockCompressionFormat::Bc1, opaque, 4u, 0u);
                  })
                  && RefusesInvalidArgument([&] {
                      (void) Pipeline::EncodeBlockCompressedImage(
                          BlockCompressionFormat::Bc1, opaque, 8u, 4u);
                  }),
              "encoder refuses zero dimensions and a pixel buffer with the wrong exact size");

        std::cout << "Encoded BC1, BC2 and BC3 over a 13x7 partial block grid.\n";
        return passed ? 0 : 1;
    }
}

int main()
{
    try
    {
        return Run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Block-compression audit failed: " << error.what() << '\n';
        return 1;
    }
}
