// SPDX-License-Identifier: MS-PL
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "CNA/Content/Pipeline/XmaEncoderService.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Audio/AudioContent.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace Build = CNA::Content::Pipeline;
namespace Audio = Microsoft::Xna::Framework::Content::Pipeline::Audio;
using Microsoft::Xna::Framework::Content::Pipeline::InvalidContentException;

namespace
{
    void PutU16(std::vector<std::uint8_t>& bytes, const std::uint16_t value)
    {
        bytes.push_back(static_cast<std::uint8_t>(value & 0xFFu));
        bytes.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xFFu));
    }

    void PutU32(std::vector<std::uint8_t>& bytes, const std::uint32_t value)
    {
        for (int shift = 0; shift < 32; shift += 8)
            bytes.push_back(static_cast<std::uint8_t>((value >> shift) & 0xFFu));
    }

    void PutTag(std::vector<std::uint8_t>& bytes, const std::string_view tag)
    {
        bytes.insert(bytes.end(), tag.begin(), tag.end());
    }

    std::uint16_t U16(const std::vector<std::uint8_t>& bytes, const std::size_t at)
    {
        return at + 1u < bytes.size()
                   ? static_cast<std::uint16_t>(bytes[at] | (bytes[at + 1u] << 8u))
                   : 0u;
    }

    std::uint32_t U32(const std::vector<std::uint8_t>& bytes, const std::size_t at)
    {
        if (at + 3u >= bytes.size()) return 0u;
        return static_cast<std::uint32_t>(bytes[at]) |
               (static_cast<std::uint32_t>(bytes[at + 1u]) << 8u) |
               (static_cast<std::uint32_t>(bytes[at + 2u]) << 16u) |
               (static_cast<std::uint32_t>(bytes[at + 3u]) << 24u);
    }

    std::vector<std::uint8_t> XmaFormat(const int channels, const int sampleRate,
                                        const int bitsPerSample)
    {
        std::vector<std::uint8_t> format;
        PutU16(format, 0x0166u);
        PutU16(format, static_cast<std::uint16_t>(channels));
        PutU32(format, static_cast<std::uint32_t>(sampleRate));
        PutU32(format, 4000u);
        PutU16(format, 2u);
        PutU16(format, static_cast<std::uint16_t>(bitsPerSample));
        PutU16(format, 34u);
        for (std::uint8_t value = 1u; value <= 34u; ++value) format.push_back(value);
        return format;
    }

    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };

    std::filesystem::path MakeTemporaryDirectory()
    {
        const auto path = std::filesystem::temp_directory_path() /
            ("cna-demos-xma-service-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(path))
            throw std::runtime_error("Could not reserve a private XMA workspace.");
        return path;
    }

    std::filesystem::path WriteWave(const std::filesystem::path& directory,
                                    const std::string& name)
    {
        constexpr int frames = 400;
        constexpr int sampleRate = 8000;
        std::vector<std::uint8_t> pcm;
        pcm.reserve(frames * 2u);
        for (int frame = 0; frame < frames; ++frame)
            PutU16(pcm, static_cast<std::uint16_t>(static_cast<std::int16_t>(frame * 37 - 7000)));

        std::vector<std::uint8_t> wave;
        PutTag(wave, "RIFF"); PutU32(wave, static_cast<std::uint32_t>(36u + pcm.size()));
        PutTag(wave, "WAVEfmt "); PutU32(wave, 16u); PutU16(wave, 1u); PutU16(wave, 1u);
        PutU32(wave, sampleRate); PutU32(wave, sampleRate * 2u); PutU16(wave, 2u);
        PutU16(wave, 16u); PutTag(wave, "data"); PutU32(wave, static_cast<std::uint32_t>(pcm.size()));
        wave.insert(wave.end(), pcm.begin(), pcm.end());

        const auto path = directory / name;
        std::ofstream output(path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(wave.data()),
                     static_cast<std::streamsize>(wave.size()));
        if (!output) throw std::runtime_error("Could not write the private PCM wave.");
        return path;
    }

    std::optional<std::string> Argument(int argc, char** argv, const int start,
                                        const std::string_view name)
    {
        for (int index = start; index + 1 < argc; ++index)
            if (argv[index] == name) return std::string(argv[index + 1]);
        return std::nullopt;
    }

    int RunExternalStub(int argc, char** argv, const int start)
    {
        const std::string mode = Argument(argc, argv, start, "--mode").value_or("success");
        if (mode == "fail")
        {
            std::cerr << "controlled encoder rejected the PCM\n";
            return 7;
        }
        const auto outputName = Argument(argc, argv, start, "--output");
        if (!outputName) return 8;
        if (mode == "invalid")
        {
            std::ofstream(*outputName, std::ios::binary) << "not a RIFF file";
            return 0;
        }
        const auto inputName = Argument(argc, argv, start, "--input");
        const auto quality = Argument(argc, argv, start, "--quality");
        const auto loopStart = Argument(argc, argv, start, "--loop-start");
        const auto loopLength = Argument(argc, argv, start, "--loop-length");
        if (!inputName || !quality || !loopStart || !loopLength) return 9;

        std::ifstream input(*inputName, std::ios::binary);
        const std::vector<std::uint8_t> wave{std::istreambuf_iterator<char>(input),
                                             std::istreambuf_iterator<char>()};
        if (wave.size() < 44u || std::string_view(reinterpret_cast<const char*>(wave.data()), 4) != "RIFF")
            return 10;
        const auto dataAt = std::search(wave.begin(), wave.end(), "data", "data" + 4);
        if (dataAt == wave.end() || static_cast<std::size_t>(wave.end() - dataAt) < 8u) return 11;
        const std::size_t payloadAt = static_cast<std::size_t>(dataAt - wave.begin()) + 8u;
        const std::size_t payloadSize = std::min<std::size_t>(24u, wave.size() - payloadAt);
        const auto format = XmaFormat(U16(wave, 22u), static_cast<int>(U32(wave, 24u)), U16(wave, 34u));

        std::vector<std::uint8_t> riff;
        const std::uint32_t bodySize = static_cast<std::uint32_t>(4u + 8u + format.size() + 8u + payloadSize);
        PutTag(riff, "RIFF"); PutU32(riff, bodySize); PutTag(riff, "WAVEfmt ");
        PutU32(riff, static_cast<std::uint32_t>(format.size()));
        riff.insert(riff.end(), format.begin(), format.end());
        PutTag(riff, "data"); PutU32(riff, static_cast<std::uint32_t>(payloadSize));
        riff.insert(riff.end(), wave.begin() + static_cast<std::ptrdiff_t>(payloadAt),
                    wave.begin() + static_cast<std::ptrdiff_t>(payloadAt + payloadSize));
        std::ofstream output(*outputName, std::ios::binary);
        output.write(reinterpret_cast<const char*>(riff.data()),
                     static_cast<std::streamsize>(riff.size()));
        if (!output) return 12;
        std::cout << "controlled-stub quality=" << *quality << " loop=" << *loopStart << '+'
                  << *loopLength << " pcm=" << wave.size() - payloadAt << '\n';
        return 0;
    }

    int StubArgumentStart(int argc, char** argv)
    {
        if (argc > 1 && std::string_view(argv[1]) == "--xma-encoder-stub") return 2;
        if (argc > 2 && std::filesystem::path(argv[1]).filename() ==
                            std::filesystem::path(argv[0]).filename() &&
            std::string_view(argv[2]) == "--xma-encoder-stub")
            return 3;
        return 0;
    }

    class RecordingEncoder final : public Build::XmaEncoderService
    {
    public:
        explicit RecordingEncoder(const bool fail = false) : fail_(fail) {}

        Build::XmaEncoderIdentity Identity() const override { return {"controlled", "2.5"}; }
        bool Available() const override { return true; }
        std::string UnavailableReason() const override { return {}; }

        Build::XmaEncodeResult Encode(const Build::XmaEncodeRequest& request) const override
        {
            requests_.push_back(request);
            Build::XmaEncodeResult result;
            if (fail_)
            {
                result.diagnostics = {"controlled backend failure", "second diagnostic"};
                return result;
            }
            result.data = {0x10u, 0x20u, 0x30u, 0x40u, 0x50u, 0x60u};
            result.formatBlock = XmaFormat(request.channelCount, request.sampleRate,
                                           request.bitsPerSample);
            result.loopStart = 7;
            result.loopLength = 11;
            result.diagnostics = {"controlled backend note"};
            result.succeeded = true;
            return result;
        }

        const std::vector<Build::XmaEncodeRequest>& Requests() const { return requests_; }

    private:
        bool fail_ = false;
        mutable std::vector<Build::XmaEncodeRequest> requests_;
    };

    struct EncoderRestorer
    {
        std::shared_ptr<const Build::XmaEncoderService> previous = Build::BuildXmaEncoder();
        ~EncoderRestorer() { Build::SetBuildXmaEncoder(previous); }
    };

    template<typename Function>
    std::string Refusal(Function&& function)
    {
        try { function(); }
        catch (const InvalidContentException& error) { return error.getMessageProperty(); }
        catch (const std::exception& error) { return error.what(); }
        return {};
    }

    std::filesystem::path SelfPath()
    {
        std::filesystem::path name = "cna-demo-content-audio-xma_encoder_service";
#if defined(_WIN32)
        name += ".exe";
#endif
        return std::filesystem::path(Microsoft::Xna::Framework::TitleLocation::Path()) / name;
    }

    Build::ExternalXmaEncoderOptions ExternalOptions(const std::filesystem::path& self,
                                                      const std::string& mode,
                                                      const bool throughLauncher)
    {
        Build::ExternalXmaEncoderOptions options;
        options.executable = self;
        if (throughLauncher) options.launcher = self;
        options.arguments = {"--xma-encoder-stub", "--mode", mode,
                             "--quality", "{quality}",
                             "--loop-start", "{loopStart}",
                             "--loop-length", "{loopLength}",
                             "--input", "{input}", "--output", "{output}"};
        return options;
    }
}

