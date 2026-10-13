/* SPDX-License-Identifier: MS-PL */
/*
 * Text input from C: exercise the process-wide subscriptions with deterministic public raises,
 * then probe the bound and unbound activation contracts without synthesizing physical input.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct TextRecord {
    uint16_t code_units[4];
    int code_unit_count;
    char editing_text[32];
    uint64_t editing_bytes;
    int32_t editing_start;
    int32_t editing_length;
    int editing_count;
    char first_candidate[16];
    char second_candidate[16];
    int32_t candidate_count;
    int32_t selected;
    CNA_Bool horizontal;
    int candidate_events;
    int malformed;
} TextRecord;

typedef struct TextDemoState {
    CnaDemo* demo;
    int validated;
} TextDemoState;

static CNA_StringView text_view(const char* text, uint64_t bytes)
{
    CNA_StringView view;
    view.data = bytes == UINT64_C(0) ? NULL : text;
    view.byte_length = bytes;
    return view;
}

static void copy_view(char* destination, size_t capacity, CNA_StringView source, int* malformed)
{
    if ((source.data == NULL && source.byte_length != UINT64_C(0)) ||
        source.byte_length >= (uint64_t)capacity) {
        *malformed = 1;
        return;
    }
    memset(destination, 0, capacity);
    if (source.byte_length != UINT64_C(0)) {
        memcpy(destination, source.data, (size_t)source.byte_length);
    }
}

static void on_text_input(uint16_t code_unit, void* context)
{
    TextRecord* record = (TextRecord*)context;
    if (record->code_unit_count >= (int)(sizeof(record->code_units) / sizeof(record->code_units[0]))) {
        record->malformed = 1;
        return;
    }
    record->code_units[record->code_unit_count] = code_unit;
    ++record->code_unit_count;
}

static void on_text_editing(const CNA_TextEditingEventInfo* info, void* context)
{
    TextRecord* record = (TextRecord*)context;
    if (info == NULL || info->struct_size != (uint32_t)sizeof(*info) ||
        info->struct_version != UINT32_C(1)) {
        record->malformed = 1;
        return;
    }
    copy_view(record->editing_text, sizeof(record->editing_text), info->text, &record->malformed);
    record->editing_bytes = info->text.byte_length;
    record->editing_start = info->start;
    record->editing_length = info->length;
    ++record->editing_count;
}

static void on_text_editing_candidates(const CNA_TextEditingCandidatesEventInfo* info, void* context)
{
    TextRecord* record = (TextRecord*)context;
    if (info == NULL || info->struct_size != (uint32_t)sizeof(*info) ||
        info->struct_version != UINT32_C(1) || info->candidate_count < 0 ||
        (info->candidates == NULL && info->candidate_count != 0) ||
        info->reserved[0] != UINT8_C(0) || info->reserved[1] != UINT8_C(0) ||
        info->reserved[2] != UINT8_C(0)) {
        record->malformed = 1;
        return;
    }
    memset(record->first_candidate, 0, sizeof(record->first_candidate));
    memset(record->second_candidate, 0, sizeof(record->second_candidate));
    if (info->candidate_count > 0) {
        copy_view(record->first_candidate, sizeof(record->first_candidate), info->candidates[0],
                  &record->malformed);
    }
    if (info->candidate_count > 1) {
        copy_view(record->second_candidate, sizeof(record->second_candidate), info->candidates[1],
                  &record->malformed);
    }
    record->candidate_count = info->candidate_count;
    record->selected = info->selected;
    record->horizontal = info->horizontal;
    ++record->candidate_events;
}

static void activation_contract(CNA_Handle game, CnaDemo* demo, uint64_t original_window)
{
    const CNA_Rectangle area = {4, 8, 96, 24};
    CNA_Bool active = UINT8_C(9);
    CNA_Bool shown = UINT8_C(9);
    uint64_t window = UINT64_C(9);
    CNA_TextInputType type;
    int hints_ok = 1;

    cna_demo_check(
        demo,
        cna_text_input_set_window_handle_ext(game, UINT64_C(0)) == CNA_RESULT_SUCCESS &&
            cna_text_input_get_window_handle_ext(game, &window) == CNA_RESULT_SUCCESS &&
            window == UINT64_C(0),
        "an explicit zero window unbinds text input and round-trips as zero");
    cna_demo_check(
        demo,
        cna_text_input_is_active_ext(game, &active) == CNA_RESULT_SUCCESS && active == CNA_FALSE &&
            cna_text_input_is_screen_keyboard_shown_ext(game, &shown) == CNA_RESULT_SUCCESS &&
            shown == CNA_FALSE &&
            cna_text_input_is_screen_keyboard_shown_for_window_ext(game, UINT64_C(0), &shown) ==
                CNA_RESULT_SUCCESS &&
            shown == CNA_FALSE,
        "unbound activation and both screen-keyboard queries report false");
    cna_demo_check(
        demo,
        cna_text_input_start_ext(game) == CNA_RESULT_SUCCESS &&
            cna_text_input_is_active_ext(game, &active) == CNA_RESULT_SUCCESS && active == CNA_FALSE &&
            cna_text_input_set_input_rectangle_ext(game, area) == CNA_RESULT_SUCCESS &&
            cna_text_input_stop_ext(game) == CNA_RESULT_SUCCESS,
        "unbound start, placement and stop are successful no-ops");
    for (type = CNA_TEXT_INPUT_TYPE_TEXT; type <= CNA_TEXT_INPUT_TYPE_MAXIMUM; ++type) {
        hints_ok = hints_ok && cna_text_input_start_with_type_ext(game, type) == CNA_RESULT_SUCCESS &&
                   cna_text_input_is_active_ext(game, &active) == CNA_RESULT_SUCCESS &&
                   active == CNA_FALSE;
    }
    cna_demo_check(
        demo,
        hints_ok &&
            cna_text_input_start_with_type_ext(game, CNA_TEXT_INPUT_TYPE_MAXIMUM + UINT32_C(1)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_text_input_stop_ext(game) == CNA_RESULT_SUCCESS,
        "all nine input hints are accepted while the first undefined hint is refused");

    active = UINT8_C(9);
    shown = UINT8_C(9);
    cna_demo_check(
        demo,
        cna_text_input_set_window_handle_ext(game, original_window) == CNA_RESULT_SUCCESS &&
            cna_text_input_start_ext(game) == CNA_RESULT_SUCCESS &&
            cna_text_input_is_active_ext(game, &active) == CNA_RESULT_SUCCESS &&
            (active == CNA_FALSE || active == CNA_TRUE) &&
            cna_text_input_set_input_rectangle_ext(game, area) == CNA_RESULT_SUCCESS &&
            cna_text_input_is_screen_keyboard_shown_for_window_ext(game, original_window, &shown) ==
                CNA_RESULT_SUCCESS &&
            (shown == CNA_FALSE || shown == CNA_TRUE) &&
            cna_text_input_stop_ext(game) == CNA_RESULT_SUCCESS,
        "the backend's original window accepts activation, placement, visibility query and stop");
    cna_demo_check(
        demo,
        cna_text_input_is_active_ext(game, &active) == CNA_RESULT_SUCCESS &&
            active == CNA_FALSE,
        "stopping leaves text input inactive for either backend window state");
}

static void event_contract(CNA_Handle game, CnaDemo* demo)
{
    static const char composition[] = "\xE6\x97\xA5\xC3\xA9";
    static const char first[] = "\xE6\x97\xA5";
    static const char second[] = "\xC3\xA9";
    CNA_StringView candidates[2];
    TextRecord record;
    CNA_TextInputRegistrationHandle text_registration = CNA_INVALID_HANDLE;
    CNA_TextInputRegistrationHandle editing_registration = CNA_INVALID_HANDLE;
    CNA_TextInputRegistrationHandle candidates_registration = CNA_INVALID_HANDLE;

    memset(&record, 0, sizeof(record));
    candidates[0] = text_view(first, (uint64_t)(sizeof(first) - 1U));
    candidates[1] = text_view(second, (uint64_t)(sizeof(second) - 1U));

    cna_demo_check(
        demo,
        cna_text_input_subscribe_text_input_ext(on_text_input, &record, &text_registration) ==
                CNA_RESULT_SUCCESS &&
            cna_text_input_subscribe_text_editing_ext(on_text_editing, &record,
                                                       &editing_registration) == CNA_RESULT_SUCCESS &&
            cna_text_input_subscribe_text_editing_candidates_ext(
                on_text_editing_candidates, &record, &candidates_registration) == CNA_RESULT_SUCCESS &&
            text_registration != CNA_INVALID_HANDLE && editing_registration != CNA_INVALID_HANDLE &&
            candidates_registration != CNA_INVALID_HANDLE,
        "all three process-wide text event subscriptions return owned handles");
    cna_demo_check(
        demo,
        cna_text_input_raise_text_input_ext(game, UINT16_C(0x0041)) == CNA_RESULT_SUCCESS &&
            cna_text_input_raise_text_input_ext(game, UINT16_C(0xD83D)) == CNA_RESULT_SUCCESS &&
            cna_text_input_raise_text_input_ext(game, UINT16_C(0xDE00)) == CNA_RESULT_SUCCESS &&
            record.code_unit_count == 3 && record.code_units[0] == UINT16_C(0x0041) &&
            record.code_units[1] == UINT16_C(0xD83D) && record.code_units[2] == UINT16_C(0xDE00),
        "committed UTF-16 units arrive synchronously, including a surrogate pair");
    cna_demo_check(
        demo,
        cna_text_input_raise_text_editing_ext(
            game, text_view(composition, (uint64_t)(sizeof(composition) - 1U)), 1000, -5) ==
                CNA_RESULT_SUCCESS &&
            record.editing_count == 1 && record.malformed == 0 &&
            record.editing_bytes == (uint64_t)(sizeof(composition) - 1U) &&
            memcmp(record.editing_text, composition, sizeof(composition) - 1U) == 0 &&
            record.editing_start == 1000 && record.editing_length == -5,
        "an IME draft is copied as UTF-8 while its byte offsets pass through unchanged");
    cna_demo_check(
        demo,
        cna_text_input_raise_text_editing_candidates_ext(game, candidates, 2, 7, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            record.candidate_events == 1 && record.candidate_count == 2 && record.selected == 7 &&
            record.horizontal == CNA_TRUE && record.malformed == 0 &&
            memcmp(record.first_candidate, first, sizeof(first) - 1U) == 0 &&
            memcmp(record.second_candidate, second, sizeof(second) - 1U) == 0,
        "candidate UTF-8 views, orientation and an unchecked selected index cross the callback");
    cna_demo_check(
        demo,
        cna_text_input_unsubscribe_ext(text_registration) == CNA_RESULT_SUCCESS &&
            cna_text_input_raise_text_input_ext(game, UINT16_C(0x0042)) == CNA_RESULT_SUCCESS &&
            record.code_unit_count == 3 &&
            cna_text_input_raise_text_editing_ext(game, text_view(NULL, UINT64_C(0)), 0, 0) ==
                CNA_RESULT_SUCCESS &&
            record.editing_count == 2 &&
            cna_text_input_raise_text_editing_candidates_ext(game, NULL, 0, -1, CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            record.candidate_events == 2,
        "unsubscribing committed text leaves both IME event families attached");

    cna_demo_check(
        demo,
        cna_text_input_subscribe_text_input_ext(on_text_input, &record, &text_registration) ==
                CNA_RESULT_SUCCESS &&
            cna_text_input_reset_for_tests_ext(game) == CNA_RESULT_SUCCESS &&
            cna_text_input_raise_text_input_ext(game, UINT16_C(0x0043)) == CNA_RESULT_SUCCESS &&
            cna_text_input_raise_text_editing_ext(
                game, text_view(composition, (uint64_t)(sizeof(composition) - 1U)), 0, 0) ==
                CNA_RESULT_SUCCESS &&
            cna_text_input_raise_text_editing_candidates_ext(game, candidates, 2, 0, CNA_FALSE) ==
                CNA_RESULT_SUCCESS &&
            record.code_unit_count == 3 && record.editing_count == 2 && record.candidate_events == 2,
        "reset removes every text handler before later public raises return");
    cna_demo_check(
        demo,
        cna_text_input_unsubscribe_ext(text_registration) == CNA_RESULT_SUCCESS &&
            cna_text_input_unsubscribe_ext(editing_registration) == CNA_RESULT_SUCCESS &&
            cna_text_input_unsubscribe_ext(candidates_registration) == CNA_RESULT_SUCCESS,
        "all three registrations release safely after reset cleared their events");
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    TextDemoState* state = (TextDemoState*)context;
    uint64_t original_window = UINT64_C(0);
    uint64_t reset_window = UINT64_C(9);
    CNA_Bool active = UINT8_C(9);
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    if (!cna_demo_check(
            state->demo,
            cna_text_input_get_window_handle_ext(game, &original_window) == CNA_RESULT_SUCCESS,
            "capture the backend's original text-input window token")) {
        return CNA_RESULT_SUCCESS;
    }

    activation_contract(game, state->demo, original_window);
    event_contract(game, state->demo);
    cna_demo_check(
        state->demo,
        cna_text_input_get_window_handle_ext(game, &reset_window) == CNA_RESULT_SUCCESS &&
            reset_window == UINT64_C(0) &&
            cna_text_input_is_active_ext(game, &active) == CNA_RESULT_SUCCESS && active == CNA_FALSE,
        "reset clears the window binding and leaves text input inactive");
    cna_demo_check_result(state->demo,
                          cna_text_input_set_window_handle_ext(game, original_window),
                          "restore the backend's original text-input window token");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    TextDemoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.input.text_input", argc, argv, 0);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run one frame for text input access");
    cna_demo_check(&demo, state.validated == 1, "the update callback completed the text-input audit once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
