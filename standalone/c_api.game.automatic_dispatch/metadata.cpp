// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.automatic_dispatch",
    .title = "Normal C game updates serve a worker automatically",
    .description = "A worker reads and changes the owner's target step while ordinary host frames serve its calls. "
                   "Draw is suppressed, no explicit pump is called, and the owner's Update observes the change. "
                   "A served device borrow proves callback scope and expires before host control resumes.",
    .apis = {"cna_game_create", "cna_game_destroy", "cna_game_run_frame_ext", "cna_game_request_exit",
             "cna_game_set_frame_hooks_ext", "cna_game_set_foreign_thread_calls_ext",
             "cna_game_get_target_elapsed_time_ticks", "cna_game_set_target_elapsed_time_ticks",
             "cna_game_get_graphics_device", "cna_graphics_device_get_renderer_name_size"},
    .tags = {"c", "binding", "threads", "game", "callbacks"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C11 program; requires CNA::CApi and C11 threads/atomics. "
                    "Bounded host-frame service and dispatch shutdown precede joining. Draw is suppressed; "
                    "no rendered image, timing/latency guarantee, automatic Draw service or browser scheduling is claimed.",
})
