// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.microphone_availability",
    .title = "Microphone availability in C",
    .description = "A C program reports the host microphone count/default and reads capability metadata without "
                   "opening capture. It checks sample conversion plus every safe first-index-beyond-snapshot route.",
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
    .restrictions = "Standalone C program. It never starts or reads a real microphone; real BufferReady/GetData "
                    "lifecycle coverage is the explicitly activated audio.microphone.input_level demo. Mutating C "
                    "routes are exercised only with the invalid Count index, before hardware access.",
})
