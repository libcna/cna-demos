// SPDX-License-Identifier: MS-PL
// Inspect the complete public SoundEffect CNB codec contract without opening an audio device.
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbSoundEffectCodec.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"

namespace Cnb = CNA::Content::Cnb;
using Microsoft::Xna::Framework::Content::ContentLoadException;

namespace
{
    bool RefusesContent(const std::function<void()>& action)
    {
        try
        {
            action();
        }
        catch (const ContentLoadException&)
        {
            return true;
        }
        return false;
    }

    std::vector<std::uint8_t> BuildRawSound(
        const Cnb::CnbSoundEffectData& sound,
        const std::uint32_t schemaVersion,
        const std::uint32_t headerFlags = 0u,
        const std::uint32_t assetType = Cnb::CnbAssetTypeId::SoundEffect,
        const std::size_t headerCount = 1u,
        const std::size_t dataCount = 1u)
    {
        Cnb::CnbByteWriter header;
        header.WriteU32(static_cast<std::uint32_t>(sound.format));
        header.WriteU32(sound.sampleRate);
        header.WriteU32(sound.channels);
        header.WriteU32(sound.frameCount);
        header.WriteU32(sound.loopStart);
        header.WriteU32(sound.loopLength);
        header.WriteU32(headerFlags);
        const std::vector<std::uint8_t> headerBytes = header.Take();

        Cnb::CnbWriter writer(assetType, schemaVersion);
        writer.SetMetadata("Microsoft.Xna.Framework.Audio.SoundEffect", "sounds/raw");
        for (std::size_t index = 0; index < headerCount; ++index)
        {
            writer.AddChunk(Cnb::CnbSoundEffectChunk::Header, headerBytes,
                            Cnb::CnbChunkFlags::Mandatory, 4u);
        }
        for (std::size_t index = 0; index < dataCount; ++index)
        {
            writer.AddChunk(Cnb::CnbSoundEffectChunk::Data, sound.samples,
                            Cnb::CnbChunkFlags::Mandatory, 16u);
        }
        return writer.Build();
    }

