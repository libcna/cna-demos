// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.component_members",
    .title = "Game component members and lifecycle in C",
    .description = "Two caller-defined C components expose their owner, properties, ordering and "
                   "explicit lifecycle while event registrations prove change-only delivery and detachment.",
    .apis = {"cna_game_create", "cna_game_run_one_frame", "cna_game_destroy",
             "cna_game_component_callbacks_init", "cna_game_component_create",
             "cna_drawable_game_component_create", "cna_game_component_get_game",
             "cna_game_component_get_enabled", "cna_game_component_set_enabled",
             "cna_game_component_get_update_order", "cna_game_component_set_update_order",
             "cna_drawable_game_component_get_draw_order",
             "cna_drawable_game_component_set_draw_order",
             "cna_drawable_game_component_get_visible",
             "cna_drawable_game_component_set_visible", "cna_game_component_compare_to",
             "cna_game_component_initialize", "cna_game_component_update",
             "cna_drawable_game_component_draw", "cna_game_component_dispose",
             "cna_game_component_subscribe", "cna_game_component_unsubscribe",
             "cna_game_component_destroy", "CNA_GameComponentCallbacks", "CNA_GameComponentEvent"},
    .tags = {"c", "binding", "game", "component", "lifecycle", "events", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone game workflow requiring a video display. It drives public component "
                    "members directly and makes no rendered-pixel or wall-clock claim.",
})
