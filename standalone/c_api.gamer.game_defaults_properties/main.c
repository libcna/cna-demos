/* SPDX-License-Identifier: MS-PL */
/* Canonical GameDefaults and every C-writable PropertyDictionary kind through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

#define TEXT_VIEW(literal) text_view((literal), sizeof(literal) - 1U)

static int color_is_zero(CNA_Color value)
{
    return value.r == 0U && value.g == 0U && value.b == 0U && value.a == 0U;
}

static int defaults_are_canonical(const CNA_GameDefaults* value)
{
    return value->struct_size == (uint32_t)sizeof(*value) &&
        value->struct_version == UINT32_C(1) &&
        value->game_difficulty == CNA_GAME_DIFFICULTY_EASY &&
        value->controller_sensitivity == CNA_CONTROLLER_SENSITIVITY_LOW &&
        value->racing_camera_angle == CNA_RACING_CAMERA_ANGLE_BACK &&
        value->has_primary_color == CNA_FALSE &&
        value->has_secondary_color == CNA_FALSE && value->auto_aim == CNA_FALSE &&
        value->auto_center == CNA_FALSE &&
        value->move_with_right_thumb_stick == CNA_FALSE &&
        value->invert_y_axis == CNA_FALSE && value->manual_transmission == CNA_FALSE &&
        value->accelerate_with_buttons == CNA_FALSE &&
        value->brake_with_buttons == CNA_FALSE && value->reserved[0] == 0U &&
        value->reserved[1] == 0U && value->reserved[2] == 0U &&
        color_is_zero(value->primary_color) && color_is_zero(value->secondary_color);
}

int main(int argc, char** argv)
{
    static const char GamerName[] = "Defaults Gamer";
    static const char StoredText[] = "copied text";
    static const char* const Keys[] = {
        "date", "double", "int32", "int64", "outcome", "single", "string", "timespan"
    };
    static const CNA_PropertyValueKind Kinds[] = {
        CNA_PROPERTY_VALUE_KIND_DATE_TIME, CNA_PROPERTY_VALUE_KIND_DOUBLE,
        CNA_PROPERTY_VALUE_KIND_INT32, CNA_PROPERTY_VALUE_KIND_INT64,
        CNA_PROPERTY_VALUE_KIND_OUTCOME, CNA_PROPERTY_VALUE_KIND_SINGLE,
        CNA_PROPERTY_VALUE_KIND_STRING, CNA_PROPERTY_VALUE_KIND_TIME_SPAN
    };
    const int64_t date_ticks = INT64_C(638000000000000000);
    const int64_t int64_value = INT64_C(0x123456789ABC);
    const int64_t span_ticks = -INT64_C(7654321);
    CnaDemo demo;
    CNA_SignedInGamerHandle gamer = CNA_INVALID_HANDLE;
    CNA_PropertyDictionaryHandle dictionary = CNA_INVALID_HANDLE;
    CNA_GameDefaults initialized;
    CNA_GameDefaults gamer_defaults;
    CNA_GameDefaults malformed;
    unsigned char malformed_before[sizeof(CNA_GameDefaults)];
    CNA_Bool flag = CNA_TRUE;
    CNA_PropertyValueKind kind = CNA_PROPERTY_VALUE_KIND_TIME_SPAN;
    int32_t count = -1;
    int32_t int32_result = 0;
    int64_t int64_result = 0;
    double double_result = 0.0;
    float single_result = 0.0F;
    CNA_LeaderboardOutcome outcome_result = CNA_LEADERBOARD_OUTCOME_NONE;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    char mutable_text[] = "copied text";
    char buffer[32];
    char before[32];
    int all_kinds = 1;
    int all_keys = 1;
    int index;

    cna_demo_init(&demo, "c_api.gamer.game_defaults_properties", argc, argv, 1);

    memset(&initialized, 0xA5, sizeof(initialized));
    cna_demo_check(
        &demo,
        cna_game_defaults_init(&initialized) == CNA_RESULT_SUCCESS &&
            defaults_are_canonical(&initialized),
        "GameDefaults initializes every field to its canonical value");
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(GamerName, sizeof(GamerName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &gamer),
            "create an offline gamer with canonical defaults")) {
        goto cleanup;
    }
    memset(&gamer_defaults, 0, sizeof(gamer_defaults));
    gamer_defaults.struct_size = (uint32_t)sizeof(gamer_defaults);
    gamer_defaults.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_get_game_defaults(gamer, &gamer_defaults) ==
                CNA_RESULT_SUCCESS &&
            defaults_are_canonical(&gamer_defaults),
        "the signed-in gamer exposes the same complete canonical defaults");
    memset(&malformed, 0xA5, sizeof(malformed));
    malformed.struct_size = (uint32_t)sizeof(malformed);
    malformed.struct_version = UINT32_C(2);
    memcpy(malformed_before, &malformed, sizeof(malformed));
    cna_demo_check(
        &demo,
        cna_signed_in_gamer_get_game_defaults(gamer, &malformed) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&malformed, malformed_before, sizeof(malformed)) == 0,
        "an unknown defaults structure version is rejected without changing the output");

    if (!cna_demo_check_result(
            &demo, cna_property_dictionary_create_ext(&dictionary),
            "create an empty property dictionary")) {
        goto cleanup;
    }
    flag = CNA_TRUE;
    kind = CNA_PROPERTY_VALUE_KIND_TIME_SPAN;
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_count(dictionary, &count) == CNA_RESULT_SUCCESS &&
            count == 0 &&
            cna_property_dictionary_get_is_read_only(dictionary, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_property_dictionary_contains_key(dictionary, TEXT_VIEW("missing"), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_property_dictionary_try_get_value_kind_ext(
                dictionary, TEXT_VIEW("missing"), &flag, &kind) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && kind == CNA_PROPERTY_VALUE_KIND_UNKNOWN,
        "the empty dictionary reports zero entries, canonical read-only metadata and an absent key");

    cna_demo_check(
        &demo,
        cna_property_dictionary_set_date_time_ticks(
            dictionary, TEXT_VIEW("date"), date_ticks) == CNA_RESULT_SUCCESS &&
            cna_property_dictionary_set_double(dictionary, TEXT_VIEW("double"), 1.25) ==
                CNA_RESULT_SUCCESS &&
            cna_property_dictionary_set_int32(dictionary, TEXT_VIEW("int32"), -42) ==
                CNA_RESULT_SUCCESS &&
            cna_property_dictionary_set_int64(dictionary, TEXT_VIEW("int64"), int64_value) ==
                CNA_RESULT_SUCCESS &&
            cna_property_dictionary_set_outcome(
                dictionary, TEXT_VIEW("outcome"), CNA_LEADERBOARD_OUTCOME_WIN) ==
                CNA_RESULT_SUCCESS &&
            cna_property_dictionary_set_single(dictionary, TEXT_VIEW("single"), 3.5F) ==
                CNA_RESULT_SUCCESS &&
            cna_property_dictionary_set_string(
                dictionary, TEXT_VIEW("string"),
                text_view(mutable_text, sizeof(mutable_text) - 1U)) == CNA_RESULT_SUCCESS &&
            cna_property_dictionary_set_time_span_ticks(
                dictionary, TEXT_VIEW("timespan"), span_ticks) == CNA_RESULT_SUCCESS,
        "all eight C-writable tagged property kinds can share one dictionary");
    mutable_text[0] = 'X';
    flag = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_count(dictionary, &count) == CNA_RESULT_SUCCESS &&
            count == 8 &&
            cna_property_dictionary_get_is_read_only(dictionary, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_property_dictionary_contains_key(dictionary, TEXT_VIEW("string"), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "the writable dictionary retains eight entries despite its canonical read-only flag");

    for (index = 0; index < 8; ++index) {
        flag = CNA_FALSE;
        kind = CNA_PROPERTY_VALUE_KIND_UNKNOWN;
        if (cna_property_dictionary_try_get_value_kind_ext(
                dictionary, text_view(Keys[index], (uint64_t)strlen(Keys[index])),
                &flag, &kind) != CNA_RESULT_SUCCESS ||
            flag != CNA_TRUE || kind != Kinds[index]) {
            all_kinds = 0;
        }
    }
    cna_demo_check(
        &demo, all_kinds,
        "kind discovery identifies every date, number, outcome, text and time-span tag");

    cna_demo_check(
        &demo,
        cna_property_dictionary_get_date_time_ticks(
            dictionary, TEXT_VIEW("date"), &int64_result) == CNA_RESULT_SUCCESS &&
            int64_result == date_ticks &&
            cna_property_dictionary_get_double(
                dictionary, TEXT_VIEW("double"), &double_result) == CNA_RESULT_SUCCESS &&
            double_result == 1.25 &&
            cna_property_dictionary_get_int32(
                dictionary, TEXT_VIEW("int32"), &int32_result) == CNA_RESULT_SUCCESS &&
            int32_result == -42 &&
            cna_property_dictionary_get_int64(
                dictionary, TEXT_VIEW("int64"), &int64_result) == CNA_RESULT_SUCCESS &&
            int64_result == int64_value,
        "date, double and signed integer getters preserve their exact values");
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_outcome(
            dictionary, TEXT_VIEW("outcome"), &outcome_result) == CNA_RESULT_SUCCESS &&
            outcome_result == CNA_LEADERBOARD_OUTCOME_WIN &&
            cna_property_dictionary_get_single(
                dictionary, TEXT_VIEW("single"), &single_result) == CNA_RESULT_SUCCESS &&
            single_result == 3.5F &&
            cna_property_dictionary_get_time_span_ticks(
                dictionary, TEXT_VIEW("timespan"), &int64_result) == CNA_RESULT_SUCCESS &&
            int64_result == span_ticks,
        "outcome, single and time-span getters preserve their exact values");

    memset(buffer, 0xA5, sizeof(buffer));
    memset(before, 0xA5, sizeof(before));
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_string_size(
            dictionary, TEXT_VIEW("string"), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(StoredText) - 1U &&
            cna_property_dictionary_copy_string(
                dictionary, TEXT_VIEW("string"), buffer, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && memcmp(buffer, before, sizeof(buffer)) == 0 &&
            cna_property_dictionary_copy_string(
                dictionary, TEXT_VIEW("string"), buffer, bytes, &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(buffer, StoredText, (size_t)bytes) == 0 &&
            (unsigned char)buffer[bytes] == 0xA5U,
        "text is copied on set and copied out exactly without partial writes or a terminator");

    flag = CNA_TRUE;
    bytes = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_stream_size_ext(
            dictionary, TEXT_VIEW("string"), &flag, &bytes) == CNA_RESULT_INVALID_STATE &&
            flag == CNA_FALSE && bytes == UINT64_C(0),
        "the stream query resets its outputs before refusing a non-stream property");
    int32_result = 12345;
    int64_result = INT64_C(67890);
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_int32(
            dictionary, TEXT_VIEW("double"), &int32_result) == CNA_RESULT_INVALID_STATE &&
            int32_result == 12345 &&
            cna_property_dictionary_get_int64(
                dictionary, TEXT_VIEW("missing"), &int64_result) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            int64_result == INT64_C(67890),
        "typed getters distinguish a wrong kind from a missing key and preserve outputs");
    cna_demo_check(
        &demo,
        cna_property_dictionary_set_outcome(
            dictionary, TEXT_VIEW("outcome"),
            CNA_LEADERBOARD_OUTCOME_MAXIMUM + UINT32_C(1)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_property_dictionary_get_outcome(
                dictionary, TEXT_VIEW("outcome"), &outcome_result) == CNA_RESULT_SUCCESS &&
            outcome_result == CNA_LEADERBOARD_OUTCOME_WIN &&
            cna_property_dictionary_get_count(dictionary, &count) == CNA_RESULT_SUCCESS &&
            count == 8,
        "an undefined outcome is rejected without replacing its existing value");

    cna_demo_check(
        &demo,
        cna_property_dictionary_set_int32(dictionary, TEXT_VIEW("string"), 77) ==
                CNA_RESULT_SUCCESS &&
            cna_property_dictionary_try_get_value_kind_ext(
                dictionary, TEXT_VIEW("string"), &flag, &kind) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && kind == CNA_PROPERTY_VALUE_KIND_INT32 &&
            cna_property_dictionary_get_int32(
                dictionary, TEXT_VIEW("string"), &int32_result) == CNA_RESULT_SUCCESS &&
            int32_result == 77 &&
            cna_property_dictionary_get_count(dictionary, &count) == CNA_RESULT_SUCCESS &&
            count == 8 &&
            cna_property_dictionary_set_string(
                dictionary, TEXT_VIEW("string"), TEXT_VIEW("copied text")) ==
                CNA_RESULT_SUCCESS,
        "setting an existing key replaces its tag and value without changing the count");

    for (index = 0; index < 8; ++index) {
        const uint64_t expected = (uint64_t)strlen(Keys[index]);
        memset(buffer, 0xA5, sizeof(buffer));
        bytes = UINT64_C(99);
        copied = UINT64_C(99);
        if (cna_property_dictionary_get_key_size_at(dictionary, index, &bytes) !=
                CNA_RESULT_SUCCESS ||
            bytes != expected ||
            cna_property_dictionary_copy_key_at(
                dictionary, index, buffer, expected, &copied) != CNA_RESULT_SUCCESS ||
            copied != expected || memcmp(buffer, Keys[index], (size_t)expected) != 0 ||
            (unsigned char)buffer[expected] != 0xA5U) {
            all_keys = 0;
        }
    }
    cna_demo_check(
        &demo, all_keys,
        "indexed enumeration returns all eight keys in ascending order with exact byte copies");
    memset(buffer, 0xA5, sizeof(buffer));
    memset(before, 0xA5, sizeof(before));
    bytes = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_property_dictionary_copy_key_at(dictionary, 0, buffer, 3U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == 4U && memcmp(buffer, before, sizeof(buffer)) == 0,
        "an undersized key destination reports the full length without a partial write");
    bytes = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_property_dictionary_get_key_size_at(dictionary, 8, &bytes) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            bytes == UINT64_C(99),
        "key enumeration rejects its upper bound without changing the size output");

    flag = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_property_dictionary_remove(dictionary, TEXT_VIEW("date"), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_property_dictionary_remove(dictionary, TEXT_VIEW("date"), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_property_dictionary_get_count(dictionary, &count) == CNA_RESULT_SUCCESS &&
            count == 7,
        "remove distinguishes a deleted property from an absent no-op");
    flag = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_property_dictionary_clear(dictionary) == CNA_RESULT_SUCCESS &&
            cna_property_dictionary_get_count(dictionary, &count) == CNA_RESULT_SUCCESS &&
            count == 0 &&
            cna_property_dictionary_contains_key(dictionary, TEXT_VIEW("string"), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_property_dictionary_get_is_read_only(dictionary, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "clear empties the writable dictionary while its canonical read-only flag remains true");

    cna_demo_check_result(
        &demo, cna_property_dictionary_destroy(dictionary),
        "release the property dictionary once");
    count = 12345;
    cna_demo_check(
        &demo,
        cna_property_dictionary_destroy(dictionary) == CNA_RESULT_INVALID_HANDLE &&
            cna_property_dictionary_get_count(dictionary, &count) == CNA_RESULT_INVALID_HANDLE &&
            count == 12345,
        "a released dictionary rejects repeated destruction and stale reads atomically");
    dictionary = CNA_INVALID_HANDLE;

cleanup:
    if (dictionary != CNA_INVALID_HANDLE) {
        (void)cna_property_dictionary_destroy(dictionary);
    }
    if (gamer != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(gamer),
            "release the offline gamer after reading its defaults");
    }
    return cna_demo_finish(&demo);
}