int main(int argc, char** argv)
{
    const int stubStart = StubArgumentStart(argc, argv);
    if (stubStart != 0) return RunExternalStub(argc, argv, stubStart);

    try
    {
        const EncoderRestorer restoreEncoder;
        bool passed = true;
        const auto check = [&](const bool condition, const std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };

        check(std::string(Build::XmaEncodeQualityName(Build::XmaEncodeQuality::Low)) == "low" &&
                  std::string(Build::XmaEncodeQualityName(Build::XmaEncodeQuality::Medium)) == "medium" &&
                  std::string(Build::XmaEncodeQualityName(Build::XmaEncodeQuality::Best)) == "best",
              "all XMA encode qualities have exact stable names");

        const Build::XmaEncodeRequest defaultRequest;
        const Build::XmaEncodeResult defaultResult;
        check(defaultRequest.pcm.empty() && defaultRequest.channelCount == 1 &&
                  defaultRequest.sampleRate == 44100 && defaultRequest.bitsPerSample == 16 &&
                  defaultRequest.loopStart == 0 && defaultRequest.loopLength == 0 &&
                  defaultRequest.quality == Build::XmaEncodeQuality::Best &&
                  defaultResult.data.empty() && defaultResult.formatBlock.empty() &&
                  defaultResult.loopStart == 0 && defaultResult.loopLength == 0 &&
                  defaultResult.diagnostics.empty() && !defaultResult.succeeded,
              "request and result carriers expose their complete safe defaults");

        check(Build::XmaEncoderIdentity{}.ToString().empty() &&
                  Build::XmaEncoderIdentity{"encoder", {}}.ToString() == "encoder" &&
                  Build::XmaEncoderIdentity{"encoder", "1.2"}.ToString() == "encoder 1.2",
              "encoder identities distinguish empty, unversioned and versioned backends");

        const auto unavailable = Build::MakeUnavailableXmaEncoder();
        const auto unavailableResult = unavailable->Encode(defaultRequest);
        check(unavailable && !unavailable->Available() && unavailable->Identity().ToString().empty() &&
                  unavailable->UnavailableReason().starts_with(Build::XmaEncoderUnavailableSentence) &&
                  unavailable->UnavailableReason().find("FFmpeg") != std::string::npos &&
                  !unavailableResult.succeeded && unavailableResult.data.empty() &&
                  unavailableResult.diagnostics.size() == 1u &&
                  unavailableResult.diagnostics.front() == unavailable->UnavailableReason(),
              "the explicit unavailable backend retains the canonical reason through Encode");

        Build::ExternalXmaEncoderOptions missingOptions;
        missingOptions.executable = std::filesystem::temp_directory_path() /
            "cna-demos-definitely-missing-xma-encoder";
        missingOptions.launcher = "unused-launcher";
        missingOptions.arguments = {"--in", "{input}", "--out", "{output}"};
        const auto missing = Build::MakeExternalXmaEncoder(missingOptions);
        check(missingOptions == Build::ExternalXmaEncoderOptions{
                                    missingOptions.executable, "unused-launcher",
                                    {"--in", "{input}", "--out", "{output}"}} &&
                  missing && !missing->Available() && missing->Identity().ToString().empty() &&
                  missing->UnavailableReason().find(missingOptions.executable.filename().string()) !=
                      std::string::npos &&
                  !missing->UnavailableReason().starts_with(Build::XmaEncoderUnavailableSentence),
              "an explicitly missing encoder reports its path separately from no attached backend");

        Build::SetBuildXmaEncoder(nullptr);
        const auto defaultAttached = Build::BuildXmaEncoder();
        check(defaultAttached && defaultAttached == Build::BuildXmaEncoder() &&
                  !defaultAttached->Available() &&
                  defaultAttached->UnavailableReason().starts_with(Build::XmaEncoderUnavailableSentence),
              "the process-wide slot lazily returns one stable unavailable service");

        const auto directory = MakeTemporaryDirectory();
        {
            TemporaryDirectory cleanup{directory};
            const auto wave = WriteWave(directory, "tone.wav");
            const auto controlled = std::make_shared<RecordingEncoder>();
            Build::SetBuildXmaEncoder(controlled);
            check(Build::BuildXmaEncoder() == controlled && controlled->Available() &&
                      controlled->UnavailableReason().empty() &&
                      controlled->Identity() == Build::XmaEncoderIdentity{"controlled", "2.5"},
                  "SetBuildXmaEncoder exposes the exact attached service and its identity");

            const std::vector<Audio::ConversionQuality> xnaQualities{
                Audio::ConversionQuality::Low, Audio::ConversionQuality::Medium,
                Audio::ConversionQuality::Best};
            const std::vector<Build::XmaEncodeQuality> serviceQualities{
                Build::XmaEncodeQuality::Low, Build::XmaEncodeQuality::Medium,
                Build::XmaEncodeQuality::Best};
            const auto expectedFormat = XmaFormat(1, 8000, 16);
            const std::vector<SharpRuntime::bytecs> expectedNativeFormat(
                expectedFormat.begin(), expectedFormat.end());
            std::vector<std::uint8_t> originalPcm;
            bool adopted = true;
            for (std::size_t index = 0; index < xnaQualities.size(); ++index)
            {
                Audio::AudioContent audio(wave.string(), Audio::AudioFileType::Wav);
                if (index == 0u)
                    originalPcm.assign(audio.getDataProperty().begin(), audio.getDataProperty().end());
                audio.ConvertFormat(Audio::ConversionFormat::Xma, xnaQualities[index], {});
                adopted &= audio.getDataProperty() ==
                               std::vector<SharpRuntime::bytecs>{0x10u, 0x20u, 0x30u,
                                                                 0x40u, 0x50u, 0x60u} &&
                           audio.getFormatProperty() &&
                           audio.getFormatProperty()->getFormatProperty() == 0x0166 &&
                           audio.getFormatProperty()->getChannelCountProperty() == 1 &&
                           audio.getFormatProperty()->getSampleRateProperty() == 8000 &&
                           audio.getFormatProperty()->getNativeWaveFormatProperty() ==
                               expectedNativeFormat &&
                           audio.getLoopStartProperty() == 7 && audio.getLoopLengthProperty() == 11;
            }
            bool requestsMatch = controlled->Requests().size() == 3u;
            for (std::size_t index = 0; index < controlled->Requests().size(); ++index)
            {
                const auto& request = controlled->Requests()[index];
                requestsMatch &= request.pcm == originalPcm && request.channelCount == 1 &&
                    request.sampleRate == 8000 && request.bitsPerSample == 16 &&
                    request.loopStart == 0 && request.loopLength == 400 &&
                    request.quality == serviceQualities[index];
            }
            check(requestsMatch,
                  "AudioContent forwards exact PCM shape, authored loops and all three qualities");
            check(adopted,
                  "AudioContent adopts the service payload, full XMA2 format block and returned loops");

            const auto failing = std::make_shared<RecordingEncoder>(true);
            Build::SetBuildXmaEncoder(failing);
            Audio::AudioContent failedAudio(wave.string(), Audio::AudioFileType::Wav);
            const auto failureText = Refusal([&] {
                failedAudio.ConvertFormat(Audio::ConversionFormat::Xma,
                                          Audio::ConversionQuality::Best, {});
            });
            check(failing->Requests().size() == 1u &&
                      failureText.find("controlled 2.5") != std::string::npos &&
                      failureText.find("controlled backend failure") != std::string::npos &&
                      failureText.find("second diagnostic") != std::string::npos,
                  "a failed attached backend reports its identity and every diagnostic");

            Build::SetBuildXmaEncoder(nullptr);
            Audio::AudioContent unavailableAudio(wave.string(), Audio::AudioFileType::Wav);
            const auto unavailableText = Refusal([&] {
                unavailableAudio.ConvertFormat(Audio::ConversionFormat::Xma,
                                               Audio::ConversionQuality::Best, {});
            });
            check(unavailableText.find(Build::XmaEncoderUnavailableSentence) != std::string::npos,
                  "AudioContent preserves the canonical no-encoder sentence");

            const auto self = std::filesystem::canonical(SelfPath());
            const auto externalOptions = ExternalOptions(self, "success", true);
            const auto external = Build::MakeExternalXmaEncoder(externalOptions);
            check(externalOptions.launcher == self && external && external->Available() &&
                      external->UnavailableReason().empty() &&
                      external->Identity().backend == self.filename().string() &&
                      !external->Identity().version.empty() &&
                      !external->Identity().ToString().empty(),
                  "the external factory fingerprints an executable driven through its launcher field");

            Build::XmaEncodeRequest externalRequest;
            externalRequest.pcm = originalPcm;
            externalRequest.channelCount = 1;
            externalRequest.sampleRate = 8000;
            externalRequest.bitsPerSample = 16;
            externalRequest.loopStart = 13;
            externalRequest.loopLength = 37;
            externalRequest.quality = Build::XmaEncodeQuality::Medium;
            const auto externalResult = external->Encode(externalRequest);
            bool exactDiagnostic = false;
            for (const auto& diagnostic : externalResult.diagnostics)
                exactDiagnostic |= diagnostic ==
                    "controlled-stub quality=medium loop=13+37 pcm=800\n";
            check(externalResult.succeeded && externalResult.data.size() == 24u &&
                      externalResult.formatBlock == XmaFormat(1, 8000, 16) &&
                      externalResult.loopStart == 13 && externalResult.loopLength == 37 &&
                      exactDiagnostic,
                  "the external seam substitutes quality, loops and files and parses XMA2 RIFF output");

            const auto invalid = Build::MakeExternalXmaEncoder(
                ExternalOptions(self, "invalid", false))->Encode(externalRequest);
            check(!invalid.succeeded && invalid.data.empty() && invalid.formatBlock.empty() &&
                      invalid.diagnostics.size() == 1u &&
                      invalid.diagnostics.front().find("not a RIFF WAVE file") != std::string::npos,
                  "external output that is not RIFF WAVE is refused with the parser diagnosis");

            const auto rejected = Build::MakeExternalXmaEncoder(
                ExternalOptions(self, "fail", false))->Encode(externalRequest);
            bool ownWords = false, status = false;
            for (const auto& diagnostic : rejected.diagnostics)
            {
                ownWords |= diagnostic.find("controlled encoder rejected the PCM") != std::string::npos;
                status |= diagnostic.find("exited with status 7") != std::string::npos;
            }
            check(!rejected.succeeded && rejected.data.empty() && ownWords && status,
                  "a nonzero external encoder preserves its own diagnostic and exact exit status");
        }
        check(!std::filesystem::exists(directory), "the demo removes its private PCM source");

        Build::SetBuildXmaEncoder(nullptr);
        check(!Build::BuildXmaEncoder()->Available(),
              "passing null restores the canonical unavailable backend");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "XMA encoder-service audit failed: " << error.what() << '\n';
        return 1;
    }
}
