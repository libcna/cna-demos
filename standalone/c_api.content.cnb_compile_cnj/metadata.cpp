// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_compile_cnj",
    .title = "Generic CNJ-to-CNB compilation in C",
    .description = "A renderer-free C program compiles a Model CNJ, audits the generic result's "
                   "bytes and dependency lists, then decodes the resulting CNB document.",
    .apis = {"cna_cnb_compile_cnj", "cna_cnb_cnj_result_destroy",
             "cna_cnb_cnj_result_get_asset_type_id",
             "cna_cnb_cnj_result_get_asset_type_name_size",
             "cna_cnb_cnj_result_copy_asset_type_name", "cna_cnb_cnj_result_copy_bytes",
             "cna_cnb_cnj_result_get_absorbed_file_count",
             "cna_cnb_cnj_result_get_absorbed_file_size",
             "cna_cnb_cnj_result_copy_absorbed_file",
             "cna_cnb_cnj_result_get_external_reference_count",
             "cna_cnb_cnj_result_get_external_reference_size",
             "cna_cnb_cnj_result_copy_external_reference"},
    .tags = {"c", "binding", "content", "cnb", "cnj", "compiler", "model"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Creates a private Model CNJ and binary sidecars. No ContentManager, graphics "
                    "device, renderer, animation playback, draw call or visual claim is involved.",
})
