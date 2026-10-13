// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.effect_core",
    .title = "Core Effect lifecycle and reflection in C",
    .description = "A C game exercises an empty Effect's reflection, selection, clone, disposal and retained "
                   "views, then probes malformed, foreign and real XNA Effect Framework binaries against the "
                   "selected renderer's explicit compiled-effect capability.",
    .apis = {"cna_effect_create_empty", "cna_effect_create_compiled", "cna_effect_destroy",
             "cna_effect_clone", "cna_effect_dispose", "cna_effect_apply",
             "cna_effect_get_parameters", "cna_effect_get_techniques",
             "cna_effect_get_current_technique", "cna_effect_set_current_technique",
             "cna_effect_get_graphics_device", "cna_effect_get_type_name_byte_count",
             "cna_effect_copy_type_name", "cna_effect_get_is_compiled_ext",
             "cna_effect_get_vertex_source_byte_count", "cna_effect_copy_vertex_source",
             "cna_effect_get_fragment_source_byte_count", "cna_effect_copy_fragment_source",
             "cna_effect_has_renderer", "cna_effect_is_exact_stock_sprite_effect"},
    .tags = {"c", "binding", "graphics", "effect", "compiled-effect", "reflection", "technique", "lifecycle"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::ThreeD,
    .frames = 1,
    .assets = {"effects/CnaConformanceEffect.fxb"},
    .restrictions = "Standalone C game. Compiled reflection and effect-owned Apply run when the selected renderer "
                    "advertises compiled effects; an unsupported renderer must refuse the same valid fixture. "
                    "Apply state is inspected through the public object graph, with no draw, pixel or visual claim. "
                    "Built only with CNA_BUILD_C_API=ON.",
})
