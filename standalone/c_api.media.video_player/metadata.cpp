// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.media.video_player",
    .title = "Decoded video playback in C",
    .description = "A C game drives a real decoded VideoPlayer through configuration, borrowed Texture2D type "
                   "identity, pause and resume, an observed loop, replay generation and disposed lifetime behavior.",
    .apis = {"cna_video_player_create", "cna_video_player_get_is_disposed",
             "cna_video_player_get_is_looped", "cna_video_player_set_is_looped",
             "cna_video_player_get_is_muted", "cna_video_player_set_is_muted",
             "cna_video_player_get_play_position_ticks", "cna_video_player_get_state",
             "cna_video_player_get_video", "cna_video_player_get_volume",
             "cna_video_player_set_volume", "cna_video_player_get_frame_ext",
             "cna_video_player_get_texture", "cna_video_player_play", "cna_video_player_stop",
             "cna_video_player_pause", "cna_video_player_resume",
             "cna_video_player_set_audio_track_ext", "cna_video_player_set_video_track_ext",
             "cna_video_player_dispose", "cna_video_player_destroy",
             "cna_video_player_get_type_name_size", "cna_video_player_copy_type_name",
             "cna_texture2d_get_type_name_byte_count", "cna_texture2d_copy_type_name",
             "CNA_VideoFrameEXT", "CNA_MediaState"},
    .tags = {"c", "binding", "media", "video", "player", "decode", "lifetime"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Video | CnaDemos::Need::Audio,
    .frames = 360,
    .assets = {"media/videos/bounce.wmv"},
    .restrictions = "Uses CNA's real optional video decoder and waits for the authored three-second clip to loop. "
                    "Texture-handle validity and frame generations are checked, but pixels and audible output are "
                    "not judged visually or acoustically.",
})
