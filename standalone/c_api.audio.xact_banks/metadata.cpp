// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.xact_banks",
    .title = "XACT banks and categories in C",
    .description = "A C program opens resident and streaming wave banks plus a sound bank, then observes category "
                   "controls through live Music/SFX cues, fire-and-forget playback and bank ownership.",
    .apis = {"cna_audio_engine_get_category", "cna_audio_category_destroy",
             "cna_audio_category_get_name_size", "cna_audio_category_copy_name",
             "cna_audio_category_pause", "cna_audio_category_resume",
             "cna_audio_category_set_volume", "cna_audio_category_stop",
             "cna_audio_category_equals", "cna_audio_category_get_hash_code",
             "cna_wave_bank_create", "cna_wave_bank_create_streaming", "cna_wave_bank_destroy",
             "cna_wave_bank_get_is_disposed", "cna_wave_bank_get_is_prepared",
             "cna_wave_bank_get_is_in_use", "cna_wave_bank_subscribe_disposing_ext",
             "cna_sound_bank_create", "cna_sound_bank_destroy",
             "cna_sound_bank_get_is_disposed", "cna_sound_bank_get_is_in_use",
             "cna_sound_bank_get_cue", "cna_sound_bank_play_cue",
             "cna_sound_bank_play_cue_3d", "cna_sound_bank_subscribe_disposing_ext",
             "cna_cue_get_info", "cna_cue_play", "cna_cue_destroy",
             "cna_audio_engine_create", "cna_audio_engine_update", "cna_audio_engine_destroy",
             "cna_audio_listener_init", "cna_audio_emitter_init",
             "cna_audio_unsubscribe_ext", "cna_title_location_copy_path"},
    .tags = {"c", "binding", "audio", "xact", "bank", "category", "ownership"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 1,
    .deterministic = true,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb", "xact/music.xwb"},
    .restrictions = "Uses generated XACT banks and a dummy audio device in automation. Category state, bank use "
                    "and ownership are observed through cues; volume gain, spatial speaker output and audible "
                    "quality are not measured.",
})
