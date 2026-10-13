// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.math.color_values",
    .title = "Color values in C",
    .description = "A renderer-independent C program constructs colors from every public input family, "
                   "observes packed and vector forms, exact text buffers and value identity, then applies "
                   "interpolation, scaling, premultiplication and native vector packing.",
    .apis = {"cna_color_init_vector4", "cna_color_init_vector3",
             "cna_color_init_float_rgb", "cna_color_init_float_rgba",
             "cna_color_init_int_rgb", "cna_color_init_int_rgba",
             "cna_color_init_bytes_rgb", "cna_color_init_bytes_rgba",
             "cna_color_get_packed_value", "cna_color_set_packed_value",
             "cna_color_get_debug_string_byte_count", "cna_color_copy_debug_string",
             "cna_color_equals", "cna_color_not_equals", "cna_color_to_vector3",
             "cna_color_to_vector4", "cna_color_get_hash_code",
             "cna_color_get_string_byte_count", "cna_color_copy_string",
             "cna_color_lerp", "cna_color_from_non_premultiplied_vector4",
             "cna_color_from_non_premultiplied_int", "cna_color_multiply",
             "cna_color_pack_from_vector4", "CNA_Color"},
    .tags = {"c", "binding", "math", "color", "packing", "interpolation", "value"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent C value workflow. It creates no game, graphics "
                    "device or image and makes no colour-space, blending or display-quality claim.",
})
