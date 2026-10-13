// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.avatar_renderer",
    .title = "Avatar rendering and skeletons in C",
    .description = "A C game waits for a random avatar renderer, audits its transforms, lighting, 71-bone "
                   "hierarchy and bind pose, then draws both an AvatarAnimation and caller-supplied bones.",
    .apis = {"cna_avatar_renderer_create", "cna_avatar_renderer_destroy",
             "cna_avatar_renderer_get_info", "cna_avatar_renderer_get_transforms",
             "cna_avatar_renderer_set_transforms", "cna_avatar_renderer_get_lighting",
             "cna_avatar_renderer_set_lighting", "cna_avatar_renderer_get_parent_bone_at",
             "cna_avatar_renderer_get_bind_pose_at", "cna_avatar_renderer_draw_animation",
             "cna_avatar_renderer_draw_bones"},
    .tags = {"c", "binding", "gamer", "avatar", "renderer", "skeleton", "bones", "3d"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Window | CnaDemos::Need::ThreeD,
    .frames = 600,
    .restrictions = "Creates an isolated C game and GamerServicesComponent, then requires the built-in avatar "
                    "catalog to reach Ready. Pixel readback is checked on supported HiDef renderers; the random "
                    "avatar's appearance varies between runs.",
})