    void CnbSoundEffectCodec(CnaDemos::DemoContext& ctx)
    {
        const Cnb::CnbSoundEffectData defaults;
        ctx.Check(defaults.format == Cnb::CnbAudioFormat::Pcm16 &&
                      defaults.sampleRate == 44100u && defaults.channels == 1u &&
                      defaults.frameCount == 0u && defaults.loopStart == 0u &&
                      defaults.loopLength == 0u && defaults.samples.empty(),
                  "CnbSoundEffectData defaults to valid empty mono PCM16");

        ctx.Check(Cnb::CnbAudioFormatMax == 5u && Cnb::CnbSoundEffectSchemaVersion == 2u &&
                      Cnb::CnbSoundEffectHeaderStride == 28u &&
                      Cnb::CnbMaxAudioSampleRate == 384000u &&
                      Cnb::CnbSoundEffectChunk::Header == Cnb::MakeChunkId('A', 'U', 'D', 'H') &&
                      Cnb::CnbSoundEffectChunk::Data == Cnb::MakeChunkId('A', 'U', 'D', 'D'),
                  "the public constants freeze the SoundEffect schema and its two chunks");

        struct FormatExpectation
        {
            Cnb::CnbAudioFormat format;
            std::uint32_t schema;
            std::uint32_t monoBytes;
            const char* name;
        };
        const std::array formats{
            FormatExpectation{Cnb::CnbAudioFormat::Unknown, 0u, 0u, "Unknown"},
            FormatExpectation{Cnb::CnbAudioFormat::Pcm16, 1u, 2u, "Pcm16"},
            FormatExpectation{Cnb::CnbAudioFormat::Pcm8, 2u, 1u, "Pcm8"},
            FormatExpectation{Cnb::CnbAudioFormat::PcmFloat32, 0u, 4u, "PcmFloat32"},
            FormatExpectation{Cnb::CnbAudioFormat::Adpcm, 0u, 0u, "Adpcm"},
            FormatExpectation{Cnb::CnbAudioFormat::Vorbis, 0u, 0u, "Vorbis"},
        };
        bool completeFormatTable = true;
        for (const FormatExpectation& expected : formats)
        {
            completeFormatTable = completeFormatTable &&
                Cnb::CnbSoundEffectSchemaVersionForFormat(expected.format) == expected.schema &&
                Cnb::CnbAudioFrameBytes(expected.format, 1u) == expected.monoBytes &&
                Cnb::CnbAudioFrameBytes(expected.format, 2u) == expected.monoBytes * 2u &&
                Cnb::CnbAudioFormatToString(expected.format) == expected.name;
        }
        const auto unknown = static_cast<Cnb::CnbAudioFormat>(99u);
        ctx.Check(completeFormatTable &&
                      Cnb::CnbSoundEffectSchemaVersionForFormat(unknown) == 0u &&
                      Cnb::CnbAudioFrameBytes(unknown, 2u) == 0u &&
                      Cnb::CnbAudioFormatToString(unknown) == "unknown audio format 99",
                  "every assigned and unknown format has an exact schema, frame width, and name");

        Cnb::CnbSoundEffectData pcm16;
        pcm16.format = Cnb::CnbAudioFormat::Pcm16;
        pcm16.sampleRate = 8000u;
        pcm16.channels = 2u;
        pcm16.frameCount = 2u;
        pcm16.loopStart = 1u;
        pcm16.loopLength = 1u;
        pcm16.samples = {0x00u, 0x80u, 0xFFu, 0xFFu, 0x00u, 0x00u, 0xFFu, 0x7Fu};
        ctx.Check(Cnb::CnbSoundEffectSamplesAsPcm16(pcm16) == pcm16.samples,
                  "PCM16 runtime adaptation preserves every source byte");

        Cnb::CnbSoundEffectData pcm8;
        pcm8.format = Cnb::CnbAudioFormat::Pcm8;
        pcm8.sampleRate = 11025u;
        pcm8.channels = 1u;
        pcm8.frameCount = 4u;
        pcm8.loopStart = 0u;
        pcm8.loopLength = 4u;
        pcm8.samples = {0u, 64u, 128u, 255u};
        ctx.Check(Cnb::CnbSoundEffectSamplesAsPcm16(pcm8) ==
                      std::vector<std::uint8_t>{0x00u, 0x80u, 0x00u, 0xC0u,
                                                0x00u, 0x00u, 0x00u, 0x7Fu},
                  "PCM8 widens exactly from unsigned bias to signed little-endian PCM16");

        Cnb::CnbSoundEffectData reserved = pcm16;
        reserved.format = Cnb::CnbAudioFormat::Vorbis;
        ctx.Check(RefusesContent([&] { (void)Cnb::CnbSoundEffectSamplesAsPcm16(reserved); }),
                  "runtime adaptation refuses a reserved format without a codec");

        const auto encoded16 = Cnb::EncodeSoundEffectToCnb(pcm16, "sounds/stereo");
        const auto encoded16Again = Cnb::EncodeSoundEffectToCnb(pcm16, "sounds/stereo");
        const Cnb::CnbDocument document16 = Cnb::CnbDocument::Parse(encoded16, "pcm16.cnb");
        const Cnb::CnbSoundEffectData decoded16 = Cnb::DecodeSoundEffectFromCnb(document16);
        const auto header16 = document16.RequireSingle(Cnb::CnbSoundEffectChunk::Header);
        const auto data16 = document16.RequireSingle(Cnb::CnbSoundEffectChunk::Data);
        ctx.Check(encoded16 == encoded16Again &&
                      document16.AssetTypeId() == Cnb::CnbAssetTypeId::SoundEffect &&
                      document16.AssetSchemaVersion() == Cnb::CnbSoundEffectSchemaVersion &&
                      document16.Metadata().contentName == "sounds/stereo" &&
                      document16.ChunkData(header16).size() == Cnb::CnbSoundEffectHeaderStride &&
                      std::vector<std::uint8_t>(document16.ChunkData(data16).begin(),
                                                document16.ChunkData(data16).end()) == pcm16.samples &&
                      decoded16.format == pcm16.format && decoded16.sampleRate == pcm16.sampleRate &&
                      decoded16.channels == pcm16.channels &&
                      decoded16.frameCount == pcm16.frameCount &&
                      decoded16.loopStart == pcm16.loopStart &&
                      decoded16.loopLength == pcm16.loopLength &&
                      decoded16.samples == pcm16.samples,
                  "PCM16 encodes deterministically and decodes with exact metadata, chunks, and samples");

        const auto encoded8 = Cnb::EncodeSoundEffectToCnb(pcm8, "sounds/eight-bit");
        const auto decoded8 = Cnb::DecodeSoundEffectFromCnb(
            Cnb::CnbDocument::Parse(encoded8, "pcm8.cnb"));
        ctx.Check(decoded8.format == Cnb::CnbAudioFormat::Pcm8 &&
                      decoded8.sampleRate == pcm8.sampleRate && decoded8.channels == pcm8.channels &&
                      decoded8.frameCount == pcm8.frameCount &&
                      decoded8.loopStart == pcm8.loopStart &&
                      decoded8.loopLength == pcm8.loopLength && decoded8.samples == pcm8.samples,
                  "schema 2 preserves PCM8 width and the complete sound description");

        Cnb::CnbSoundEffectData invalid = pcm16;
        invalid.format = Cnb::CnbAudioFormat::PcmFloat32;
        const bool reservedEncode =
            RefusesContent([&] { (void)Cnb::EncodeSoundEffectToCnb(invalid); });
        invalid = pcm16;
        invalid.sampleRate = 0u;
        const bool zeroRate = RefusesContent([&] { (void)Cnb::EncodeSoundEffectToCnb(invalid); });
        invalid = pcm16;
        invalid.sampleRate = Cnb::CnbMaxAudioSampleRate + 1u;
        const bool highRate = RefusesContent([&] { (void)Cnb::EncodeSoundEffectToCnb(invalid); });
        invalid = pcm16;
        invalid.channels = 3u;
        const bool channels = RefusesContent([&] { (void)Cnb::EncodeSoundEffectToCnb(invalid); });
        invalid = pcm16;
        invalid.samples.pop_back();
        const bool payload = RefusesContent([&] { (void)Cnb::EncodeSoundEffectToCnb(invalid); });
        invalid = pcm16;
        invalid.loopStart = 2u;
        const bool loop = RefusesContent([&] { (void)Cnb::EncodeSoundEffectToCnb(invalid); });
        ctx.Check(reservedEncode && zeroRate && highRate && channels && payload && loop,
                  "the encoder refuses unsupported formats and every inconsistent public field");

        const auto legacy16 = Cnb::DecodeSoundEffectFromCnb(
            Cnb::CnbDocument::Parse(BuildRawSound(pcm16, 1u), "legacy-pcm16.cnb"));
        ctx.Check(legacy16.format == pcm16.format && legacy16.samples == pcm16.samples,
                  "the decoder keeps backward compatibility with a valid schema-1 PCM16 file");

        const bool oldPcm8 = RefusesContent([&]
        {
            (void)Cnb::DecodeSoundEffectFromCnb(
                Cnb::CnbDocument::Parse(BuildRawSound(pcm8, 1u), "legacy-pcm8.cnb"));
        });
        const bool reservedFlags = RefusesContent([&]
        {
            (void)Cnb::DecodeSoundEffectFromCnb(
                Cnb::CnbDocument::Parse(BuildRawSound(pcm16, 2u, 1u), "flags.cnb"));
        });
        invalid = pcm16;
        invalid.frameCount = 3u;
        const bool wrongLength = RefusesContent([&]
        {
            (void)Cnb::DecodeSoundEffectFromCnb(
                Cnb::CnbDocument::Parse(BuildRawSound(invalid, 2u), "length.cnb"));
        });
        const bool wrongAsset = RefusesContent([&]
        {
            (void)Cnb::DecodeSoundEffectFromCnb(Cnb::CnbDocument::Parse(
                BuildRawSound(pcm16, 2u, 0u, Cnb::CnbAssetTypeId::Texture2D), "texture.cnb"));
        });
        const bool missingData = RefusesContent([&]
        {
            (void)Cnb::DecodeSoundEffectFromCnb(
                Cnb::CnbDocument::Parse(BuildRawSound(pcm16, 2u, 0u,
                                                      Cnb::CnbAssetTypeId::SoundEffect, 1u, 0u),
                                        "missing.cnb"));
        });
        const bool duplicateHeader = RefusesContent([&]
        {
            (void)Cnb::DecodeSoundEffectFromCnb(
                Cnb::CnbDocument::Parse(BuildRawSound(pcm16, 2u, 0u,
                                                      Cnb::CnbAssetTypeId::SoundEffect, 2u, 1u),
                                        "duplicate.cnb"));
        });
        ctx.Check(oldPcm8 && reservedFlags && wrongLength && wrongAsset && missingData &&
                      duplicateHeader,
                  "the decoder enforces schema compatibility, identity, singleton chunks, flags, and length");
    }
}

