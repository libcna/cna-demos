// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.texture_slots",
    .title = "Graphics-device texture slots in C",
    .description = "A C game binds one owned Texture2D to pixel-stage sampler slots, reads exact "
                   "slot identities, clears one or every matching slot, and observes automatic "
                   "unbinding when the texture handle is destroyed.",
    .apis = {"cna_graphics_device_get_texture", "cna_graphics_device_set_texture",
             "cna_graphics_device_unbind_texture", "CNA_TextureSlotInfo",
             "CNA_TEXTURE_COLLECTION_MAX_TEXTURES", "CNA_ShaderStage"},
    .tags = {"c", "binding", "graphics", "device", "texture", "sampler", "lifecycle"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; uses only the two highest pixel-stage slots on its own "
                    "fresh device and leaves both empty. No vertex-stage texture-fetch, drawing or "
                    "visual result is claimed. Built only with CNA_BUILD_C_API=ON.",
})
