// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.host_frames",
    .title = "A host-driven C game and its frame hooks",
    .description = "Advances a game through explicit host calls, verifies initialization and frame-hook order, "
                   "suppresses drawing, copies a larger compatible hook prefix, refuses incompatible tables, replaces hooks between "
                   "frames, then checks exit and terminal calls.",
    .apis = {"cna_game_create", "cna_game_set_frame_hooks_ext", "cna_game_run_frame_ext",
             "cna_game_request_exit", "cna_game_destroy", "CNA_GameFrameHooks", "CNA_GameCallbacks"},
    .tags = {"c", "binding", "game-loop", "callbacks", "host"},
    .needs = CnaDemos::Need::CApi,
    .frames = 4,
    .deterministic = true,
    .restrictions = "Standalone C program with four explicit host frames. Desktop evidence does not establish "
                    "browser animation-frame scheduling, callback-error termination or RunOneFrame behavior.",
})
