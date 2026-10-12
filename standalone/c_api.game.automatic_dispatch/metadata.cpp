// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.automatic_dispatch",
    .title = "Automatic C dispatch at Update and Draw",
    .description = "Ordinary host frames serve a worker through Update while Draw is suppressed, with callback-scoped device borrowing. "
                   "A second setter starts in BeginDraw after Update returns and must be observed by the very first Draw, without an explicit pump.",
    .apis = {"cna_game_create", "cna_game_destroy", "cna_game_run_frame_ext", "cna_game_request_exit",
             "cna_game_set_frame_hooks_ext", "cna_game_set_foreign_thread_calls_ext",
             "cna_game_get_target_elapsed_time_ticks", "cna_game_set_target_elapsed_time_ticks",
             "cna_game_get_graphics_device", "cna_graphics_device_get_renderer_name_size"},
    .tags = {"c", "binding", "threads", "game", "callbacks"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C11 program; requires CNA::CApi and C11 threads/atomics. "
                    "Bounded dispatch shutdown precedes joining in both phases. No rendered image, "
                    "timing/latency guarantee, other callback-location coverage or browser scheduling is claimed.",
})
