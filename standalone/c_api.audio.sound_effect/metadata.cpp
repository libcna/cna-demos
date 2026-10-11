// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.sound_effect",
    .title = "Sound in C: a SoundEffect from PCM",
    .description = "A C program asks whether audio playback is available, creates a SoundEffect from half a second "
                   "of 16-bit PCM it computes, plays an instance of it and follows the instance's state from the "
                   "update callback until the sound stops by itself; verifies refused parent destruction and released-handle rejection.",
    .apis = {"cna_audio_get_capabilities", "cna_sound_effect_create_pcm16", "cna_sound_effect_get_duration_ticks",
             "cna_sound_effect_create_instance", "cna_sound_effect_instance_set_volume",
             "cna_sound_effect_instance_play", "cna_sound_effect_instance_get_info",
             "cna_sound_effect_instance_destroy", "cna_sound_effect_destroy", "CNA_SoundEffectCreateInfo",
             "CNA_SoundEffectInstanceInfo"},
    .tags = {"c", "binding", "audio", "sound"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 120,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
