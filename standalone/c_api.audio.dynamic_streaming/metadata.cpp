// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.dynamic_streaming",
    .title = "Dynamic audio streaming in C",
    .description = "A C program feeds copied integer and float sample windows to dynamic sound-effect "
                   "instances, exercises queue ownership and format transitions, and drives BufferNeeded "
                   "through the explicit maintenance route.",
    .apis = {"cna_audio_get_capabilities", "cna_dynamic_sound_effect_instance_create",
             "cna_dynamic_sound_effect_instance_get_pending_buffer_count",
             "cna_dynamic_sound_effect_instance_submit_buffer",
             "cna_dynamic_sound_effect_instance_submit_float_buffer_ext",
             "cna_dynamic_sound_effect_instance_queue_initial_buffers_ext",
             "cna_dynamic_sound_effect_instance_clear_buffers_ext",
             "cna_dynamic_sound_effect_instance_update_ext",
             "cna_dynamic_sound_effect_instance_get_sample_duration_ticks",
             "cna_dynamic_sound_effect_instance_get_sample_size_in_bytes",
             "cna_dynamic_sound_effect_instance_subscribe_buffer_needed",
             "cna_audio_unsubscribe_ext", "cna_sound_effect_instance_get_info",
             "cna_sound_effect_instance_play", "cna_sound_effect_instance_stop",
             "cna_sound_effect_instance_destroy"},
    .tags = {"c", "binding", "audio", "streaming", "buffer", "event", "cnaext"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 1,
    .restrictions = "Standalone C program using SDL dummy audio during automation. It checks public queue, "
                    "format and event state without claiming audible content, mixer latency or output quality. "
                    "The fixed reflection type-name pair remains trivial. Built only when CNA_BUILD_C_API=ON.",
})
