// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_model_morph",
    .title = "CNB model morph data in C",
    .description = "A renderer-free C program builds two morph targets, copied delta/default streams and a "
                   "cubic weight key, then validates replacement, removal and atomic transfer behavior.",
    .apis = {"cna_cnb_model_has_morph", "cna_cnb_model_get_morph",
             "cna_cnb_model_set_morph", "cna_cnb_model_clear_morph",
             "cna_cnb_model_add_morph_target", "cna_cnb_model_set_morph_target_deltas",
             "cna_cnb_model_copy_morph_target_deltas", "cna_cnb_model_set_morph_weights",
             "cna_cnb_model_copy_morph_weights", "cna_cnb_model_add_morph_weight_key",
             "cna_cnb_model_get_morph_weight_key",
             "cna_cnb_model_copy_morph_weight_key_values"},
    .tags = {"c", "binding", "content", "cnb", "model", "morph", "animation"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Pure neutral model data: no document codec, mesh deformation, animation player, "
                    "graphics device, renderer or draw call is used.",
})
