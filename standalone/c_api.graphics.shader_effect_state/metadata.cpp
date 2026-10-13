// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.shader_effect_state",
    .title = "ShaderEffect validity, diagnostics and matrices in C",
    .description = "A C game creates source appropriate to the selected renderer dialect, distinguishes object "
                   "creation from renderer validity, proves empty successful compile diagnostics and round-trips "
                   "independent World, View and Projection matrices.",
    .apis = {"cna_shader_effect_create", "cna_shader_effect_is_valid",
             "cna_shader_effect_has_renderer", "cna_shader_effect_copy_compile_error_ext",
             "cna_shader_effect_get_world", "cna_shader_effect_set_world",
             "cna_shader_effect_get_view", "cna_shader_effect_set_view",
             "cna_shader_effect_get_projection", "cna_shader_effect_set_projection",
             "cna_graphics_device_get_shader_dialect_ext",
             "cna_graphics_device_executes_shader_effect_source_ext",
             "cna_effect_apply", "cna_effect_destroy"},
    .tags = {"c", "binding", "graphics", "effect", "shader", "source", "diagnostics", "matrices", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C game. A renderer that reports source execution must compile and apply the supplied "
                    "GLSL; non-executing renderers validate their documented object-level contract. Nothing is drawn "
                    "and no pixel or visual claim is made. Built only with CNA_BUILD_C_API=ON.",
})
