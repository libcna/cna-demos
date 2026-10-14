// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_media_metadata",
    .title = "CNB Song and Video metadata in C",
    .description = "A renderer-free C program round-trips Song and Video metadata documents and "
                   "their single external streaming references.",
    .apis = {"cna_cnb_encode_song", "cna_cnb_decode_song_duration_milliseconds",
             "cna_cnb_decode_song_stream_reference_size",
             "cna_cnb_decode_song_stream_reference", "cna_cnb_decode_song_name_size",
             "cna_cnb_decode_song_name", "cna_cnb_encode_video", "cna_cnb_decode_video",
             "cna_cnb_decode_video_stream_reference_size",
             "cna_cnb_decode_video_stream_reference"},
    .tags = {"c", "binding", "content", "cnb", "song", "video", "metadata"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses synthetic metadata and logical stream references. No media files, "
                    "ContentManager, decoder, audio/video device, playback or rendering is used.",
})
