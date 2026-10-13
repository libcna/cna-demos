// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.title_content",
    .title = "Title content location in C",
    .description = "A renderer-independent C program moves TitleLocation to the deployed assets "
                   "directory, reads a complete PNG through TitleContainer and restores the root.",
    .apis = {"cna_game_create", "cna_game_destroy", "cna_title_location_get_path_size",
             "cna_title_location_copy_path", "cna_title_location_set_path_ext",
             "cna_title_container_read_ext", "CNA_GameCreateInfo"},
    .tags = {"c", "binding", "game", "content", "title", "file", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone renderer-independent whole-file TitleContainer workflow over the "
                    "deployed checker asset. It restores the process-global TitleLocation root.",
})
