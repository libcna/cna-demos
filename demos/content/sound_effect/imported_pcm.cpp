// SPDX-License-Identifier: MS-PL
// Source-oriented WAV data and the explicit processing step that produces CNB PCM.
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbSourceImport.hpp"
#include "CNA/Content/Import/ImportedSound.hpp"
#include "CNA/Content/Pipeline/ProcessedSoundEffect.hpp"
#include "CNA/Content/Pipeline/SoundEffectContentPipeline.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Import = CNA::Content::Import;
namespace Cnb = CNA::Content::Cnb;
namespace Pipeline = CNA::Content::Pipeline;
using Microsoft::Xna::Framework::Content::ContentLoadException;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    std::vector<std::uint8_t> ReadBytes(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("Could not read the WAV fixture");
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }

    bool SameImportedSound(const Import::ImportedSound& left,
                           const Import::ImportedSound& right)
    {
        return left.encoding == right.encoding && left.sampleRate == right.sampleRate &&
            left.channels == right.channels && left.frameCount == right.frameCount &&
            left.loopStart == right.loopStart && left.loopLength == right.loopLength &&
            left.samples == right.samples;
    }

    bool SameCnbSound(const Cnb::CnbSoundEffectData& left,
                      const Cnb::CnbSoundEffectData& right)
    {
        return left.format == right.format && left.sampleRate == right.sampleRate &&
            left.channels == right.channels && left.frameCount == right.frameCount &&
            left.loopStart == right.loopStart && left.loopLength == right.loopLength &&
            left.samples == right.samples;
    }

    template<class Function>
    bool RefusesContent(Function&& function)
    {
        try
        {
            function();
            return false;
        }
        catch (const ContentLoadException&)
        {
            return true;
        }
    }

    void AppendU32(std::vector<std::uint8_t>& bytes, const std::uint32_t value)
    {
        bytes.push_back(static_cast<std::uint8_t>(value));
        bytes.push_back(static_cast<std::uint8_t>(value >> 8));
        bytes.push_back(static_cast<std::uint8_t>(value >> 16));
        bytes.push_back(static_cast<std::uint8_t>(value >> 24));
    }

    std::vector<std::int16_t> Pcm16Values(const Cnb::CnbSoundEffectData& sound)
    {
        std::vector<std::int16_t> values;
        values.reserve(sound.samples.size() / 2u);
        for (std::size_t offset = 0; offset < sound.samples.size(); offset += 2u)
        {
            values.push_back(static_cast<std::int16_t>(
                static_cast<std::uint16_t>(sound.samples[offset]) |
                (static_cast<std::uint16_t>(sound.samples[offset + 1u]) << 8u)));
        }
        return values;
    }

    Import::ImportedSound MakeImported(
        const Import::ImportedPcmEncoding encoding, std::vector<std::uint8_t> samples,
        const std::uint32_t frames)
    {
        Import::ImportedSound sound;
        sound.encoding = encoding;
        sound.sampleRate = 8000u;
        sound.channels = 1u;
        sound.frameCount = frames;
        sound.samples = std::move(samples);
        return sound;
    }

    void ImportedPcm(CnaDemos::DemoContext& ctx)
    {
        using Import::ImportedPcmEncoding;

        const Import::ImportedSound defaults;
        ctx.Check(Import::ImportedContentApiIsExperimental &&
                      defaults.encoding == ImportedPcmEncoding::Signed16LittleEndian &&
                      defaults.sampleRate == 0u && defaults.channels == 0u &&
                      defaults.frameCount == 0u && defaults.loopStart == 0u &&
                      defaults.loopLength == 0u && defaults.samples.empty(),
                  "ImportedSound identifies its experimental source-data contract and safe defaults");

        struct EncodingExpectation
        {
            ImportedPcmEncoding encoding;
            std::uint32_t bytes;
            bool narrows;
            const char* name;
        };
        constexpr std::array encodings{
            EncodingExpectation{ImportedPcmEncoding::Unsigned8, 1u, false, "8-bit unsigned PCM"},
            EncodingExpectation{ImportedPcmEncoding::Signed16LittleEndian, 2u, false, "16-bit PCM"},
            EncodingExpectation{ImportedPcmEncoding::Signed24LittleEndian, 3u, true, "24-bit PCM"},
            EncodingExpectation{ImportedPcmEncoding::Signed32LittleEndian, 4u, true, "32-bit PCM"},
            EncodingExpectation{ImportedPcmEncoding::Float32LittleEndian, 4u, true, "32-bit IEEE float"},
        };
        bool encodingTableMatches = true;
        for (const auto& expected : encodings)
        {
            encodingTableMatches = encodingTableMatches &&
                Import::ImportedPcmSampleBytes(expected.encoding) == expected.bytes &&
                Import::ImportedPcmNarrowsToPcm16(expected.encoding) == expected.narrows &&
                std::string(Import::ImportedPcmEncodingName(expected.encoding)) == expected.name;
        }
        const auto unknown = static_cast<ImportedPcmEncoding>(99);
        ctx.Check(encodingTableMatches && Import::ImportedPcmSampleBytes(unknown) == 0u &&
                      !Import::ImportedPcmNarrowsToPcm16(unknown) &&
                      std::string(Import::ImportedPcmEncodingName(unknown)) == "an unknown encoding",
                  "the encoding helpers expose every width, narrowing decision and diagnostic name");

        const auto wavPath =
            std::filesystem::path(TitleLocation::Path()) / "assets/sounds/tone_660.wav";
        const Import::ImportedSound imported = Cnb::ImportWavAsImportedSound(wavPath);
        ctx.Check(imported.encoding == ImportedPcmEncoding::Signed16LittleEndian &&
                      imported.sampleRate == 22050u && imported.channels == 1u &&
                      imported.frameCount == 5512u && imported.loopStart == 0u &&
                      imported.loopLength == 0u && imported.samples.size() == 11024u,
                  "the headless WAV importer preserves the authored PCM width, rate, frames and loop state");

        const std::vector<std::uint8_t> wavBytes = ReadBytes(wavPath);
        const Import::ImportedSound importedNarrow =
            Cnb::ImportWavAsImportedSound(wavPath.string());
        const Import::ImportedSound importedMemory =
            Cnb::DecodeWavAsImportedSound(wavBytes, "memory tone_660.wav");
        ctx.Check(SameImportedSound(imported, importedNarrow) &&
                      SameImportedSound(imported, importedMemory),
                  "WAV memory and both path overloads return the same source-oriented PCM value");

        const auto asAuthored = Cnb::ProcessImportedSoundEffect(
            imported, Cnb::SoundEffectLoopPolicy::AsAuthored);
        const auto wholeSound = Cnb::ProcessImportedSoundEffect(
            imported, Cnb::SoundEffectLoopPolicy::WholeSoundWhenUnset);
        ctx.Check(asAuthored.format == Cnb::CnbAudioFormat::Pcm16 &&
                      asAuthored.samples == imported.samples && asAuthored.loopLength == 0u &&
                      wholeSound.samples == imported.samples &&
                      wholeSound.loopStart == 0u && wholeSound.loopLength == imported.frameCount,
                  "processing preserves exact PCM while the selected loop policy owns an unset loop");

        const Cnb::CnbSoundEffectData cnbNative = Cnb::ImportWavAsCnbSoundEffect(wavPath);
        const Cnb::CnbSoundEffectData cnbNarrow =
            Cnb::ImportWavAsCnbSoundEffect(wavPath.string());
        const Cnb::CnbSoundEffectData cnbMemory =
            Cnb::DecodeWavAsCnbSoundEffect(wavBytes, "memory tone_660.wav");
        ctx.Check(SameCnbSound(cnbNative, wholeSound) && SameCnbSound(cnbNarrow, wholeSound) &&
                      SameCnbSound(cnbMemory, wholeSound),
                  "the compatibility decoder and both file overloads apply the whole-sound loop policy identically");

        ctx.Check(RefusesContent([&]
                  {
                      (void)Cnb::DecodeWavAsImportedSound(
                          std::span<const std::uint8_t>(wavBytes).first(11u), "truncated.wav");
                  }) &&
                      RefusesContent([&]
                      {
                          (void)Cnb::ImportWavAsCnbSoundEffect(
                              wavPath.parent_path() / "missing.wav");
                      }),
                  "WAV import refuses truncated memory and a missing source path independently");

        const Pipeline::ProcessedSoundEffect processedDefaults;
        const auto unknownProcessed = static_cast<Pipeline::ProcessedPcmEncoding>(99);
        ctx.Check(processedDefaults.encoding ==
                          Pipeline::ProcessedPcmEncoding::Signed16LittleEndian &&
                      processedDefaults.sampleRate == 0u && processedDefaults.channels == 0u &&
                      processedDefaults.frameCount == 0u && processedDefaults.loopStart == 0u &&
                      processedDefaults.loopLength == 0u && processedDefaults.samples.empty() &&
                      Pipeline::ProcessedPcmSampleBytes(Pipeline::ProcessedPcmEncoding::Unsigned8) == 1u &&
                      Pipeline::ProcessedPcmBitsPerSample(Pipeline::ProcessedPcmEncoding::Unsigned8) == 8u &&
                      Pipeline::ProcessedPcmSampleBytes(
                          Pipeline::ProcessedPcmEncoding::Signed16LittleEndian) == 2u &&
                      Pipeline::ProcessedPcmBitsPerSample(
                          Pipeline::ProcessedPcmEncoding::Signed16LittleEndian) == 16u &&
                      std::string(Pipeline::ProcessedPcmEncodingName(
                          Pipeline::ProcessedPcmEncoding::Unsigned8)) == "8-bit unsigned PCM" &&
                      Pipeline::ProcessedPcmSampleBytes(unknownProcessed) == 0u &&
                      Pipeline::ProcessedPcmBitsPerSample(unknownProcessed) == 0u &&
                      std::string(Pipeline::ProcessedPcmEncodingName(unknownProcessed)) ==
                          "an unknown encoding",
                  "ProcessedSoundEffect exposes safe defaults and its complete two-encoding helper table");

        const Pipeline::ProcessedSoundEffect xnaProcessed =
            Pipeline::MakeProcessedSoundEffect(imported);
        ctx.Check(xnaProcessed.encoding ==
                          Pipeline::ProcessedPcmEncoding::Signed16LittleEndian &&
                      xnaProcessed.sampleRate == imported.sampleRate &&
                      xnaProcessed.channels == imported.channels &&
                      xnaProcessed.frameCount == imported.frameCount &&
                      xnaProcessed.loopStart == 0u &&
                      xnaProcessed.loopLength == imported.frameCount &&
                      xnaProcessed.samples == imported.samples &&
                      Pipeline::ProcessedFrameBytes(xnaProcessed) == 2u,
                  "the XNA-compatible processor preserves 16-bit PCM and supplies the whole-source loop");

        const Cnb::CnbSoundEffectData adapted = Pipeline::ToCnbSoundEffect(xnaProcessed);
        ctx.Check(adapted.format == Cnb::CnbAudioFormat::Pcm16 &&
                      adapted.sampleRate == xnaProcessed.sampleRate &&
                      adapted.channels == xnaProcessed.channels &&
                      adapted.frameCount == xnaProcessed.frameCount &&
                      adapted.loopStart == xnaProcessed.loopStart &&
                      adapted.loopLength == xnaProcessed.loopLength &&
                      adapted.samples == xnaProcessed.samples,
                  "the CNB writer adapter changes only the container-specific format identity");

        const Pipeline::ProcessedSoundEffect pcm8Processed =
            Pipeline::MakeProcessedSoundEffect(MakeImported(
                ImportedPcmEncoding::Unsigned8, {0u, 64u, 128u, 255u}, 4u));
        ctx.Check(pcm8Processed.encoding == Pipeline::ProcessedPcmEncoding::Unsigned8 &&
                      Pipeline::ProcessedFrameBytes(pcm8Processed) == 1u &&
                      pcm8Processed.loopLength == 4u &&
                      pcm8Processed.samples == std::vector<std::uint8_t>{0u, 64u, 128u, 255u} &&
                      Pipeline::ToCnbSoundEffect(pcm8Processed).format == Cnb::CnbAudioFormat::Pcm8,
                  "the processor and CNB adapter retain an 8-bit source without widening it");

        bool widerXnaSourceRejected = false;
        try
        {
            (void)Pipeline::MakeProcessedSoundEffect(MakeImported(
                ImportedPcmEncoding::Signed24LittleEndian, {0u, 0u, 0u}, 1u));
        }
        catch (const ContentLoadException&)
        {
            widerXnaSourceRejected = true;
        }
        ctx.Check(widerXnaSourceRejected,
                  "the XNA-compatible processor refuses wider PCM instead of silently narrowing it");

        const auto pcm24 = Cnb::ProcessImportedSoundEffect(MakeImported(
            ImportedPcmEncoding::Signed24LittleEndian,
            {0x00u, 0x00u, 0x80u, 0xFFu, 0xFFu, 0xFFu,
             0x80u, 0x00u, 0x00u, 0xFFu, 0xFFu, 0x7Fu},
            4u));
        ctx.Check(Pcm16Values(pcm24) ==
                      std::vector<std::int16_t>{-32768, 0, 1, 32767},
                  "24-bit processing rounds halves upward and saturates at the signed-16 limits");

        std::vector<std::uint8_t> pcm32Bytes;
        for (const std::uint32_t value :
             {UINT32_C(0x80000000), UINT32_C(0xFFFF8000),
              UINT32_C(0x00008000), UINT32_C(0x7FFFFFFF)})
        {
            AppendU32(pcm32Bytes, value);
        }
        const auto pcm32 = Cnb::ProcessImportedSoundEffect(MakeImported(
            ImportedPcmEncoding::Signed32LittleEndian, std::move(pcm32Bytes), 4u));
        ctx.Check(Pcm16Values(pcm32) ==
                      std::vector<std::int16_t>{-32768, 0, 1, 32767},
                  "32-bit processing uses the same documented rounding and saturation rule");

        std::vector<std::uint8_t> floatBytes;
        for (const float value :
             {-2.0f, -1.0f, std::numeric_limits<float>::quiet_NaN(), 0.5f, 2.0f})
        {
            AppendU32(floatBytes, std::bit_cast<std::uint32_t>(value));
        }
        const auto pcmFloat = Cnb::ProcessImportedSoundEffect(MakeImported(
            ImportedPcmEncoding::Float32LittleEndian, std::move(floatBytes), 5u));
        ctx.Check(Pcm16Values(pcmFloat) ==
                      std::vector<std::int16_t>{-32768, -32767, 0, 16384, 32767},
                  "float processing clamps finite samples and maps NaN to safe silence");

        bool shortPayloadRejected = false;
        bool unknownEncodingRejected = false;
        try
        {
            (void)Cnb::ProcessImportedSoundEffect(MakeImported(
                ImportedPcmEncoding::Signed16LittleEndian, {0u, 0u}, 2u));
        }
        catch (const ContentLoadException&)
        {
            shortPayloadRejected = true;
        }
        try
        {
            (void)Cnb::ProcessImportedSoundEffect(MakeImported(unknown, {}, 0u));
        }
        catch (const ContentLoadException&)
        {
            unknownEncodingRejected = true;
        }
        ctx.Check(shortPayloadRejected && unknownEncodingRejected,
                  "processing refuses inconsistent byte counts and unknown source encodings");
    }
}

