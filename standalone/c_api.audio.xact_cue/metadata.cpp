// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.xact_cue",
    .title = "XACT cue behavior in C",
    .description = "A C program drives caller-owned XACT cues through names, independent variables, 3D values, "
                   "pause/resume, immediate and authored-release stops, and disposal notification ownership.",
    .apis = {"cna_cue_get_name_size", "cna_cue_copy_name", "cna_cue_apply_3d",
             "cna_cue_get_variable", "cna_cue_set_variable", "cna_cue_pause", "cna_cue_resume",
             "cna_cue_stop", "cna_cue_subscribe_disposing_ext", "cna_cue_get_info",
             "cna_cue_play", "cna_cue_destroy", "cna_sound_bank_get_cue",
             "cna_sound_bank_create", "cna_sound_bank_destroy", "cna_wave_bank_create",
             "cna_wave_bank_destroy", "cna_audio_engine_create", "cna_audio_engine_update",
             "cna_audio_engine_destroy", "cna_audio_listener_init", "cna_audio_emitter_init",
             "cna_audio_unsubscribe_ext"},
    .tags = {"c", "binding", "audio", "xact", "cue", "variables", "3d", "lifetime"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 1,
    .deterministic = true,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb",
               "xact/release.xwb", "xact/release.xsb"},
    .restrictions = "Uses generated XACT banks and a dummy audio device in automation. Cue state and public "
                    "spatial variables are checked; acoustic position, gain, fade shape and audible quality are "
                    "not measured.",
})
