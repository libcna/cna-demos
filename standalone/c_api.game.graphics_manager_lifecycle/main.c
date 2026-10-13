/* SPDX-License-Identifier: MS-PL */
/* GraphicsDeviceManager event data, reconfiguration, drawing and disposal through the public C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct State {
    CnaDemo* demo;
    CNA_GraphicsDeviceManagerHandle manager;
    int validated;
    int ordinary_events;
    int disposed_events;
    int observed_calls;
    int mutable_calls;
    int32_t observed_width;
    int32_t mutable_observed_width;
    int32_t requested_width;
    int corrupt_next;
} State;

static int accepted_or_platform(CNA_Result result)
{
    return result == CNA_RESULT_SUCCESS || result == CNA_RESULT_PLATFORM;
}

static void count_ordinary_event(void* context)
{
    ++((State*)context)->ordinary_events;
}

static void count_disposed_event(void* context)
{
    ++((State*)context)->disposed_events;
}

static void observe_settings(const CNA_GraphicsDeviceInformation* information, void* context)
{
    State* state = (State*)context;
    ++state->observed_calls;
    state->observed_width = information->presentation_parameters.back_buffer_width;
}

static void mutate_settings(CNA_GraphicsDeviceInformation* information, void* context)
{
    State* state = (State*)context;
    ++state->mutable_calls;
    state->mutable_observed_width = information->presentation_parameters.back_buffer_width;
    if (state->corrupt_next) {
        information->struct_version = 0U;
    } else {
        information->presentation_parameters.back_buffer_width = state->requested_width;
    }
}

static CNA_Result exercise_manager(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_PresentationParameters active;
    CNA_Bool should_draw = UINT8_C(99);
    CNA_Bool service_present = CNA_TRUE;
    int calls_before;
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    memset(&active, 0, sizeof(active));
    active.struct_size = (uint32_t)sizeof(active);
    active.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        state->observed_calls > 0 && state->mutable_calls == state->observed_calls &&
            state->observed_width > 0 && state->mutable_observed_width > 0,
        "read-only and mutable handlers observed the initial device preparation");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_get_graphics_device(state->manager, &device) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_presentation_parameters(device, &active) == CNA_RESULT_SUCCESS &&
            active.back_buffer_width == state->requested_width,
        "the initialized device uses the width written by the mutable handler");

    cna_demo_check(
        state->demo,
        accepted_or_platform(cna_graphics_device_manager_apply_changes(state->manager)),
        "applying unchanged manager preferences succeeds or reports platform refusal");
    calls_before = state->mutable_calls;
    active.back_buffer_width = 0;
    cna_demo_check(
        state->demo,
        accepted_or_platform(cna_graphics_device_manager_create_device(state->manager)) &&
            state->mutable_calls > calls_before &&
            cna_graphics_device_get_presentation_parameters(device, &active) == CNA_RESULT_SUCCESS &&
            active.back_buffer_width == state->requested_width,
        "explicit device creation applies a fresh mutation to the prepared settings");

    state->corrupt_next = 1;
    active.back_buffer_width = 0;
    cna_demo_check(
        state->demo,
        accepted_or_platform(cna_graphics_device_manager_create_device(state->manager)) &&
            cna_graphics_device_get_presentation_parameters(device, &active) == CNA_RESULT_SUCCESS &&
            active.back_buffer_width == CNA_GRAPHICS_DEVICE_MANAGER_DEFAULT_BACK_BUFFER_WIDTH,
        "a corrupt mutable event value is ignored and the unmodified manager width is applied");
    state->corrupt_next = 0;
    active.back_buffer_width = 0;
    cna_demo_check(
        state->demo,
        accepted_or_platform(cna_graphics_device_manager_create_device(state->manager)) &&
            cna_graphics_device_get_presentation_parameters(device, &active) == CNA_RESULT_SUCCESS &&
            active.back_buffer_width == state->requested_width,
        "the next valid mutation changes the prepared width again");
    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_begin_draw(state->manager, &should_draw) == CNA_RESULT_SUCCESS &&
            (should_draw == CNA_FALSE || should_draw == CNA_TRUE) &&
            accepted_or_platform(cna_graphics_device_manager_end_draw(state->manager)),
        "the explicit draw bracket reports eligibility and presents or reports platform refusal");
    cna_demo_check(
        state->demo,
        state->ordinary_events > 0 && state->observed_calls == state->mutable_calls,
        "device operations raised manager events and kept both settings views paired");

    cna_demo_check(
        state->demo,
        cna_graphics_device_manager_dispose(state->manager) == CNA_RESULT_SUCCESS &&
            state->disposed_events == 1 &&
            cna_graphics_device_manager_dispose(state->manager) == CNA_RESULT_SUCCESS &&
            state->disposed_events == 1 &&
            cna_game_services_contains_ext(
                game, CNA_GAME_SERVICE_TYPE_GRAPHICS_DEVICE_MANAGER, &service_present) ==
                CNA_RESULT_SUCCESS && service_present == CNA_FALSE,
        "manager disposal is idempotent, raises once and unregisters the manager service");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_GameEventRegistrationHandle registrations[7];
    int i;

    cna_demo_init(&demo, "c_api.game.graphics_manager_lifecycle", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.manager = CNA_INVALID_HANDLE;
    state.requested_width = 640;
    for (i = 0; i < 7; ++i) {
        registrations[i] = CNA_INVALID_HANDLE;
    }
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = exercise_manager;
    callbacks.context = &state;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the manager-lifecycle game") ||
        !cna_demo_check_result(
            &demo, cna_graphics_device_manager_create(game, &state.manager),
            "create its graphics device manager before initialization")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_graphics_device_manager_subscribe(
            state.manager, CNA_GRAPHICS_DEVICE_MANAGER_EVENT_DEVICE_CREATED,
            count_ordinary_event, &state, &registrations[0]) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_subscribe(
                state.manager, CNA_GRAPHICS_DEVICE_MANAGER_EVENT_DEVICE_DISPOSING,
                count_ordinary_event, &state, &registrations[1]) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_subscribe(
                state.manager, CNA_GRAPHICS_DEVICE_MANAGER_EVENT_DEVICE_RESET,
                count_ordinary_event, &state, &registrations[2]) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_subscribe(
                state.manager, CNA_GRAPHICS_DEVICE_MANAGER_EVENT_DEVICE_RESETTING,
                count_ordinary_event, &state, &registrations[3]) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_subscribe(
                state.manager, CNA_GRAPHICS_DEVICE_MANAGER_EVENT_DISPOSED,
                count_disposed_event, &state, &registrations[4]) == CNA_RESULT_SUCCESS,
        "owned registrations attach all five data-free manager events");
    cna_demo_check(
        &demo,
        cna_graphics_device_manager_subscribe_preparing_device_settings(
            state.manager, observe_settings, &state, &registrations[5]) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_subscribe_preparing_device_settings_ext(
                state.manager, mutate_settings, &state, &registrations[6]) == CNA_RESULT_SUCCESS,
        "read-only and mutable registrations share the preparing-settings event");
    cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                          "initialize and exercise the subscribed manager");
    cna_demo_check(&demo, state.validated == 1, "the one-shot manager lifecycle audit ran once");

cleanup:
    for (i = 0; i < 7; ++i) {
        if (registrations[i] != CNA_INVALID_HANDLE) {
            cna_demo_check_result(&demo, cna_game_unsubscribe(registrations[i]),
                                  "release a manager event registration");
        }
    }
    if (state.manager != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_graphics_device_manager_destroy(state.manager) == CNA_RESULT_SUCCESS &&
                cna_graphics_device_manager_destroy(state.manager) == CNA_RESULT_INVALID_HANDLE,
            "destroy releases the disposed manager handle exactly once");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the manager-lifecycle game");
    }
    return cna_demo_finish(&demo);
}