CNA_DEMOS_CONSOLE(ImportedPcm, {
    .id = "content.sound_effect.imported_pcm",
    .title = "Imported PCM processing",
    .description = "Keeps source WAV encoding and loop data in ImportedSound before applying explicit CNB processing "
                   "policy, including deterministic integer and float narrowing without an audio device.",
    .apis = {"CNA::Content::Import::ImportedContentApiIsExperimental",
             "CNA::Content::Import::ImportedPcmEncoding", "CNA::Content::Import::ImportedPcmSampleBytes",
             "CNA::Content::Import::ImportedPcmNarrowsToPcm16", "CNA::Content::Import::ImportedPcmEncodingName",
             "CNA::Content::Import::ImportedSound", "CNA::Content::Cnb::ImportWavAsImportedSound",
             "CNA::Content::Cnb::DecodeWavAsImportedSound",
             "CNA::Content::Cnb::ImportWavAsCnbSoundEffect",
             "CNA::Content::Cnb::DecodeWavAsCnbSoundEffect",
             "CNA::Content::Cnb::ProcessImportedSoundEffect", "CNA::Content::Cnb::SoundEffectLoopPolicy",
             "CNA::Content::Pipeline::ProcessedPcmEncoding",
             "CNA::Content::Pipeline::ProcessedPcmSampleBytes",
             "CNA::Content::Pipeline::ProcessedPcmBitsPerSample",
             "CNA::Content::Pipeline::ProcessedPcmEncodingName",
             "CNA::Content::Pipeline::ProcessedSoundEffect",
             "CNA::Content::Pipeline::ProcessedFrameBytes",
             "CNA::Content::Pipeline::MakeProcessedSoundEffect",
             "CNA::Content::Pipeline::ToCnbSoundEffect"},
    .tags = {"content", "pipeline", "audio", "wav", "pcm", "import"},
    .assets = {"sounds/tone_660.wav"},
})
