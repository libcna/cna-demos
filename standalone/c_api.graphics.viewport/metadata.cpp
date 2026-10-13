// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.viewport",
    .title = "Viewport values and live device state in C",
    .description = "A C program constructs Viewport values, derives bounds, aspect and safe area, performs exact "
                   "string-buffer checks, projects and unprojects through identity and perspective transforms, then "
                   "round-trips and restores the live graphics-device viewport.",
    .apis = {"cna_viewport_init", "cna_viewport_init_bounds", "cna_viewport_init_from_rectangle",
             "cna_viewport_get_aspect_ratio", "cna_viewport_get_bounds", "cna_viewport_set_bounds",
             "cna_viewport_get_title_safe_area", "cna_viewport_project", "cna_viewport_unproject",
             "cna_viewport_get_string_size", "cna_viewport_copy_string",
             "cna_graphics_device_get_viewport", "cna_graphics_device_set_viewport",
             "cna_matrix_get_identity", "cna_matrix_create_look_at",
             "cna_matrix_create_perspective_field_of_view", "CNA_Viewport"},
    .tags = {"c", "binding", "graphics", "device", "viewport", "projection", "screen-space"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; the live viewport is restored during the same callback. Nothing is drawn "
                    "and no visual result is claimed. Built only with CNA_BUILD_C_API=ON.",
})
