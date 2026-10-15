// SPDX-License-Identifier: MS-PL
// Raw/URI Video construction and trusted-metadata playback validation.
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Video/Video.hpp"
#include "Microsoft/Xna/Framework/Media/Video/VideoPlayer.hpp"
#include "Microsoft/Xna/Framework/Media/VideoSoundtrackType.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/InvalidOperationException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    template<typename TException>
    bool ThrowsExactly(const std::function<void()>& action)
    {
        try
        {
            action();
        }
        catch (const TException&)
        {
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    class CurrentPathGuard
    {
    public:
        CurrentPathGuard() : original_(std::filesystem::current_path()) {}
        ~CurrentPathGuard() { std::filesystem::current_path(original_); }

        CurrentPathGuard(const CurrentPathGuard&) = delete;
        CurrentPathGuard& operator=(const CurrentPathGuard&) = delete;

    private:
        std::filesystem::path original_;
    };

    class UriAndMetadataDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            const std::filesystem::path titlePath = TitleLocation::Path();
            const std::filesystem::path mediaPath =
                titlePath / "assets/media/videos/track_matrix.mkv";
            const std::filesystem::path missingPath =
                titlePath / "assets/media/videos/does_not_exist.mkv";

            std::unique_ptr<Video> absolute(
                Video::FromUriEXT(mediaPath.string(), &getGraphicsDeviceProperty()));
            ctx.Check(absolute && absolute->getWidthProperty() == 160 &&
                          absolute->getHeightProperty() == 90 &&
                          absolute->getFramesPerSecondProperty() == 10.0f &&
                          absolute->getDurationProperty().getTotalSecondsProperty() > 0.9,
                      "FromUriEXT probes a plain absolute local path");

            std::unique_ptr<Video> fileUri;
            try
            {
                fileUri.reset(Video::FromUriEXT("file://" + mediaPath.generic_string(),
                                                &getGraphicsDeviceProperty()));
            }
            catch (...)
            {
            }
            ctx.Check(fileUri && fileUri->getWidthProperty() == 160,
                      "FromUriEXT accepts an absolute local file URI (CNA-DEMOS-ISSUE-101)");

            std::unique_ptr<Video> relative;
            {
                CurrentPathGuard guard;
                std::filesystem::current_path(std::filesystem::temp_directory_path());
                try
                {
                    relative.reset(Video::FromUriEXT("assets/media/videos/track_matrix.mkv",
                                                     &getGraphicsDeviceProperty()));
                }
                catch (...)
                {
                }
            }
            ctx.Check(relative && relative->getWidthProperty() == 160,
                      "FromUriEXT resolves a relative URI below TitleLocation "
                      "(CNA-DEMOS-ISSUE-101)");

            ctx.Check(ThrowsExactly<System::InvalidOperationException>([&] {
                          std::unique_ptr<Video> remote(
                              Video::FromUriEXT("https://example.invalid/movie.ogv",
                                                &getGraphicsDeviceProperty()));
                      }),
                      "FromUriEXT rejects a non-file absolute URI with InvalidOperationException "
                      "(CNA-DEMOS-ISSUE-101)");
            ctx.Check(ThrowsExactly<System::IO::FileNotFoundException>([&] {
                          Video missing(missingPath.string(), &getGraphicsDeviceProperty());
                      }),
                      "the raw-file constructor rejects a missing local file");

            Video deferred(missingPath.string(), &getGraphicsDeviceProperty(), 1000, 160, 90,
                           10.0f, VideoSoundtrackType::Dialog);
            ctx.Check(deferred.getDurationProperty() == System::TimeSpan::FromMilliseconds(1000) &&
                          deferred.getWidthProperty() == 160 && deferred.getHeightProperty() == 90 &&
                          deferred.getFramesPerSecondProperty() == 10.0f &&
                          deferred.getVideoSoundtrackTypeProperty() == VideoSoundtrackType::Dialog,
                      "trusted metadata construction defers touching the referenced file");
            VideoPlayer missingPlayer;
            const bool missingPlayRejected = ThrowsExactly<System::IO::FileNotFoundException>(
                [&] { missingPlayer.Play(&deferred); });
            ctx.Check(missingPlayRejected && missingPlayer.getStateProperty() == MediaState::Stopped &&
                          missingPlayer.getVideoProperty() == nullptr,
                      "Play reports a deferred missing file and remains closed "
                      "(CNA-DEMOS-ISSUE-101)");

            Video mismatch(mediaPath.string(), &getGraphicsDeviceProperty(), 1000, 159, 90,
                           10.0f, VideoSoundtrackType::MusicAndDialog);
            ctx.Check(mismatch.getVideoSoundtrackTypeProperty() == VideoSoundtrackType::MusicAndDialog,
                      "trusted metadata retains the combined music-and-dialog classification");
            VideoPlayer mismatchPlayer;
            ctx.Check(ThrowsExactly<System::InvalidOperationException>([&] {
                          mismatchPlayer.Play(&mismatch);
                      }) &&
                          mismatchPlayer.getStateProperty() == MediaState::Stopped &&
                          mismatchPlayer.getVideoProperty() == nullptr,
                      "Play rejects trusted dimensions that disagree with the decoded stream atomically");

            mismatchPlayer.Dispose();
            missingPlayer.Dispose();
            if (ctx.Automated()) Exit();
        }
    };
}

CNA_DEMOS_GAME(UriAndMetadataDemo, {
    .id = "media.video.uri_and_metadata",
    .title = "Constructing videos from URIs and metadata",
    .description = "Exercises raw and trusted Video construction, local URI resolution, deferred missing-file "
                   "reporting and decoded-metadata validation.",
    .apis = {"Video::Video", "Video::FromUriEXT", "Video::Width", "Video::Height",
             "Video::FramesPerSecond", "Video::Duration", "Video::VideoSoundtrackType",
             "VideoSoundtrackType::Dialog", "VideoSoundtrackType::MusicAndDialog",
             "VideoPlayer::Play", "VideoPlayer::State", "VideoPlayer::Video"},
    .tags = {"media", "video", "uri", "metadata", "validation"},
    .needs = CnaDemos::Need::Video,
    .frames = 1,
    .assets = {"media/videos/track_matrix.mkv"},
    .restrictions = "Uses FNA's public URI and deferred-load contracts. All observations are local path, metadata, "
                    "exception and state checks; no network request, audio, screenshot or visual judgment is used.",
    .knownIssue = "CNA-DEMOS-ISSUE-101",
})
