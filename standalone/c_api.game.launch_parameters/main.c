/* SPDX-License-Identifier: MS-PL */
/* The C launch-parameter map owns UTF-8 byte spans and publishes a sorted count/copy view. */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>
#include "CnaDemos/CApiDemo.h"

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCreateInfo info = {0};
    CNA_Handle game = CNA_INVALID_HANDLE;
    const CNA_StringView mode = {"mode!", 4}; /* The trailing ! is outside the view. */
    const CNA_StringView profile = {"profile", 7};
    /* Intentional localization payload: four characters, five UTF-8 bytes. */
    const char expected[] = "Caf\xc3\xa9";
    char profile_argument[] = "/profile:Caf\xc3\xa9";
    const CNA_StringView arguments[] = {
        {profile_argument, sizeof(profile_argument) - 1},
        {"--mode:first", sizeof("--mode:first") - 1},
        {"/Zeta:last", sizeof("/Zeta:last") - 1},
        {"/mode:second", sizeof("/mode:second") - 1},
        {"--windowed", sizeof("--windowed") - 1},
        {"--equals=ignored", sizeof("--equals=ignored") - 1},
    };
    uint64_t count = 0, needed = 0, copied = 0;
    char buffer[32], original[32];
    CNA_Bool present = CNA_TRUE;
    cna_demo_init(&demo, "c_api.game.launch_parameters", argc, argv, 0);
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the game that owns its launch-parameter map"))
        return cna_demo_finish(&demo);

    if (!cna_demo_check(&demo, cna_game_launch_parameters_parse_ext(game, arguments, 6) == CNA_RESULT_SUCCESS &&
                        cna_game_launch_parameters_get_count(game, &count) == CNA_RESULT_SUCCESS && count == 3,
                        "parse replaces the map, ignores bare and equals-only flags and merges duplicate names")) goto cleanup;
    memset(profile_argument, '#', sizeof(profile_argument)); /* Input bytes need not outlive parsing. */
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(&demo, cna_game_launch_parameters_copy_value(game, mode, buffer, sizeof(buffer), &copied) == CNA_RESULT_SUCCESS &&
                   copied == 5 && memcmp(buffer, "first", 5) == 0,
                   "the first colon-delimited value wins and lookup honors the explicit key byte span");
    cna_demo_check(&demo, cna_game_launch_parameters_get_value_size(game, profile, &needed) == CNA_RESULT_SUCCESS &&
                   needed == sizeof(expected) - 1,
                   "the localized value size counts five UTF-8 bytes without a terminator");
    memset(buffer, '#', sizeof(buffer));
    memcpy(original, buffer, sizeof(buffer));
    copied = UINT64_MAX;
    cna_demo_check(&demo, cna_game_launch_parameters_copy_value(game, profile, buffer, 4, &copied) == CNA_RESULT_BUFFER_TOO_SMALL &&
                   copied == 5 && memcmp(buffer, original, sizeof(buffer)) == 0,
                   "a four-byte output refuses the multibyte value without any partial write");
    copied = UINT64_MAX;
    cna_demo_check(&demo, cna_game_launch_parameters_copy_value(game, profile, buffer + 1, 5, &copied) == CNA_RESULT_SUCCESS &&
                   copied == 5 && memcmp(buffer + 1, expected, 5) == 0 && buffer[0] == '#' && buffer[6] == '#',
                   "exact-capacity copying preserves guards and the owned UTF-8 bytes after caller storage changes");

    const char* const names[] = {"Zeta", "mode", "profile"};
    int ordered = 1;
    for (uint64_t index = 0; index < 3; ++index) {
        memset(buffer, '#', sizeof(buffer));
        if (cna_game_launch_parameters_get_key_size(game, index, &needed) != CNA_RESULT_SUCCESS ||
            needed != strlen(names[index]) || needed >= sizeof(buffer) ||
            cna_game_launch_parameters_copy_key(game, index, buffer, needed, &copied) != CNA_RESULT_SUCCESS ||
            copied != needed || memcmp(buffer, names[index], (size_t)needed) != 0 || buffer[needed] != '#') {
            ordered = 0;
            break;
        }
        buffer[needed] = '\0'; /* The caller adds a terminator outside the copied payload. */
        printf("Parameter %llu: %s\n", (unsigned long long)index, buffer);
    }
    cna_demo_check(&demo, ordered, "count/copy enumeration puts uppercase Zeta before lowercase names with no implicit terminators");
    cna_demo_check(&demo, cna_game_launch_parameters_get_key_size(game, 3, &needed) == CNA_RESULT_INVALID_ARGUMENT &&
                   cna_game_launch_parameters_get_value_size(game, (CNA_StringView){"missing", 7}, &needed) == CNA_RESULT_INVALID_ARGUMENT,
                   "an absent key and an index at count are refused");
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(&demo, cna_game_launch_parameters_add(game, mode, (CNA_StringView){"replacement", 11}) == CNA_RESULT_SUCCESS &&
                   cna_game_launch_parameters_copy_value(game, mode, buffer, sizeof(buffer), &copied) == CNA_RESULT_SUCCESS &&
                   copied == 5 && memcmp(buffer, "first", 5) == 0 &&
                   cna_game_launch_parameters_get_count(game, &count) == CNA_RESULT_SUCCESS && count == 3,
                   "the documented C Add contract keeps an existing value while returning success");
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(&demo, cna_game_launch_parameters_add(game, (CNA_StringView){"Alpha", 5}, (CNA_StringView){"new", 3}) == CNA_RESULT_SUCCESS &&
                   cna_game_launch_parameters_get_count(game, &count) == CNA_RESULT_SUCCESS && count == 4 &&
                   cna_game_launch_parameters_copy_key(game, 0, buffer, sizeof(buffer), &copied) == CNA_RESULT_SUCCESS &&
                   copied == 5 && memcmp(buffer, "Alpha", 5) == 0,
                   "adding a new key changes the count and a fresh sorted enumeration");
    /* The first argument would replace mode, but the next contains a truncated UTF-8 sequence.
       Fully allocated input storage makes this an encoding error, not an out-of-bounds probe. */
    const CNA_StringView malformed[] = {
        {"--mode:changed", sizeof("--mode:changed") - 1},
        {"--bad:\xc3", sizeof("--bad:\xc3") - 1},
    };
    cna_demo_check(&demo, cna_game_launch_parameters_parse_ext(game, malformed, 2) == CNA_RESULT_ENCODING,
                   "a malformed UTF-8 argument refuses replacement parsing");
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(&demo, cna_game_launch_parameters_get_count(game, &count) == CNA_RESULT_SUCCESS && count == 4 &&
                   cna_game_launch_parameters_copy_value(game, mode, buffer, sizeof(buffer), &copied) == CNA_RESULT_SUCCESS &&
                   copied == 5 && memcmp(buffer, "first", 5) == 0,
                   "failed parsing preserves the previous count and value despite an earlier valid replacement argument");
    needed = copied = UINT64_MAX;
    cna_demo_check(&demo, cna_game_launch_parameters_add(game, (CNA_StringView){"empty", 5}, (CNA_StringView){NULL, 0}) == CNA_RESULT_SUCCESS &&
                   cna_game_launch_parameters_get_value_size(game, (CNA_StringView){"empty", 5}, &needed) == CNA_RESULT_SUCCESS && needed == 0 &&
                   cna_game_launch_parameters_copy_value(game, (CNA_StringView){"empty", 5}, NULL, 0, &copied) == CNA_RESULT_SUCCESS && copied == 0,
                   "a null zero-length input represents an owned empty value with a valid zero-capacity copy");
    cna_demo_check(&demo, cna_game_launch_parameters_parse_ext(game, NULL, 0) == CNA_RESULT_SUCCESS &&
                   cna_game_launch_parameters_get_count(game, &count) == CNA_RESULT_SUCCESS && count == 0 &&
                   cna_game_launch_parameters_contains_key(game, mode, &present) == CNA_RESULT_SUCCESS && !present,
                   "an explicit empty argument list clears the map instead of reparsing the process command line");
cleanup:
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the game and its parameter map");
    return cna_demo_finish(&demo);
}
