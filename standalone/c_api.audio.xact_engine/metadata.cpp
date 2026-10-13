// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.audio.xact_engine",
    .title = "XACT engine state in C",
    .description = "A C program opens generated XACT settings through both engine constructors, inspects the "
                   "renderer value, changes a bounded global variable, pumps the engine and observes disposal.",
    .apis = {"cna_audio_engine_create", "cna_audio_engine_create_with_renderer",
             "cna_audio_engine_destroy", "cna_audio_engine_get_is_disposed",
             "cna_audio_engine_get_renderer_count",
             "cna_audio_engine_get_renderer_friendly_name_size",
             "cna_audio_engine_copy_renderer_friendly_name",
             "cna_audio_engine_get_renderer_id_size", "cna_audio_engine_copy_renderer_id",
             "cna_audio_engine_get_renderer_text_size", "cna_audio_engine_copy_renderer_text",
             "cna_audio_engine_get_renderer_hash_code", "cna_audio_engine_renderers_equal",
             "cna_audio_engine_get_global_variable", "cna_audio_engine_set_global_variable",
             "cna_audio_engine_update", "cna_audio_engine_subscribe_disposing_ext"},
    .tags = {"c", "binding", "audio", "xact", "engine", "renderer", "variables"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Audio,
    .frames = 1,
    .deterministic = true,
    .assets = {"xact/demo.xgs"},
    .restrictions = "Uses the generated XACT settings and a dummy audio device in automation. Renderer identity "
                    "describes CNA's one backend; no speaker output, scheduling look-ahead effect or audible "
                    "quality is claimed.",
})
