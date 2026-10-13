// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.runtime_controls",
    .title = "Game runtime controls in C",
    .description = "A C-owned game round-trips timing and window properties, separates the framework "
                   "pump from a frame tick, suppresses one draw and owns event registrations.",
    .apis = {"cna_game_create", "cna_game_destroy", "cna_game_run_one_frame",
             "cna_game_set_window_title", "cna_game_window_get_title_size",
             "cna_game_window_copy_title", "cna_game_get_is_mouse_visible",
             "cna_game_set_is_mouse_visible", "cna_game_get_is_fixed_time_step",
             "cna_game_set_is_fixed_time_step", "cna_game_get_target_elapsed_time_ticks",
             "cna_game_set_target_elapsed_time_ticks",
             "cna_game_get_inactive_sleep_time_ticks",
             "cna_game_set_inactive_sleep_time_ticks", "cna_game_get_target_fps_ext",
             "cna_game_get_target_ms_frame_time_ext",
             "cna_game_fps_to_milliseconds_per_frame_ext",
             "cna_game_get_run_application_ext", "cna_game_set_run_application_ext",
             "cna_game_reset_elapsed_time", "cna_game_suppress_draw", "cna_game_tick",
             "cna_game_subscribe", "cna_game_unsubscribe", "cna_framework_dispatcher_update",
             "CNA_GameEvent", "CNA_GameEventRegistrationHandle"},
    .tags = {"c", "binding", "game", "timing", "tick", "event", "window"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone game/window workflow requiring a video display. It verifies control "
                    "state and callback counts, not wall-clock pacing, focus changes or pixels.",
})
