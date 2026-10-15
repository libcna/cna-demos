// SPDX-License-Identifier: MS-PL
// Exercise the shared Song/Video CNB metadata-and-stream-reference codec.
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbDocument.hpp"
#include "CNA/Content/Cnb/CnbMediaCodec.hpp"
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

    Cnb::CnbExternalReference MediaReference(
        std::string name,
        const std::uint32_t expectedType = Cnb::CnbAssetTypeId::Invalid)
    {
        Cnb::CnbExternalReference reference;
        reference.expectedAssetTypeId = expectedType;
        reference.logicalName = std::move(name);
        return reference;
    }

    std::vector<std::uint8_t> BuildRawSong(
        const Cnb::CnbSongData& song,
        std::vector<Cnb::CnbExternalReference> references,
        const std::uint32_t assetType = Cnb::CnbAssetTypeId::Song,
        const std::uint32_t schemaVersion = Cnb::CnbMediaSchemaVersion,
        const std::uint32_t headerFlags = 0u,
        const std::size_t headerCount = 1u)
    {
        Cnb::CnbByteWriter header;
        header.WriteU32(song.durationMs);
        header.WriteU32(headerFlags);
        header.WriteString(song.name);
        const std::vector<std::uint8_t> headerBytes = header.Take();

        Cnb::CnbWriter writer(assetType, schemaVersion);
        writer.SetMetadata("Microsoft.Xna.Framework.Media.Song", "songs/raw");
        writer.SetExternalReferences(std::move(references));
        for (std::size_t index = 0; index < headerCount; ++index)
        {
            writer.AddChunk(Cnb::CnbMediaChunk::SongHeader, headerBytes,
                            Cnb::CnbChunkFlags::Mandatory, 4u);
        }
        return writer.Build();
    }

    std::vector<std::uint8_t> BuildRawVideo(
        const Cnb::CnbVideoData& video,
        std::vector<Cnb::CnbExternalReference> references,
        const std::uint32_t assetType = Cnb::CnbAssetTypeId::Video,
        const std::uint32_t schemaVersion = Cnb::CnbMediaSchemaVersion,
        const std::uint32_t headerFlags = 0u,
        const std::size_t headerCount = 1u)
    {
        Cnb::CnbByteWriter header;
        header.WriteU32(video.durationMs);
        header.WriteU32(video.width);
        header.WriteU32(video.height);
        header.WriteF32(video.framesPerSecond);
        header.WriteU32(video.soundtrackType);
        header.WriteU32(headerFlags);
        const std::vector<std::uint8_t> headerBytes = header.Take();

        Cnb::CnbWriter writer(assetType, schemaVersion);
        writer.SetMetadata("Microsoft.Xna.Framework.Media.Video", "videos/raw");
        writer.SetExternalReferences(std::move(references));
        for (std::size_t index = 0; index < headerCount; ++index)
        {
            writer.AddChunk(Cnb::CnbMediaChunk::VideoHeader, headerBytes,
                            Cnb::CnbChunkFlags::Mandatory, 4u);
        }
        return writer.Build();
    }

    void CnbMediaCodec(CnaDemos::DemoContext& ctx)
    {
        const Cnb::CnbSongData songDefaults;
        const Cnb::CnbVideoData videoDefaults;
        ctx.Check(Cnb::CnbMediaSchemaVersion == 1u &&
                      Cnb::CnbSongHeaderFixedStride == 8u &&
                      Cnb::CnbVideoHeaderStride == 24u &&
                      Cnb::CnbMaxVideoDimension == 65536u &&
                      Cnb::CnbMediaChunk::SongHeader == Cnb::MakeChunkId('S', 'N', 'G', 'H') &&
                      Cnb::CnbMediaChunk::VideoHeader == Cnb::MakeChunkId('V', 'I', 'D', 'H') &&
                      songDefaults.streamReference.empty() && songDefaults.name.empty() &&
                      songDefaults.durationMs == 0u && videoDefaults.streamReference.empty() &&
                      videoDefaults.durationMs == 0u && videoDefaults.width == 1u &&
                      videoDefaults.height == 1u && videoDefaults.framesPerSecond == 30.0f &&
                      videoDefaults.soundtrackType == 0u,
                  "the shared media schema constants and both metadata values have safe defaults");

        Cnb::CnbSongData song;
        song.streamReference = "Music/theme.ogg";
        song.name = "Main Theme";
        song.durationMs = 185000u;
        const auto encodedSong = Cnb::EncodeSongToCnb(song, "Music/theme");
        const auto encodedSongAgain = Cnb::EncodeSongToCnb(song, "Music/theme");
        const Cnb::CnbDocument songDocument =
            Cnb::CnbDocument::Parse(encodedSong, "theme.cnb");
        const Cnb::CnbSongData decodedSong = Cnb::DecodeSongFromCnb(songDocument);
        const auto& songReferences = songDocument.ExternalReferences();
        ctx.Check(encodedSong == encodedSongAgain &&
                      songDocument.AssetTypeId() == Cnb::CnbAssetTypeId::Song &&
                      songDocument.AssetSchemaVersion() == Cnb::CnbMediaSchemaVersion &&
                      songDocument.Metadata().assetTypeName ==
                          "Microsoft.Xna.Framework.Media.Song" &&
                      songDocument.Metadata().contentName == "Music/theme" &&
                      songDocument.FindAll(Cnb::CnbMediaChunk::SongHeader).size() == 1u &&
                      songReferences.size() == 1u && songReferences[0].flags == 0u &&
                      songReferences[0].expectedAssetTypeId == Cnb::CnbAssetTypeId::Invalid &&
                      songReferences[0].logicalName == song.streamReference &&
                      decodedSong.streamReference == song.streamReference &&
                      decodedSong.name == song.name && decodedSong.durationMs == song.durationMs,
                  "a Song round trip is deterministic and exposes its one untyped media XREF");

        Cnb::CnbSongData invalidSong = song;
        invalidSong.streamReference.clear();
        const bool emptySongReference =
            RefusesContent([&] { (void)Cnb::EncodeSongToCnb(invalidSong); });
        invalidSong = song;
        invalidSong.streamReference = "../escape.ogg";
        const bool escapingSongReference =
            RefusesContent([&] { (void)Cnb::EncodeSongToCnb(invalidSong); });
        invalidSong = song;
        invalidSong.streamReference = "/absolute.ogg";
        const bool absoluteSongReference =
            RefusesContent([&] { (void)Cnb::EncodeSongToCnb(invalidSong); });
        invalidSong = song;
        invalidSong.streamReference = "Music\\theme.ogg";
        const bool nativeSeparator =
            RefusesContent([&] { (void)Cnb::EncodeSongToCnb(invalidSong); });
        invalidSong = song;
        invalidSong.durationMs = 0x80000000u;
        const bool songDurationOverflow =
            RefusesContent([&] { (void)Cnb::EncodeSongToCnb(invalidSong); });
        ctx.Check(emptySongReference && escapingSongReference && absoluteSongReference &&
                      nativeSeparator && songDurationOverflow,
                  "Song encoding requires a safe root-relative stream and signed-runtime duration");

        const auto decodeSong = [&](std::vector<std::uint8_t> bytes)
        {
            return Cnb::DecodeSongFromCnb(Cnb::CnbDocument::Parse(std::move(bytes), "raw-song.cnb"));
        };
        const bool songWrongAsset = RefusesContent([&]
        {
            (void)decodeSong(BuildRawSong(song, {MediaReference(song.streamReference)},
                                         Cnb::CnbAssetTypeId::Video));
        });
        const bool songWrongSchema = RefusesContent([&]
        {
            (void)decodeSong(BuildRawSong(song, {MediaReference(song.streamReference)},
                                         Cnb::CnbAssetTypeId::Song, 2u));
        });
        const bool songMissingHeader = RefusesContent([&]
        {
            (void)decodeSong(BuildRawSong(song, {MediaReference(song.streamReference)},
                                         Cnb::CnbAssetTypeId::Song, 1u, 0u, 0u));
        });
        const bool songDuplicateHeader = RefusesContent([&]
        {
            (void)decodeSong(BuildRawSong(song, {MediaReference(song.streamReference)},
                                         Cnb::CnbAssetTypeId::Song, 1u, 0u, 2u));
        });
        const bool songNoReference = RefusesContent([&]
        {
            (void)decodeSong(BuildRawSong(song, {}));
        });
        const bool songTwoReferences = RefusesContent([&]
        {
            (void)decodeSong(BuildRawSong(song, {MediaReference("one.ogg"),
                                                 MediaReference("two.ogg")}));
        });
        const bool songTypedReference = RefusesContent([&]
        {
            (void)decodeSong(BuildRawSong(song, {MediaReference(
                song.streamReference, Cnb::CnbAssetTypeId::SoundEffect)}));
        });
        const bool songReservedFlags = RefusesContent([&]
        {
            (void)decodeSong(BuildRawSong(song, {MediaReference(song.streamReference)},
                                         Cnb::CnbAssetTypeId::Song, 1u, 1u));
        });
        ctx.Check(songWrongAsset && songWrongSchema && songMissingHeader && songDuplicateHeader &&
                      songNoReference && songTwoReferences && songTypedReference && songReservedFlags,
                  "Song decoding enforces identity, schema, singleton header, one untyped XREF, and flags");

        Cnb::CnbVideoData video;
        video.streamReference = "Movies/intro.mp4";
        video.durationMs = 42000u;
        video.width = 1920u;
        video.height = 1080u;
        video.framesPerSecond = 29.97f;
        video.soundtrackType = 2u;
        const auto encodedVideo = Cnb::EncodeVideoToCnb(video, "Movies/intro");
        const auto encodedVideoAgain = Cnb::EncodeVideoToCnb(video, "Movies/intro");
        const Cnb::CnbDocument videoDocument =
            Cnb::CnbDocument::Parse(encodedVideo, "intro.cnb");
        const Cnb::CnbVideoData decodedVideo = Cnb::DecodeVideoFromCnb(videoDocument);
        const auto videoHeader = videoDocument.RequireSingle(Cnb::CnbMediaChunk::VideoHeader);
        const auto& videoReferences = videoDocument.ExternalReferences();
        ctx.Check(encodedVideo == encodedVideoAgain &&
                      videoDocument.AssetTypeId() == Cnb::CnbAssetTypeId::Video &&
                      videoDocument.Metadata().assetTypeName ==
                          "Microsoft.Xna.Framework.Media.Video" &&
                      videoDocument.Metadata().contentName == "Movies/intro" &&
                      videoDocument.ChunkData(videoHeader).size() == Cnb::CnbVideoHeaderStride &&
                      videoReferences.size() == 1u && videoReferences[0].flags == 0u &&
                      videoReferences[0].expectedAssetTypeId == Cnb::CnbAssetTypeId::Invalid &&
                      videoReferences[0].logicalName == video.streamReference &&
                      decodedVideo.streamReference == video.streamReference &&
                      decodedVideo.durationMs == video.durationMs &&
                      decodedVideo.width == video.width && decodedVideo.height == video.height &&
                      decodedVideo.framesPerSecond == video.framesPerSecond &&
                      decodedVideo.soundtrackType == video.soundtrackType,
                  "a Video round trip is deterministic and preserves every field plus its media XREF");

        bool dimensionBoundaries = true;
        for (const std::uint32_t dimension : {1u, Cnb::CnbMaxVideoDimension})
        {
            Cnb::CnbVideoData boundary = video;
            boundary.width = dimension;
            boundary.height = dimension;
            const auto decoded = Cnb::DecodeVideoFromCnb(Cnb::CnbDocument::Parse(
                Cnb::EncodeVideoToCnb(boundary), "dimension.cnb"));
            dimensionBoundaries = dimensionBoundaries && decoded.width == dimension &&
                decoded.height == dimension;
        }
        bool soundtrackAndDurationBoundaries = true;
        for (const std::uint32_t soundtrack : {0u, 1u, 2u})
        {
            Cnb::CnbVideoData boundary = video;
            boundary.durationMs = 0x7FFFFFFFu;
            boundary.soundtrackType = soundtrack;
            const auto decoded = Cnb::DecodeVideoFromCnb(Cnb::CnbDocument::Parse(
                Cnb::EncodeVideoToCnb(boundary), "enum.cnb"));
            soundtrackAndDurationBoundaries = soundtrackAndDurationBoundaries &&
                decoded.durationMs == 0x7FFFFFFFu && decoded.soundtrackType == soundtrack;
        }
        ctx.Check(dimensionBoundaries && soundtrackAndDurationBoundaries,
                  "Video accepts both dimension limits, every soundtrack value, and maximum duration");

        const auto videoEncodingRefused = [&](const Cnb::CnbVideoData& candidate)
        {
            return RefusesContent([&] { (void)Cnb::EncodeVideoToCnb(candidate); });
        };
        Cnb::CnbVideoData invalidVideo = video;
        invalidVideo.streamReference.clear();
        const bool videoNoReference = videoEncodingRefused(invalidVideo);
        invalidVideo = video;
        invalidVideo.durationMs = 0x80000000u;
        const bool videoDurationOverflow = videoEncodingRefused(invalidVideo);
        invalidVideo = video;
        invalidVideo.width = 0u;
        const bool zeroWidth = videoEncodingRefused(invalidVideo);
        invalidVideo = video;
        invalidVideo.height = Cnb::CnbMaxVideoDimension + 1u;
        const bool highHeight = videoEncodingRefused(invalidVideo);
        invalidVideo = video;
        invalidVideo.framesPerSecond = 0.0f;
        const bool zeroRate = videoEncodingRefused(invalidVideo);
        invalidVideo.framesPerSecond = -1.0f;
        const bool negativeRate = videoEncodingRefused(invalidVideo);
        invalidVideo.framesPerSecond = std::numeric_limits<float>::quiet_NaN();
        const bool nanRate = videoEncodingRefused(invalidVideo);
        invalidVideo.framesPerSecond = std::numeric_limits<float>::infinity();
        const bool infiniteRate = videoEncodingRefused(invalidVideo);
        invalidVideo = video;
        invalidVideo.soundtrackType = 3u;
        const bool badSoundtrack = videoEncodingRefused(invalidVideo);
        ctx.Check(videoNoReference && videoDurationOverflow && zeroWidth && highHeight && zeroRate &&
                      negativeRate && nanRate && infiniteRate && badSoundtrack,
                  "Video encoding refuses unsafe streams and every out-of-range runtime field");

        const auto decodeVideo = [&](std::vector<std::uint8_t> bytes)
        {
            return Cnb::DecodeVideoFromCnb(
                Cnb::CnbDocument::Parse(std::move(bytes), "raw-video.cnb"));
        };
        invalidVideo = video;
        invalidVideo.width = 0u;
        const bool decodedWidth = RefusesContent([&]
        {
            (void)decodeVideo(BuildRawVideo(invalidVideo,
                                           {MediaReference(video.streamReference)}));
        });
        invalidVideo = video;
        invalidVideo.framesPerSecond = std::numeric_limits<float>::quiet_NaN();
        const bool decodedRate = RefusesContent([&]
        {
            (void)decodeVideo(BuildRawVideo(invalidVideo,
                                           {MediaReference(video.streamReference)}));
        });
        invalidVideo = video;
        invalidVideo.soundtrackType = 3u;
        const bool decodedSoundtrack = RefusesContent([&]
        {
            (void)decodeVideo(BuildRawVideo(invalidVideo,
                                           {MediaReference(video.streamReference)}));
        });
        const bool videoWrongAsset = RefusesContent([&]
        {
            (void)decodeVideo(BuildRawVideo(video, {MediaReference(video.streamReference)},
                                           Cnb::CnbAssetTypeId::Song));
        });
        const bool videoNoHeader = RefusesContent([&]
        {
            (void)decodeVideo(BuildRawVideo(video, {MediaReference(video.streamReference)},
                                           Cnb::CnbAssetTypeId::Video, 1u, 0u, 0u));
        });
        const bool videoNoXref = RefusesContent([&]
        {
            (void)decodeVideo(BuildRawVideo(video, {}));
        });
        const bool videoReservedFlags = RefusesContent([&]
        {
            (void)decodeVideo(BuildRawVideo(video, {MediaReference(video.streamReference)},
                                           Cnb::CnbAssetTypeId::Video, 1u, 1u));
        });
        ctx.Check(decodedWidth && decodedRate && decodedSoundtrack && videoWrongAsset &&
                      videoNoHeader && videoNoXref && videoReservedFlags,
                  "Video decoding independently enforces metadata, identity, header, XREF, and flags");
    }
}

CNA_DEMOS_CONSOLE(CnbMediaCodec, {
    .id = "content.media.cnb_codec",
    .title = "CNB Song and Video codec",
    .description = "Exercises CNA's complete public Song/Video CNB metadata contract, deterministic round trips, "
                   "external stream references, boundary values, and malformed-description refusals.",
    .apis = {"CNA::Content::Cnb::CnbMediaChunk::SongHeader",
             "CNA::Content::Cnb::CnbMediaChunk::VideoHeader",
             "CNA::Content::Cnb::CnbMediaSchemaVersion",
             "CNA::Content::Cnb::CnbSongHeaderFixedStride",
             "CNA::Content::Cnb::CnbVideoHeaderStride",
             "CNA::Content::Cnb::CnbMaxVideoDimension",
             "CNA::Content::Cnb::CnbSongData",
             "CNA::Content::Cnb::CnbVideoData",
             "CNA::Content::Cnb::EncodeSongToCnb",
             "CNA::Content::Cnb::EncodeVideoToCnb",
             "CNA::Content::Cnb::DecodeSongFromCnb",
             "CNA::Content::Cnb::DecodeVideoFromCnb"},
    .tags = {"content", "media", "song", "video", "cnb", "codec", "serialization"},
})
