// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.guide_social_offline",
    .title = "Guide sign-in and offline social contracts in C",
    .description = "A C game opens and pixel-checks the local-profile sign-in Guide, closes it, then proves every "
                   "service-backed social screen refuses honestly without opening an overlay.",
    .apis = {"cna_guide_show_sign_in", "cna_guide_show_compose_message",
             "cna_guide_show_friend_request", "cna_guide_show_friends",
             "cna_guide_show_game_invite", "cna_guide_show_game_invite_for_session",
             "cna_guide_show_gamer_card", "cna_guide_show_marketplace",
             "cna_guide_show_messages", "cna_guide_show_party",
             "cna_guide_show_party_sessions", "cna_guide_show_player_review",
             "cna_guide_show_players", "cna_guide_show_achievements_ext",
             "cna_guide_get_is_visible", "cna_guide_simulate_keyboard_input_cancel_ext",
             "cna_gamer_services_component_create", "cna_game_components_add",
             "cna_game_set_frame_hooks_ext", "cna_graphics_device_get_backbuffer_data_rgba8",
             "cna_error_get_last_info", "cna_error_get_last_exception_type_size_ext",
             "cna_error_copy_last_exception_type_ext", "GamerServicesNotAvailableException",
             "GamerPrivilegeException"},
    .tags = {"c", "binding", "gamer", "guide", "sign-in", "social", "offline"},
    .needs = CnaDemos::Need::CApi | CnaDemos::Need::Window,
    .frames = 20,
    .deterministic = true,
    .restrictions = "Runs against an isolated local offline profile store. It renders the local sign-in screen; "
                    "friends, messages, parties, invitations, cards, reviews and achievements require a configured "
                    "CNA account service and are validated here only through their documented offline refusals.",
})
