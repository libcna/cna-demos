// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.components",
    .title = "Game components in C",
    .description = "C cannot implement IUpdateable or IDrawable, so a C component is a callback table and CNA "
                   "supplies the object: an updateable clock and two overlapping drawable panels are added to the "
                   "game's collection, then the game lowers a draw order, hides a panel, disables and removes the "
                   "clock, and each change raises its event.",
    .apis = {"cna_game_component_create", "cna_drawable_game_component_create", "cna_game_component_callbacks_init",
             "cna_drawable_game_component_set_draw_order", "cna_drawable_game_component_set_visible",
             "cna_game_component_set_enabled", "cna_game_component_subscribe", "cna_game_components_add",
             "cna_game_components_remove", "cna_game_components_get_count", "cna_game_components_subscribe_added",
             "cna_game_components_subscribe_removed", "cna_game_component_destroy", "CNA_GameComponentCallbacks"},
    .tags = {"c", "binding", "game", "component", "draw-order", "events"},
    .needs = CnaDemos::Need::CApi,
    .frames = 6,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON).",
})
