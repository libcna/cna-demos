// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.additional_loaders",
    .title = "Additional ContentManager loaders in C",
    .description = "A C-owned manager loads a WAV, a DDS cube and a caller-defined CNJ object, "
                   "including decoded state, independent handles and foreign-object caching.",
    .apis = {"cna_content_manager_create",
             "cna_content_manager_load_sound_effect", "cna_content_manager_load_texture_cube",
             "cna_content_manager_load_foreign_ext", "cna_content_manager_register_cnj_loader_ext",
             "cna_content_manager_unload", "cna_content_manager_destroy", "CNA_CnjLoaderCallback"},
    .tags = {"c", "binding", "content", "audio", "texture-cube", "cnj", "cache", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone game workflow over deployed original assets; automated audio uses SDL's dummy "
                    "driver. It verifies decoded state and ownership, not audible or rendered quality.",
})
