// SPDX-License-Identifier: MS-PL
// ISSUE-101 follow-up: an existing non-media file reaches the real decoder-open failure.
// Provisional FNA expectation only: do not register before genuine XNA codec-error review.
#include <gtest/gtest.h>
#include <typeinfo>
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Media/Video/Video.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/InvalidOperationException.hpp"
#include "VideoPlayerTestAccess.hpp"

namespace
{
using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
using namespace Microsoft::Xna::Framework::Media;

void ExpectDecoderOpenRefusal(VideoPlayer& player, Video& video)
{
    bool exactRefusal = false;
    try { player.Play(&video); }
    catch (const System::IO::FileNotFoundException& error)
    { exactRefusal = typeid(error) == typeid(System::IO::FileNotFoundException); }
    catch (...) { }
    EXPECT_TRUE(exactRefusal);
    EXPECT_EQ(player.getStateProperty(), MediaState::Stopped);
    EXPECT_EQ(player.getVideoProperty(), nullptr);
    EXPECT_EQ(VideoPlayerTestAccess::GetDecoderWidth(player), -1);
    EXPECT_FALSE(VideoPlayerTestAccess::HasFrameTexture(player));
    EXPECT_FALSE(VideoPlayerTestAccess::HasAudioStream(player));
    EXPECT_EQ(VideoPlayerTestAccess::GetAudioBufferSize(player), 0u);
    EXPECT_THROW((void)player.GetTexture(), System::InvalidOperationException);
}

TEST(VideoDeferredOpenFailureTest, ExistingNonMediaFileReportsActualDecoderOpenFailure)
{
    // Read an existing text source, without writing fixtures or racing deletion.
    const std::string path = "modules/media/src/Xna/Video/VideoPlayer.cpp";
    ASSERT_TRUE(System::IO::File::Exists(path));
    GraphicsDevice device;
    Video video(path, &device, 2000, 160, 90, 25.0f, VideoSoundtrackType::Dialog);
    VideoPlayer player;
    ExpectDecoderOpenRefusal(player, video);
    Video valid("tests/assets/media/video/chroma_420.mkv", &device);
    ASSERT_NO_THROW(player.Play(&valid));
    EXPECT_EQ(player.getStateProperty(), MediaState::Playing);
    EXPECT_EQ(player.getVideoProperty(), &valid);
}

TEST(VideoDeferredOpenFailureTest, FailedReplacementClosesOldResourcesAndRemainsReusable)
{
    const std::string path = "modules/media/src/Xna/Video/VideoPlayer.cpp";
    ASSERT_TRUE(System::IO::File::Exists(path));
    GraphicsDevice device;
    Video valid("tests/assets/media/video/chroma_420.mkv", &device);
    Video invalid(path, &device, 2000, 160, 90, 25.0f, VideoSoundtrackType::Dialog);
    VideoPlayer player;
    player.Play(&valid);
    ASSERT_EQ(player.getStateProperty(), MediaState::Playing);
    ASSERT_TRUE(VideoPlayerTestAccess::HasFrameTexture(player));
    ExpectDecoderOpenRefusal(player, invalid);
    ASSERT_NO_THROW(player.Play(&valid));
    EXPECT_EQ(player.getStateProperty(), MediaState::Playing);
    EXPECT_EQ(player.getVideoProperty(), &valid);
}
} // namespace
