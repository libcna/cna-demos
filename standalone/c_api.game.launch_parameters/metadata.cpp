// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.launch_parameters",
    .title = "Owned UTF-8 launch parameters through the C ABI",
    .description = "Parses explicit argument spans, checks first-value precedence and copied UTF-8 ownership, "
                   "enumerates names in ordinal order, verifies exact-capacity buffers, and retains the prior map "
                   "after malformed replacement input, distinguishes empty values and explicitly clears the map.",
    .apis = {"cna_game_create", "cna_game_destroy", "cna_game_launch_parameters_parse_ext",
             "cna_game_launch_parameters_get_count", "cna_game_launch_parameters_contains_key",
             "cna_game_launch_parameters_get_value_size", "cna_game_launch_parameters_copy_value",
             "cna_game_launch_parameters_get_key_size", "cna_game_launch_parameters_copy_key",
             "cna_game_launch_parameters_add", "CNA_StringView"},
    .tags = {"c", "binding", "arguments", "utf8", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program with an owner-thread Game; no game loop or rendered image. "
                    "Intentional localization payload exercises UTF-8 byte counts. Duplicate Add follows CNA's "
                    "documented first-value behavior, which differs from XNA Dictionary.Add.",
})
