/* SPDX-License-Identifier: MS-PL */
/* Game properties, explicit pumping/ticking and event ownership through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct State {
    int updates;
    int draws;
    int retained_disposed;
    int removed_disposed;
} State;

static CNA_StringView view(const char* text, uint64_t bytes)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = bytes;
    return result;
}

static CNA_Result update(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    (void)game;
    (void)game_time;
    (void)out_error;
    ++state->updates;
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    (void)game;
    (void)game_time;
    (void)out_error;
    ++state->draws;
    return CNA_RESULT_SUCCESS;
}

static void count_event(void* context)
{
    ++*(int*)context;
}

static int near(double left, double right)
{
    return fabs(left - right) <= 0.001;
}

int main(int argc, char** argv)
{
    static const char WindowTitle[] = "C API runtime controls";
    static const char EmbeddedNul[] = {'b', 'a', 'd', '\0', 'x'};
    CnaDemo demo;
    State state = {0};
    CNA_GameCallbacks callbacks = {0};
    CNA_GameCreateInfo info = {0};
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_GameEventRegistrationHandle retained = CNA_INVALID_HANDLE;
    CNA_GameEventRegistrationHandle removed = CNA_INVALID_HANDLE;
    CNA_GameEventRegistrationHandle rejected = UINT64_MAX;
    CNA_Bool flag = UINT8_C(77);
    CNA_Bool initial_mouse = CNA_FALSE;
    int64_t initial_sleep = INT64_C(-1);
    int64_t ticks = INT64_C(-1);
    double scalar = -1.0;
    uint64_t title_bytes = UINT64_MAX;
    uint64_t copied_bytes = UINT64_MAX;
    char title[64];
    char before_title[64];
    int updates_before;
    int draws_before;

    cna_demo_init(&demo, "c_api.game.runtime_controls", argc, argv, 1);
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.context = &state;
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.window_title = view("initial", UINT64_C(7));
    info.callbacks = &callbacks;

    if (!cna_demo_check_result(
            &demo, cna_game_create(&info, &game), "create the runtime-control game")) {
        return cna_demo_finish(&demo);
    }

    memset(title, 0xA5, sizeof(title));
    cna_demo_check(
        &demo,
        cna_game_set_window_title(
            game, view(WindowTitle, sizeof(WindowTitle) - 1U)) == CNA_RESULT_SUCCESS &&
            cna_game_window_get_title_size(game, &title_bytes) == CNA_RESULT_SUCCESS &&
            title_bytes == sizeof(WindowTitle) - 1U &&
            cna_game_window_copy_title(game, title, title_bytes, &copied_bytes) ==
                CNA_RESULT_SUCCESS && copied_bytes == title_bytes &&
            memcmp(title, WindowTitle, sizeof(WindowTitle) - 1U) == 0 &&
            (unsigned char)title[title_bytes] == 0xA5U,
        "the game title round-trips through its public window as exact bytes");
    memset(title, 0xA5, sizeof(title));
    memcpy(before_title, title, sizeof(title));
    cna_demo_check(
        &demo,
        cna_game_window_copy_title(game, title, title_bytes - 1U, &copied_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL && copied_bytes == title_bytes &&
            memcmp(title, before_title, sizeof(title)) == 0 &&
            cna_game_set_window_title(game, view(EmbeddedNul, sizeof(EmbeddedNul))) ==
                CNA_RESULT_ENCODING &&
            cna_game_window_copy_title(game, title, title_bytes, &copied_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(title, WindowTitle, sizeof(WindowTitle) - 1U) == 0,
        "short title copies and malformed title updates preserve the current window title");

    cna_demo_check(
        &demo,
        cna_game_get_is_mouse_visible(game, &initial_mouse) == CNA_RESULT_SUCCESS &&
            cna_game_set_is_mouse_visible(
                game, initial_mouse == CNA_TRUE ? CNA_FALSE : CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_game_get_is_mouse_visible(game, &flag) == CNA_RESULT_SUCCESS &&
            flag != initial_mouse &&
            cna_game_set_is_mouse_visible(game, initial_mouse) == CNA_RESULT_SUCCESS &&
            cna_game_get_is_mouse_visible(game, &flag) == CNA_RESULT_SUCCESS &&
            flag == initial_mouse,
        "mouse visibility changes and the original value is restored");

    cna_demo_check(
        &demo,
        cna_game_get_is_fixed_time_step(game, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_game_set_is_fixed_time_step(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_game_get_is_fixed_time_step(game, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_game_set_is_fixed_time_step(game, CNA_TRUE) == CNA_RESULT_SUCCESS,
        "fixed and variable timing modes round-trip before the run");

    cna_demo_check(
        &demo,
        cna_game_get_target_elapsed_time_ticks(game, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(166667) &&
            cna_game_set_target_elapsed_time_ticks(game, INT64_C(333334)) ==
                CNA_RESULT_SUCCESS &&
            cna_game_get_target_fps_ext(game, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, 29.99994) &&
            cna_game_get_target_ms_frame_time_ext(game, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, 33.3334) &&
            cna_game_fps_to_milliseconds_per_frame_ext(60, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, 16.6666667),
        "target ticks derive matching frames-per-second and millisecond views");

    cna_demo_check(
        &demo,
        cna_game_get_inactive_sleep_time_ticks(game, &initial_sleep) == CNA_RESULT_SUCCESS &&
            initial_sleep >= 0 &&
            cna_game_set_inactive_sleep_time_ticks(game, INT64_C(100000)) ==
                CNA_RESULT_SUCCESS &&
            cna_game_get_inactive_sleep_time_ticks(game, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(100000) &&
            cna_game_set_inactive_sleep_time_ticks(game, INT64_C(-1)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_game_get_inactive_sleep_time_ticks(game, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(100000) &&
            cna_game_set_inactive_sleep_time_ticks(game, initial_sleep) == CNA_RESULT_SUCCESS,
        "inactive sleep duration rejects negative values without changing state");

    cna_demo_check(
        &demo,
        cna_game_get_run_application_ext(game, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_game_set_run_application_ext(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_game_get_run_application_ext(game, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_game_set_run_application_ext(game, CNA_TRUE) == CNA_RESULT_SUCCESS,
        "the raw run-loop switch can be stopped and resumed before stepping");

    cna_demo_check(
        &demo,
        CNA_GAME_EVENT_ACTIVATED == 0U && CNA_GAME_EVENT_DISPOSED == 2U &&
            CNA_GAME_EVENT_MAXIMUM == CNA_GAME_EVENT_EXITING &&
            cna_game_subscribe(
                game, CNA_GAME_EVENT_DISPOSED, count_event, &state.retained_disposed,
                &retained) == CNA_RESULT_SUCCESS && retained != CNA_INVALID_HANDLE &&
            cna_game_subscribe(
                game, CNA_GAME_EVENT_DISPOSED, count_event, &state.removed_disposed,
                &removed) == CNA_RESULT_SUCCESS && removed != CNA_INVALID_HANDLE,
        "two owned subscriptions attach independent callbacks to the Disposed event");
    cna_demo_check(
        &demo,
        cna_game_unsubscribe(removed) == CNA_RESULT_SUCCESS &&
            cna_game_unsubscribe(removed) == CNA_RESULT_INVALID_HANDLE &&
            cna_game_subscribe(
                game, CNA_GAME_EVENT_MAXIMUM + 1U, count_event, &state.removed_disposed,
                &rejected) == CNA_RESULT_INVALID_ARGUMENT && rejected == CNA_INVALID_HANDLE,
        "unsubscribe is one-shot and an undefined event clears its output handle");

    if (!cna_demo_check_result(
            &demo, cna_game_run_one_frame(game), "initialize and run one ordinary frame")) {
        goto cleanup;
    }
    updates_before = state.updates;
    draws_before = state.draws;
    cna_demo_check(
        &demo,
        cna_game_reset_elapsed_time(game) == CNA_RESULT_SUCCESS &&
            cna_framework_dispatcher_update(game) == CNA_RESULT_SUCCESS &&
            state.updates == updates_before && state.draws == draws_before,
        "resetting accumulated time and pumping framework services do not step the game");

    cna_demo_check(
        &demo,
        cna_game_set_is_fixed_time_step(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_game_suppress_draw(game) == CNA_RESULT_SUCCESS &&
            cna_game_tick(game) == CNA_RESULT_SUCCESS && state.updates == updates_before + 1 &&
            state.draws == draws_before && cna_game_tick(game) == CNA_RESULT_SUCCESS &&
            state.updates == updates_before + 2 && state.draws == draws_before + 1,
        "in variable-step mode, SuppressDraw skips exactly one Tick draw while both ticks update");

cleanup:
    (void)cna_game_set_target_elapsed_time_ticks(game, INT64_C(166667));
    cna_demo_check(
        &demo,
        cna_game_destroy(game) == CNA_RESULT_SUCCESS && state.retained_disposed == 1 &&
            state.removed_disposed == 0 &&
            cna_game_unsubscribe(retained) == CNA_RESULT_SUCCESS &&
            cna_game_unsubscribe(retained) == CNA_RESULT_INVALID_HANDLE,
        "destroy raises only the retained callback, whose detached handle remains releasable once");
    return cna_demo_finish(&demo);
}
