// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_model_from_cnj",
    .title = "CNJ model compiler result in C",
    .description = "A renderer-free C program compiles a filesystem CNJ model, inspects its "
                   "absorbed and external dependency lists, and transfers the compiled model.",
    .apis = {"cna_cnb_build_model_from_cnj", "cna_cnb_model_from_cnj_destroy",
             "cna_cnb_model_from_cnj_take_model",
             "cna_cnb_model_from_cnj_get_absorbed_file_count",
             "cna_cnb_model_from_cnj_get_absorbed_file_size",
             "cna_cnb_model_from_cnj_copy_absorbed_file",
             "cna_cnb_model_from_cnj_get_external_reference_count",
             "cna_cnb_model_from_cnj_get_external_reference_size",
             "cna_cnb_model_from_cnj_copy_external_reference"},
    .tags = {"c", "binding", "content", "cnb", "cnj", "model", "filesystem"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Creates a private temporary CNJ fixture on Linux. It compiles neutral model "
                    "data without a ContentManager, graphics device, renderer or draw call.",
})
