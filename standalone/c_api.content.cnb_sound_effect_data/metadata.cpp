// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_sound_effect_data",
    .title = "CNB SoundEffect data and codec in C",
    .description = "A renderer-free C program inspects CNB audio formats and round-trips copied "
                   "PCM8 samples through a complete SoundEffect document.",
    .apis = {"cna_cnb_audio_frame_bytes", "cna_cnb_get_audio_format_name_size",
             "cna_cnb_copy_audio_format_name", "cna_cnb_sound_effect_data_create",
             "cna_cnb_sound_effect_data_destroy", "cna_cnb_sound_effect_data_get_info",
             "cna_cnb_sound_effect_data_copy_samples", "cna_cnb_encode_sound_effect",
             "cna_cnb_decode_sound_effect"},
    .tags = {"c", "binding", "content", "cnb", "sound-effect", "audio", "pcm"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses synthetic PCM8 bytes and neutral CNB data. No SoundEffect runtime "
                    "object, audio device, playback or audible-output claim is involved.",
})
