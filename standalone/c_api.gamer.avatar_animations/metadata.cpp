// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.avatar_animations",
    .title = "Avatar animation clips in C",
    .description = "A renderer-free C program samples every built-in avatar clip, then verifies clamped and "
                   "looping time, moving bones, keyed facial expressions and animation ownership.",
    .apis = {"cna_avatar_animation_create", "cna_avatar_animation_destroy",
             "cna_avatar_animation_get_info", "cna_avatar_animation_set_current_position",
             "cna_avatar_animation_get_expression", "cna_avatar_animation_update",
             "cna_avatar_animation_get_bone_transform_at"},
    .tags = {"c", "binding", "gamer", "avatar", "animation", "bones", "expression", "timeline"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Samples built-in CPU animation data only; opens no game, renderer or service. It validates "
                    "local transforms and facial keys, not composed skeleton placement or drawn pixels. The real "
                    "timelines expose CNA-DEMOS-ISSUE-080's stale C header description.",
})
