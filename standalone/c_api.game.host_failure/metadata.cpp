// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.host_failure",
    .title = "Host frame failure and terminal cleanup",
    .description = "Fails every startup, frame, completion and unload callback position, checking CALLBACK and the "
                   "supplied diagnostic, terminal state without replay, exactly-once unload and handle release; also "
                   "checks the public promise that exiting and end_run run on failed completion.",
    .apis = {"cna_game_create", "cna_game_set_frame_hooks_ext", "cna_game_run_frame_ext", "cna_game_destroy",
             "cna_error_get_last_info", "cna_error_get_last_message_size", "cna_error_copy_last_message",
             "cna_game_get_target_elapsed_time_ticks", "cna_game_request_exit"},
    .tags = {"c", "binding", "game", "lifecycle", "errors"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Sequential C games share one process and renderer; no rendered scene or browser scheduling "
                    "validation. ISSUE-051: the public completion comment promises C exit hooks that the adapter "
                    "suppresses after startup/frame/exiting failure.",
    .knownIssue = "CNA-DEMOS-ISSUE-051",
})
