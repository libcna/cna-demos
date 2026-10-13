// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.avatar_descriptions",
    .title = "Avatar descriptions in C",
    .description = "A renderer-free C program initializes a face, constructs foreign, random and gamer avatar "
                   "descriptions, verifies exact bytes and owns a per-description change registration.",
    .apis = {"cna_avatar_expression_init", "cna_avatar_description_create",
             "cna_avatar_description_create_random", "cna_avatar_description_create_random_for_body_type",
             "cna_avatar_description_get_from_gamer", "cna_avatar_description_destroy",
             "cna_avatar_description_get_info", "cna_avatar_description_copy_description",
             "cna_avatar_description_subscribe_changed_ext", "cna_gamer_unsubscribe_ext",
             "cna_signed_in_gamer_create_ext", "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "gamer", "avatar", "description", "async", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "In-memory and explicit offline-gamer descriptions only; opens no game, renderer or "
                    "service. The Changed registration is owned and remains correctly silent because no public "
                    "C route changes a description; real service delivery remains separate.",
})
