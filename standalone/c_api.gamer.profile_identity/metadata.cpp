// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.profile_identity",
    .title = "Gamer identity and profile in C",
    .description = "A renderer-free C program mutates the shared Gamer identity surface, reads independent "
                   "synchronous and asynchronous profiles, and records unsupported global lookup services.",
    .apis = {"cna_gamer_get_display_name_size", "cna_gamer_copy_display_name",
             "cna_gamer_set_display_name", "cna_gamer_get_gamertag_size",
             "cna_gamer_copy_gamertag", "cna_gamer_get_text_size", "cna_gamer_copy_text",
             "cna_gamer_get_is_disposed", "cna_gamer_get_tag", "cna_gamer_set_tag",
             "cna_gamer_get_profile", "cna_gamer_begin_get_profile",
             "cna_gamer_get_from_gamertag", "cna_gamer_begin_get_from_gamertag",
             "cna_gamer_get_partner_token_size", "cna_gamer_copy_partner_token",
             "cna_gamer_begin_get_partner_token", "cna_gamer_profile_get_info",
             "cna_gamer_profile_get_motto_size", "cna_gamer_profile_copy_motto",
             "cna_gamer_profile_get_region_name_size", "cna_gamer_profile_copy_region_name",
             "cna_gamer_profile_get_picture_size", "cna_gamer_profile_copy_picture",
             "cna_gamer_profile_destroy", "cna_signed_in_gamer_create_ext",
             "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "gamer", "identity", "profile", "async", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Explicit offline gamer only; opens no game, renderer or network service. The profile is a "
                    "local snapshot with no picture; global gamertag lookup and partner-token operations are "
                    "documented runtime refusals rather than service-backed results.",
})
