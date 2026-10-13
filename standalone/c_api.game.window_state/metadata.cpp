// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.window_state",
    .title = "GameWindow state and native identity in C",
    .description = "A C-owned game reads and restores window state, brackets a client-size change "
                   "with its event registration and inspects the borrowed native-window identity.",
    .apis = {"cna_game_create", "cna_game_destroy",
             "cna_game_window_get_allow_user_resizing",
             "cna_game_window_set_allow_user_resizing", "cna_game_window_get_client_bounds",
             "cna_game_window_get_current_orientation", "cna_game_window_get_native_handle_ext",
             "cna_game_window_get_screen_device_name_size",
             "cna_game_window_copy_screen_device_name",
             "cna_game_window_get_is_borderless_ext",
             "cna_game_window_set_is_borderless_ext", "cna_game_window_minimize_ext",
             "cna_game_window_restore_ext", "cna_game_window_begin_screen_device_change",
             "cna_game_window_end_screen_device_change", "cna_game_window_get_type_name_size",
             "cna_game_window_copy_type_name", "cna_game_window_subscribe",
             "cna_game_unsubscribe", "cna_native_window_handle_init",
             "cna_game_window_get_native_window_ext", "CNA_GameWindowEvent",
             "CNA_NativeWindowHandle", "CNA_NativeWindowSystem"},
    .tags = {"c", "binding", "game", "window", "native", "event", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone window workflow requiring a video display. Native pointers are "
                    "borrowed and only classified; appearance and manual window interaction are not claimed.",
})
