// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.model_loading",
    .title = "Loaded Model variants and processor tags in C",
    .description = "A C ContentManager loads a glTF Model, selects each authored material "
                   "variant and reads both dictionary-shaped and caller-created XNB Model tags.",
    .apis = {"cna_content_manager_load_model",
             "cna_model_get_material_variant_count_ext",
             "cna_model_get_material_variant_name_byte_count_ext",
             "cna_model_copy_material_variant_name_ext",
             "cna_model_get_material_variant_ext",
             "cna_model_set_material_variant_ext",
             "cna_model_get_content_tag_dictionary_ext",
             "cna_model_get_content_tag_foreign_object_ext"},
    .tags = {"c", "binding", "content", "model", "gltf", "material-variant", "xnb", "tag"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .deterministic = true,
    .assets = {"models/loose_paint_toy.gltf"},
    .restrictions = "Uses exact back-buffer pixel probes for the supplied deterministic glTF and "
                    "temporary hand-written XNB fixtures. It makes no screenshot or visual-quality claim.",
})
