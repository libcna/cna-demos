// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.foreign_thread_dispatch",
    .title = "Serve C worker calls on the game owner thread",
    .description = "Checks default handle thread affinity, explicitly pumps opted-in worker reads and writes "
                   "of the game's target step, forwards a served native failure with its structured exception details, "
                   "checks owner-only refusals, cancels a queued property write without replay and resumes dispatch. "
                   "Final owner destruction releases another waiting property call and invalidates the game handle.",
    .apis = {"cna_game_create", "cna_game_destroy", "cna_game_run_frame_ext", "cna_game_set_foreign_thread_calls_ext",
             "cna_game_run_foreign_thread_calls_ext", "cna_game_get_target_elapsed_time_ticks",
             "cna_game_set_target_elapsed_time_ticks", "cna_error_get_last_info",
             "cna_sound_effect_set_master_volume", "cna_error_get_last_message_size", "cna_error_copy_last_message",
             "cna_error_get_last_exception_type_size_ext", "cna_error_copy_last_exception_type_ext",
             "cna_error_get_last_exception_param_name_size_ext", "cna_error_copy_last_exception_param_name_ext"},
    .tags = {"c", "binding", "threads", "game", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C11 program; requires CNA::CApi and C11 threads/atomics. "
                    "Owner-only probes never pump and safely cancel incorrectly queued calls (ISSUE-050). "
                    "A permitted property write is cancelled before execution; re-enabled dispatch serves a fresh worker. "
                    "Final owner destruction cancels a waiting property call before bounded joining. "
                    "No game loop, rendering or automatic callback pumping is demonstrated.",
    .knownIssue = "CNA-DEMOS-ISSUE-050",
})
