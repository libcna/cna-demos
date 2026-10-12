// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.lifecycle_callbacks",
    .title = "A game in C: lifecycle callbacks",
    .description = "A C program checks the C ABI version, creates a CNA game with a table of lifecycle callbacks "
                   "(load_content, update, draw, unload_content, exiting), lets cna_game_run drive a fixed 60 Hz "
                   "loop that clears the screen, ends it with cna_game_request_exit, and verifies callback-scoped device borrowing plus refused recursive driving and destruction.",
    .apis = {"cna_get_abi_version", "cna_game_create", "cna_game_run", "cna_game_request_exit", "cna_game_clear",
             "cna_game_get_graphics_device", "cna_graphics_device_get_renderer_name_size", "cna_game_destroy", "CNA_GameCallbacks", "CNA_GameCreateInfo",
             "CNA_GameTime", "cna_game_run_frame_ext", "cna_error_get_last_info",
             "cna_error_get_last_message_size", "cna_error_copy_last_message"},
    .tags = {"c", "binding", "game-loop", "callbacks"},
    .needs = CnaDemos::Need::CApi,
    .frames = 30,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
