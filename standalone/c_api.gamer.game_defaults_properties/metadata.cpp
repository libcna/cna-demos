// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.gamer.game_defaults_properties",
    .title = "Game defaults and property dictionaries in C",
    .description = "A renderer-free C program verifies canonical gamer defaults, then writes, identifies, "
                   "reads, replaces, enumerates, removes and clears every C-writable property kind.",
    .apis = {"cna_game_defaults_init", "cna_signed_in_gamer_get_game_defaults",
             "cna_property_dictionary_create_ext", "cna_property_dictionary_destroy",
             "cna_property_dictionary_get_count", "cna_property_dictionary_get_is_read_only",
             "cna_property_dictionary_contains_key", "cna_property_dictionary_try_get_value_kind_ext",
             "cna_property_dictionary_get_date_time_ticks", "cna_property_dictionary_get_double",
             "cna_property_dictionary_get_int32", "cna_property_dictionary_get_int64",
             "cna_property_dictionary_get_outcome", "cna_property_dictionary_get_single",
             "cna_property_dictionary_get_stream_size_ext", "cna_property_dictionary_get_string_size",
             "cna_property_dictionary_copy_string", "cna_property_dictionary_get_time_span_ticks",
             "cna_property_dictionary_set_date_time_ticks", "cna_property_dictionary_set_double",
             "cna_property_dictionary_set_int32", "cna_property_dictionary_set_int64",
             "cna_property_dictionary_set_outcome", "cna_property_dictionary_set_single",
             "cna_property_dictionary_set_string", "cna_property_dictionary_set_time_span_ticks",
             "cna_property_dictionary_remove", "cna_property_dictionary_clear",
             "cna_property_dictionary_get_key_size_at", "cna_property_dictionary_copy_key_at",
             "cna_signed_in_gamer_create_ext", "cna_signed_in_gamer_destroy"},
    .tags = {"c", "binding", "gamer", "defaults", "properties", "tagged-values", "ownership"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Uses an explicit offline gamer and an in-memory dictionary; opens no game, renderer or "
                    "service. The C factory has no stream setter, so the stream route is demonstrated through "
                    "its typed mismatch/reset contract rather than a positive service-supplied stream.",
})
