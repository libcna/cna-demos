// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.microphone_availability",
    .title = "Microphone availability in C",
    .description = "A C program reports the host microphone count and default, captures one bounded "
                   "buffer when a device is available, restores its duration setting, and verifies "
                   "that the first index beyond the snapshot is refused consistently.",
    .apis = {"cna_microphone_get_count", "cna_microphone_get_default_index_ext",
             "cna_microphone_get_name_size_at", "cna_microphone_copy_name_at",
             "cna_microphone_get_buffer_duration_ticks_at",
             "cna_microphone_set_buffer_duration_ticks_at", "cna_microphone_get_is_headset_at",
             "cna_microphone_get_sample_rate_at", "cna_microphone_get_state_at",
             "cna_microphone_start_at", "cna_microphone_stop_at", "cna_microphone_get_data_at",
             "cna_microphone_get_sample_duration_ticks_at",
             "cna_microphone_get_sample_size_in_bytes_at",
             "cna_microphone_subscribe_buffer_ready_at", "cna_microphone_check_all_buffers_ext",
             "cna_audio_unsubscribe_ext", "CNA_MicrophoneState"},
    .tags = {"c", "binding", "audio", "microphone", "capture", "availability"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 1,
    .restrictions = "Standalone C program. Automated capture uses the selected audio backend's device (SDL dummy "
                    "in CI), so it validates API state, buffering and delivery without claiming a physical microphone, "
                    "audible content, input quality or callback latency. Fixed type-name formatting is trivial.",
})
