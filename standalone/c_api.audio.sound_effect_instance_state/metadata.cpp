// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.sound_effect_instance_state",
    .title = "SoundEffectInstance controls in C",
    .description = "A C game configures an instance's volume, range-checked pitch, pan and looping, then "
                   "observes Play, Pause, Resume, authored release, replay and immediate Stop "
                   "through versioned state snapshots.",
    .apis = {"cna_audio_get_capabilities", "cna_sound_effect_create_pcm16",
             "cna_sound_effect_create_instance", "cna_sound_effect_instance_get_info",
             "cna_sound_effect_instance_get_is_disposed",
             "cna_sound_effect_instance_set_volume", "cna_sound_effect_instance_set_pitch",
             "cna_sound_effect_instance_set_pan", "cna_sound_effect_instance_set_is_looped",
             "cna_sound_effect_instance_play", "cna_sound_effect_instance_pause",
             "cna_sound_effect_instance_resume", "cna_sound_effect_instance_stop",
             "cna_sound_effect_instance_destroy", "cna_sound_effect_destroy",
             "CNA_SoundEffectInstanceInfo", "CNA_AudioStopOptions"},
    .tags = {"c", "binding", "audio", "sound-effect-instance", "playback", "loop"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 120,
    .restrictions = "Standalone C program using SDL dummy audio during automation. It validates "
                    "control state and bounded completion, not samples, exact latency or audible "
                    "quality. Built only when CNA_BUILD_C_API=ON.",
})
