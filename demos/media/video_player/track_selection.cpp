// SPDX-License-Identifier: MS-PL
// FNA/CNA extensions for selecting streams in a multi-track video container.
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"
#include "Microsoft/Xna/Framework/Media/Video/Video.hpp"
#include "Microsoft/Xna/Framework/Media/Video/VideoPlayer.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    bool HasDominantColour(Texture2D* texture, bool blue)
    {
        if (texture == nullptr) return false;
        std::vector<Color> pixels(
            static_cast<std::size_t>(texture->getWidthProperty() * texture->getHeightProperty()));
        texture->GetData(pixels.data(), static_cast<int>(pixels.size()));
        const Color sample = pixels[pixels.size() / 2];
        return blue ? sample.getBProperty() > sample.getRProperty() + 100
                    : sample.getRProperty() > sample.getBProperty() + 100;
    }

    class TrackSelectionDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            const std::string path =
                (std::filesystem::path(TitleLocation::Path()) /
                 "assets/media/videos/track_matrix.mkv").string();

            Video video(path, &getGraphicsDeviceProperty());
            ctx.Check(video.getWidthProperty() == 160 && video.getHeightProperty() == 90 &&
                          video.getFramesPerSecondProperty() == 10.0f &&
                          video.getDurationProperty().getTotalSecondsProperty() > 0.9,
                      "raw probing reports the default 160x90, 10 fps stream");

            VideoPlayer player;
            player.SetVideoTrackEXT(1);
            player.SetAudioTrackEXT(1);
            player.Play(&video);
            Texture2D* selected = player.GetTexture();
            ctx.Check(player.getStateProperty() == MediaState::Playing &&
                          player.getVideoProperty() == &video && selected != nullptr &&
                          selected->getWidthProperty() == 160 && selected->getHeightProperty() == 90 &&
                          HasDominantColour(selected, true),
                      "player preferences set before Play select the blue video stream");

            player.SetVideoTrackEXT(1);
            ctx.Check(player.GetTexture() == selected,
                      "reselecting the active video stream preserves the frame texture");
            player.SetVideoTrackEXT(99);
            ctx.Check(player.GetTexture() == selected,
                      "an out-of-range video stream is a no-op that preserves the frame texture");

            player.SetAudioTrackEXT(0);
            ctx.Check(player.GetTexture() == selected && player.getStateProperty() == MediaState::Playing,
                      "switching between real audio streams leaves video texture and transport intact");
            player.SetAudioTrackEXT(0);
            player.SetAudioTrackEXT(99);
            ctx.Check(player.GetTexture() == selected,
                      "same and out-of-range audio selections leave the unrelated texture intact");

            const std::uint64_t selectedGeneration = player.GetFrameGenerationEXT();
            player.Stop();
            player.SetVideoTrackEXT(-1);
            player.SetAudioTrackEXT(-1);
            player.Play(&video);
            Texture2D* defaultFrame = player.GetTexture();
            ctx.Check(defaultFrame != nullptr && defaultFrame->getWidthProperty() == 160 &&
                          defaultFrame->getHeightProperty() == 90 && HasDominantColour(defaultFrame, false) &&
                          player.GetFrameGenerationEXT() > selectedGeneration,
                      "the -1 sentinel restores the default red stream on replay without restarting generation");

            video.SetVideoTrackEXT(1);
            video.SetAudioTrackEXT(1);
            ctx.Check(player.GetTexture() != nullptr && player.getStateProperty() == MediaState::Playing,
                      "track selection on an attached Video forwards without stopping its active player");
            player.Stop();

            Video detached(path, &getGraphicsDeviceProperty());
            detached.SetVideoTrackEXT(1);
            detached.SetAudioTrackEXT(1);
            VideoPlayer detachedPlayer;
            detachedPlayer.Play(&detached);
            Texture2D* detachedFrame = detachedPlayer.GetTexture();
            const bool detachedBlue = HasDominantColour(detachedFrame, true);
            const bool detachedRed = HasDominantColour(detachedFrame, false);
            ctx.Print("detached Video preference decoded the {} stream",
                      detachedBlue ? "blue" : detachedRed ? "red" : "unidentified");
            ctx.Check(detachedFrame != nullptr && detachedFrame->getWidthProperty() == 160 &&
                          detachedFrame->getHeightProperty() == 90 && detachedBlue && !detachedRed,
                      "a Video-level preference set before Play selects its requested stream "
                      "(CNA-DEMOS-ISSUE-100)");
            detachedPlayer.Stop();
            detachedPlayer.Dispose();
            player.Dispose();

            if (ctx.Automated()) Exit();
        }
    };
}

CNA_DEMOS_GAME(TrackSelectionDemo, {
    .id = "media.video_player.track_selection",
    .title = "Selecting video tracks",
    .description = "Selects authored video and audio streams before and during playback, including no-op, "
                   "forwarding and lifetime-monotonic frame behavior.",
    .apis = {"Video::SetAudioTrackEXT", "Video::SetVideoTrackEXT", "VideoPlayer::SetAudioTrackEXT",
             "VideoPlayer::SetVideoTrackEXT", "VideoPlayer::Play", "VideoPlayer::Stop",
             "VideoPlayer::GetTexture", "VideoPlayer::GetFrameGenerationEXT"},
    .tags = {"media", "video", "tracks", "cnaext"},
    .needs = CnaDemos::Need::Video | CnaDemos::Need::Audio,
    .frames = 1,
    .assets = {"media/videos/track_matrix.mkv"},
    .restrictions = "The generated MKV has two visible video streams and two audio streams. Public texture dimensions "
                    "and pixels identify video selection; audio checks establish call-path isolation only, not audible "
                    "content or sample-rate selection.",
})