CNA_DEMOS_CONSOLE(CnbSoundEffectCodec, {
    .id = "content.sound_effect.cnb_codec",
    .title = "CNB SoundEffect codec",
    .description = "Exercises CNA's complete public SoundEffect CNB format table, exact PCM adaptation, deterministic "
                   "encode/decode round trips, schema-1 compatibility, and malformed-description refusals.",
    .apis = {"CNA::Content::Cnb::CnbAudioFormat",
             "CNA::Content::Cnb::CnbAudioFormatMax",
             "CNA::Content::Cnb::CnbSoundEffectChunk::Header",
             "CNA::Content::Cnb::CnbSoundEffectChunk::Data",
             "CNA::Content::Cnb::CnbSoundEffectSchemaVersion",
             "CNA::Content::Cnb::CnbSoundEffectSchemaVersionForFormat",
             "CNA::Content::Cnb::CnbSoundEffectHeaderStride",
             "CNA::Content::Cnb::CnbMaxAudioSampleRate",
             "CNA::Content::Cnb::CnbSoundEffectData",
             "CNA::Content::Cnb::CnbAudioFrameBytes",
             "CNA::Content::Cnb::CnbAudioFormatToString",
             "CNA::Content::Cnb::CnbSoundEffectSamplesAsPcm16",
             "CNA::Content::Cnb::EncodeSoundEffectToCnb",
             "CNA::Content::Cnb::DecodeSoundEffectFromCnb"},
    .tags = {"content", "audio", "cnb", "codec", "pcm", "serialization"},
})
