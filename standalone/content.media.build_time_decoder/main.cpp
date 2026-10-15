// SPDX-License-Identifier: MS-PL
// Audit CNA's optional build-time media decoder without starting audio or video playback.
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "CNA/Content/Import/ImportedSound.hpp"
#include "CNA/Content/Pipeline/BuildTimeMediaDecoder.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

namespace
{
    namespace BuildTimeMedia = CNA::Content::Pipeline::BuildTimeMedia;
    namespace Import = CNA::Content::Import;
    using Microsoft::Xna::Framework::TitleLocation;

    class TemporaryMediaFile
    {
    public:
        TemporaryMediaFile()
        {
            const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
            path_ = std::filesystem::temp_directory_path()
                / ("cna-demos-build-time-media-" + std::to_string(nonce) + ".wma");
        }

        ~TemporaryMediaFile()
        {
            std::error_code error;
            std::filesystem::remove(path_, error);
        }

        TemporaryMediaFile(const TemporaryMediaFile&) = delete;
        TemporaryMediaFile& operator=(const TemporaryMediaFile&) = delete;

        [[nodiscard]] const std::filesystem::path& Path() const noexcept { return path_; }

    private:
        std::filesystem::path path_;
    };

    template<typename Function>
    bool RefusesWithRuntimeError(Function&& function)
    {
        try
        {
            function();
        }
        catch (const std::runtime_error&)
        {
            return true;
        }
        return false;
    }

    [[nodiscard]] bool Near(const std::int64_t value, const std::int64_t expected,
                            const std::int64_t tolerance)
    {
        return value >= expected - tolerance && value <= expected + tolerance;
    }

