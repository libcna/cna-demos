// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.game.component_collection",
    .title = "Game component collection and services in C",
    .description = "Caller-owned components move through indexed collection operations while change events, "
                   "absence semantics and the runtime's named service lookup remain explicit.",
    .apis = {"cna_game_create", "cna_game_run_one_frame", "cna_game_destroy",
             "cna_game_component_callbacks_init", "cna_game_component_create",
             "cna_game_component_destroy", "cna_game_components_get_count",
             "cna_game_components_get_at", "cna_game_components_add",
             "cna_game_components_insert", "cna_game_components_remove",
             "cna_game_components_remove_at", "cna_game_components_clear",
             "cna_game_components_contains", "cna_game_components_index_of",
             "cna_game_components_subscribe_added", "cna_game_components_subscribe_removed",
             "cna_game_component_unsubscribe", "cna_game_services_contains_ext",
             "cna_game_services_remove_ext", "CNA_GameServiceType"},
    .tags = {"c", "binding", "game", "component", "collection", "services", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone game workflow requiring a video display. Removing a named runtime service is "
                    "intentionally last because the C API has no matching registration route.",
})
