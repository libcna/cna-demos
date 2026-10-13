// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.guide_dispatcher",
    .title = "Guide and gamer-services dispatcher in C",
    .description = "A C game creates the canonical gamer-services component, initializes and pumps the dispatcher, "
                   "then validates Guide state, trial latching, window identity and event registration ownership.",
    .apis = {"cna_guide_get_is_screen_saver_enabled", "cna_guide_set_is_screen_saver_enabled",
             "cna_guide_get_is_trial_mode", "cna_guide_get_is_visible",
             "cna_guide_get_notification_position", "cna_guide_set_notification_position",
             "cna_guide_get_simulate_trial_mode", "cna_guide_set_simulate_trial_mode",
             "cna_gamer_services_dispatcher_get_is_initialized",
             "cna_gamer_services_dispatcher_get_window_handle",
             "cna_gamer_services_dispatcher_set_window_handle",
             "cna_gamer_services_dispatcher_initialize", "cna_gamer_services_dispatcher_update",
             "cna_gamer_services_dispatcher_update_async",
             "cna_gamer_services_dispatcher_get_freed_gamer_count_ext",
             "cna_gamer_services_dispatcher_subscribe_installing_title_update_ext",
             "cna_gamer_services_component_create",
             "cna_signed_in_gamer_subscribe_signed_in_ext",
             "cna_signed_in_gamer_subscribe_signed_out_ext", "cna_gamer_unsubscribe_ext",
             "cna_game_create", "cna_game_components_add", "cna_game_run_one_frame",
             "cna_game_component_destroy", "cna_game_destroy", "cna_storage_set_app_name_ext"},
    .tags = {"c", "binding", "gamer", "guide", "dispatcher", "component", "events"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Window,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses an empty dedicated local-profile store and one actual game/window. Event registrations "
                    "are lifecycle-tested but intentionally remain silent; visible Guide panels, user input, "
                    "rendered appearance and title-update delivery are separate.",
})
