// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.spatial_state",
    .title = "Spatial audio state in C",
    .description = "A C program initializes listener/emitter values, temporarily changes and "
                   "restores the process spatial scales, positions an instance before and during "
                   "playback, and exercises one- and multi-listener ordering contracts.",
    .apis = {"cna_audio_emitter_init", "cna_audio_listener_init",
             "cna_sound_effect_get_distance_scale", "cna_sound_effect_set_distance_scale",
             "cna_sound_effect_get_doppler_scale", "cna_sound_effect_set_doppler_scale",
             "cna_sound_effect_get_speed_of_sound", "cna_sound_effect_set_speed_of_sound",
             "cna_sound_effect_instance_apply_3d",
             "cna_sound_effect_instance_apply_3d_multi_ext", "CNA_AudioEmitter",
             "CNA_AudioListener"},
    .tags = {"c", "binding", "audio", "spatial", "listener", "emitter", "doppler"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 1,
    .restrictions = "Standalone C program using SDL dummy audio during automation. It validates "
                    "public values, settings and call ordering; the C ABI exposes no spatial-output "
                    "readback, so attenuation, spatial pan, Doppler and audible quality are not claimed. "
                    "One property-atomicity check retains CNA-DEMOS-ISSUE-073. "
                    "Built only when CNA_BUILD_C_API=ON.",
    .knownIssue = "CNA-DEMOS-ISSUE-073",
})
