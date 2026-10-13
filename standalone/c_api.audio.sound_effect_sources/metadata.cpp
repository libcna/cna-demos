// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.sound_effect_sources",
    .title = "SoundEffect sources and state in C",
    .description = "A C program constructs owned sounds from a ranged PCM buffer, an in-memory "
                   "WAV and a private WAV file; verifies duration and name ownership, restores "
                   "process master volume, converts sample sizes, and uses both one-shot Play forms.",
    .apis = {"cna_audio_get_capabilities", "cna_sound_effect_create_pcm16_range_ext",
             "cna_sound_effect_create_from_encoded_ext", "cna_sound_effect_create_from_asset_ext",
             "cna_sound_effect_get_duration_ticks", "cna_sound_effect_get_is_disposed",
             "cna_sound_effect_get_name_size", "cna_sound_effect_copy_name",
             "cna_sound_effect_set_name", "cna_sound_effect_get_master_volume",
             "cna_sound_effect_set_master_volume", "cna_sound_effect_play",
             "cna_sound_effect_play_with_settings",
             "cna_sound_effect_get_sample_duration_ticks",
             "cna_sound_effect_get_sample_size_in_bytes", "cna_sound_effect_destroy",
             "CNA_SoundEffectCreateInfo"},
    .tags = {"c", "binding", "audio", "sound-effect", "pcm", "wav", "ownership"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 1,
    .restrictions = "Standalone C program using a generated silent WAV and SDL dummy audio during "
                    "automation. It validates API state and ownership, not audible content or "
                    "output quality. Built only when CNA_BUILD_C_API=ON.",
})
