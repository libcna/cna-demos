// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.models",
    .title = "Model ownership, transforms, diagnostics and drawing in C",
    .description = "A C game builds a procedural Model, audits its retained graph and glTF "
                   "diagnostics, composes bone transforms and pixel-checks Model drawing.",
    .apis = {"cna_model_create_default", "cna_model_create", "cna_model_create_with_parents",
             "cna_model_destroy", "cna_model_get_bones", "cna_model_get_meshes",
             "cna_model_get_root", "cna_model_get_tag", "cna_model_set_tag",
             "cna_model_set_owned_resources", "cna_model_get_bone_transform_count",
             "cna_model_copy_absolute_bone_transforms", "cna_model_set_bone_transforms",
             "cna_model_copy_bone_transforms", "cna_model_draw",
             "cna_model_get_gltf_import_report_ext", "cna_model_set_gltf_import_report_ext",
             "cna_model_add_gltf_import_diagnostic_ext",
             "cna_model_get_gltf_import_diagnostic_ext",
             "cna_model_get_gltf_import_diagnostic_code_byte_count_ext",
             "cna_model_copy_gltf_import_diagnostic_code_ext",
             "cna_model_get_gltf_import_diagnostic_subject_byte_count_ext",
             "cna_model_copy_gltf_import_diagnostic_subject_ext",
             "cna_model_get_gltf_import_diagnostic_message_byte_count_ext",
             "cna_model_copy_gltf_import_diagnostic_message_ext",
             "cna_model_get_gltf_import_diagnostic_detail_byte_count_ext",
             "cna_model_copy_gltf_import_diagnostic_detail_ext"},
    .tags = {"c", "binding", "graphics", "model", "bones", "diagnostics", "draw"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses a caller-built two-bone graph, one synthetic colored triangle and "
                    "selected framebuffer probes. No ContentManager, glTF file, animation player, "
                    "screenshot or visual-quality claim is involved.",
})
