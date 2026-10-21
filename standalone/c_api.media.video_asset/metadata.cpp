// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.media.video_asset",
    .title = "Video asset metadata and ownership in C",
    .description = "A generated WMV exercises probing and declared video construction, metadata, paths, "
                   "track selection, URI-factory behavior and owned lifetime through the C ABI.",
    .apis = {"cna_video_info_init", "cna_video_create", "cna_video_create_with_metadata",
             "cna_video_create_from_uri_ext", "cna_video_get_width", "cna_video_get_height",
             "cna_video_get_frames_per_second", "cna_video_get_info",
             "cna_video_get_soundtrack_type", "cna_video_get_duration", "cna_video_set_duration",
             "cna_video_set_audio_track_ext", "cna_video_set_video_track_ext",
             "cna_video_get_file_name_size", "cna_video_copy_file_name",
             "cna_video_get_has_graphics_device", "cna_video_get_type_name_size",
             "cna_video_copy_type_name", "cna_video_destroy", "CNA_VideoInfo",
             "cna_video_player_create", "cna_video_player_play", "cna_video_player_get_state", "cna_video_player_get_video", "cna_video_player_destroy",
             "CNA_VideoSoundtrackType"},
    .tags = {"c", "binding", "media", "video", "metadata", "file", "ownership"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Video,
    .frames = 1,
    .deterministic = true,
    .assets = {"media/videos/bounce.wmv"},
    .restrictions = "Probes the generated WMV, resolves local URIs and checks deferred missing-file Play refusal. Track preferences and stopped-state cleanup are checked; no successful player decoding, audible output or visual result is claimed.",
})