    int Run()
    {
        if (!BuildTimeMedia::IsAvailable())
        {
            std::cout << "SKIPPED: " << BuildTimeMedia::UnavailableReason() << '\n';
            return 77;
        }

        bool passed = true;
        const auto check = [&](const bool condition, const std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };

        const BuildTimeMedia::DecodedAudio safeAudio;
        const BuildTimeMedia::ProbedVideo safeVideo;
        check(safeAudio.channels == 0 && safeAudio.sampleRate == 0
                  && safeAudio.bitsPerSample == 16 && safeAudio.pcm.empty()
                  && safeAudio.durationTicks == 0 && safeVideo.width == 0
                  && safeVideo.height == 0 && safeVideo.framesPerSecond == 0.0f
                  && safeVideo.bitsPerSecond == 0 && safeVideo.durationTicks == 0
                  && !safeVideo.hasAudio,
              "decoded audio and probed video carriers have safe complete defaults");

        check(BuildTimeMedia::UnavailableReason().empty()
                  && !BuildTimeMedia::Identity().empty()
                  && BuildTimeMedia::Identity() != "none",
              "the available decoder reports no refusal and supplies a fingerprint identity");

        const std::filesystem::path assets =
            std::filesystem::path(TitleLocation::Path()) / "assets" / "media";
        const std::filesystem::path ogg = assets / "music" / "theme.ogg";
        const std::filesystem::path mp3 =
            assets / "library_music" / "track_embedded_mp3.mp3";
        const std::filesystem::path wmv = assets / "videos" / "bounce.wmv";
        const std::filesystem::path missing = assets / "missing-media-file.bin";

        const BuildTimeMedia::ProbedVideo video = BuildTimeMedia::ProbeVideo(wmv.string());
        check(video.width == 160 && video.height == 120
                  && std::abs(video.framesPerSecond - 15.0f) < 0.01f
                  && video.bitsPerSecond > 0
                  && Near(video.durationTicks, 30000000, 1000000) && video.hasAudio,
              "the WMV probe reports its exact frame shape, rate, positive bit rate, duration and audio stream");

        const auto videoProbe = BuildTimeMedia::MakeVideoMetadataProbe();
        const auto videoMetadata = videoProbe(wmv);
        check(static_cast<bool>(videoProbe) && videoMetadata.has_value()
                  && videoMetadata->width == 160u && videoMetadata->height == 120u
                  && std::abs(videoMetadata->framesPerSecond - 15.0f) < 0.01f
                  && videoMetadata->durationMs >= 2900u && videoMetadata->durationMs <= 3100u
                  && videoMetadata->soundtrackType == 0u && !videoProbe(missing).has_value(),
              "the video adapter converts ticks to bounded metadata and contains an unreadable source");

        const std::int64_t oggTicks = BuildTimeMedia::ProbeAudioDurationTicks(ogg.string());
        check(Near(oggTicks, 40000000, 1000000),
              "the direct audio probe reports the authored four-second Ogg duration");

        const auto songProbe = BuildTimeMedia::MakeSongDurationProbe();
        const auto songDuration = songProbe(ogg);
        check(static_cast<bool>(songProbe) && songDuration.has_value()
                  && *songDuration >= 3900u && *songDuration <= 4100u
                  && !songProbe(missing).has_value(),
              "the song adapter reports milliseconds and contains an unreadable source");

        const BuildTimeMedia::DecodedAudio decodedOgg = BuildTimeMedia::DecodeAudio(
            ogg.string(), 0, BuildTimeMedia::AudioSourceFormat::Any);
        check(decodedOgg.channels == 1 && decodedOgg.sampleRate == 22050
                  && decodedOgg.bitsPerSample == 16 && !decodedOgg.pcm.empty()
                  && decodedOgg.pcm.size() % 2u == 0u
                  && Near(decodedOgg.durationTicks, 40000000, 1000000),
              "Any-format decoding preserves the Ogg channel/rate shape and returns complete PCM16 frames");

        const BuildTimeMedia::DecodedAudio decodedMp3 = BuildTimeMedia::DecodeAudio(
            mp3.string(), 44100, BuildTimeMedia::AudioSourceFormat::Mpeg);
        check(decodedMp3.channels == 1 && decodedMp3.sampleRate == 44100
                  && decodedMp3.bitsPerSample == 16 && !decodedMp3.pcm.empty()
                  && decodedMp3.pcm.size() % 2u == 0u && decodedMp3.durationTicks > 0,
              "the MPEG-only route accepts the real MP3 and resamples it to the requested 44100 Hz");

        const auto soundDecoder = BuildTimeMedia::MakeCompressedSoundDecoder();
        const auto imported = soundDecoder(mp3);
        check(static_cast<bool>(soundDecoder) && imported.has_value()
                  && imported->encoding == Import::ImportedPcmEncoding::Signed16LittleEndian
                  && imported->sampleRate == 44100u && imported->channels == 1u
                  && imported->frameCount > 0u && imported->loopStart == 0u
                  && imported->loopLength == 0u
                  && imported->samples.size()
                      == static_cast<std::size_t>(imported->frameCount) * 2u
                  && !soundDecoder(missing).has_value(),
              "the compressed-sound adapter returns owned 44100 Hz PCM16 with no invented loop and contains failure");

        check(RefusesWithRuntimeError([&] {
                  (void) BuildTimeMedia::DecodeAudio(
                      ogg.string(), 0, BuildTimeMedia::AudioSourceFormat::Mpeg);
              })
                  && RefusesWithRuntimeError([&] {
                      (void) BuildTimeMedia::DecodeAudio(
                          ogg.string(), 0, BuildTimeMedia::AudioSourceFormat::WindowsMedia);
                  }),
              "format-specific MPEG and Windows Media routes refuse a readable Ogg source");

        std::vector<std::uint8_t> pcm(22050u * 2u);
        for (std::size_t frame = 0u; frame < 22050u; ++frame)
        {
            const auto sample = static_cast<std::int16_t>((static_cast<int>(frame % 200u) - 100) * 120);
            pcm[frame * 2u] = static_cast<std::uint8_t>(sample & 0xFF);
            pcm[frame * 2u + 1u] = static_cast<std::uint8_t>(
                static_cast<std::uint16_t>(sample) >> 8u);
        }
        TemporaryMediaFile encodedFile;
        BuildTimeMedia::EncodeWindowsMediaAudio(
            pcm, 1, 22050, 48000, encodedFile.Path().string());
        const BuildTimeMedia::DecodedAudio decodedWma = BuildTimeMedia::DecodeAudio(
            encodedFile.Path().string(), 0, BuildTimeMedia::AudioSourceFormat::WindowsMedia);
        check(std::filesystem::file_size(encodedFile.Path()) > 0u && decodedWma.channels == 1
                  && decodedWma.sampleRate == 22050 && decodedWma.bitsPerSample == 16
                  && !decodedWma.pcm.empty() && decodedWma.durationTicks > 0,
              "encoded Windows Media audio is a nonempty ASF file accepted by the WindowsMedia-only decoder");

        check(RefusesWithRuntimeError([&] {
                  BuildTimeMedia::EncodeWindowsMediaAudio({}, 1, 22050, 48000,
                                                         encodedFile.Path().string());
              })
                  && RefusesWithRuntimeError([&] {
                      (void) BuildTimeMedia::ProbeVideo(ogg.string());
                  })
                  && RefusesWithRuntimeError([&] {
                      (void) BuildTimeMedia::ProbeAudioDurationTicks(missing.string());
                  }),
              "the encoder refuses empty samples and direct probes distinguish no-video and missing-file failures");

        std::cout << "Decoded Ogg/MP3/WMV inputs and round-tripped generated Windows Media audio.\n";
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
        std::cerr << "Build-time media audit failed: " << error.what() << '\n';
        return 1;
    }
}
