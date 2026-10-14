// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_curve_animation",
    .title = "CNB Curve and AnimationClip codecs in C",
    .description = "A renderer-free C program round-trips runtime Curve and AnimationClip values "
                   "and verifies the shared 48-byte keyframe encoding.",
    .apis = {"cna_cnb_encode_curve", "cna_cnb_decode_curve",
             "cna_cnb_encode_animation_clip", "cna_cnb_decode_animation_clip",
             "cna_cnb_animation_clip_destroy", "cna_cnb_animation_clip_get",
             "cna_cnb_animation_clip_get_track", "cna_cnb_animation_clip_copy_keyframes",
             "cna_cnb_reader_read_seconds", "cna_cnb_reader_read_keyframe",
             "cna_cnb_byte_writer_write_keyframe"},
    .tags = {"c", "binding", "content", "cnb", "curve", "animation", "keyframe"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses synthetic curves and keyframes in neutral CNB documents. No model, "
                    "animation player, graphics device, renderer or drawing is involved.",
})
