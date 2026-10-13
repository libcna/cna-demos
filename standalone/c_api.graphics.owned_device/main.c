/* SPDX-License-Identifier: MS-PL */
/*
 * Caller-owned GraphicsDevice lifecycle from C. Two devices coexist outside any Game. The first
 * presents, resets twice with exact event ordering, applies new presentation parameters and emits
 * one disposing callback that releases its own registration before the native event source dies.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored; no pixels are inspected).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct LifecycleState {
    CNA_Handle device;
    CNA_GraphicsDeviceEventRegistrationHandle disposing_registration;
    CNA_GraphicsDeviceEventRegistrationHandle released_disposing_registration;
    CNA_Result disposing_unsubscribe_result;
    int sequence[8];
    int sequence_count;
    int lost_count;
    int disposing_count;
    int wrong_device_count;
} LifecycleState;

static void append_event(LifecycleState* state, CNA_Handle device, int event)
{
    if (device != state->device) {
        ++state->wrong_device_count;
    }
    if (state->sequence_count < (int)(sizeof(state->sequence) / sizeof(state->sequence[0]))) {
        state->sequence[state->sequence_count] = event;
    }
    ++state->sequence_count;
}

static void on_resetting(CNA_Handle device, void* context)
{
    append_event((LifecycleState*)context, device, 1);
}

static void on_reset(CNA_Handle device, void* context)
{
    append_event((LifecycleState*)context, device, 2);
}

static void on_lost(CNA_Handle device, void* context)
{
    LifecycleState* state = (LifecycleState*)context;
    if (device != state->device) {
        ++state->wrong_device_count;
    }
    ++state->lost_count;
}

static void on_disposing(CNA_Handle device, void* context)
{
    LifecycleState* state = (LifecycleState*)context;
    if (device != state->device) {
        ++state->wrong_device_count;
    }
    ++state->disposing_count;
    state->released_disposing_registration = state->disposing_registration;
    state->disposing_unsubscribe_result =
        cna_graphics_device_unsubscribe(state->disposing_registration);
    state->disposing_registration = CNA_INVALID_HANDLE;
}

static int parameters_equal_except_size(
    const CNA_PresentationParameters* left, const CNA_PresentationParameters* right)
{
    return left->struct_version == right->struct_version &&
        left->back_buffer_width == right->back_buffer_width &&
        left->back_buffer_height == right->back_buffer_height &&
        left->back_buffer_format == right->back_buffer_format &&
        left->depth_stencil_format == right->depth_stencil_format &&
        left->multi_sample_count == right->multi_sample_count &&
        left->presentation_interval == right->presentation_interval &&
        left->display_orientation == right->display_orientation &&
        left->render_target_usage == right->render_target_usage &&
        left->is_full_screen == right->is_full_screen &&
        left->headless_ext == right->headless_ext && left->reserved[0] == right->reserved[0] &&
        left->reserved[1] == right->reserved[1];
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    LifecycleState state;
    CNA_PresentationParameters initial;
    CNA_PresentationParameters changed;
    CNA_PresentationParameters observed;
    CNA_Handle first = CNA_INVALID_HANDLE;
    CNA_Handle second = CNA_INVALID_HANDLE;
    CNA_Handle rejected = CNA_INVALID_HANDLE;
    CNA_GraphicsDeviceEventRegistrationHandle resetting = CNA_INVALID_HANDLE;
    CNA_GraphicsDeviceEventRegistrationHandle reset = CNA_INVALID_HANDLE;
    CNA_GraphicsDeviceEventRegistrationHandle lost = CNA_INVALID_HANDLE;
    CNA_GraphicsDeviceEventRegistrationHandle invalid_registration = CNA_INVALID_HANDLE;
    CNA_Bool disposed = CNA_TRUE;
    CNA_GraphicsProfile profile = UINT32_MAX;
    uint32_t adapter_index = 0U;
    int sequence_before;

    cna_demo_init(&demo, "c_api.graphics.owned_device", argc, argv, 0);
    memset(&state, 0, sizeof(state));
    state.device = CNA_INVALID_HANDLE;
    state.disposing_registration = CNA_INVALID_HANDLE;
    state.released_disposing_registration = CNA_INVALID_HANDLE;
    state.disposing_unsubscribe_result = CNA_RESULT_INVALID_STATE;

    memset(&initial, 0, sizeof(initial));
    cna_demo_check(
        &demo,
        cna_presentation_parameters_init(&initial) == CNA_RESULT_SUCCESS &&
            initial.struct_size == sizeof(initial) && initial.struct_version == UINT32_C(1),
        "initialize complete presentation parameters for a device outside any Game");
    initial.back_buffer_width = 320;
    initial.back_buffer_height = 240;
    initial.depth_stencil_format = CNA_DEPTH_FORMAT_DEPTH24_STENCIL8;
    initial.presentation_interval = CNA_PRESENT_INTERVAL_IMMEDIATE;

    cna_demo_check(
        &demo,
        cna_graphics_device_create(
            0U, UINT32_C(99), &initial, &rejected) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE &&
            cna_graphics_device_create(
                UINT32_C(4096), CNA_GRAPHICS_PROFILE_HI_DEF, &initial, &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "owned-device creation rejects an unknown profile and adapter before publishing a handle");

    if (!cna_demo_check_result(
            &demo,
            cna_graphics_device_create(
                0U, CNA_GRAPHICS_PROFILE_HI_DEF, &initial, &first),
            "create the first caller-owned graphics device")) {
        return cna_demo_finish(&demo);
    }
    state.device = first;
    if (!cna_demo_check_result(
            &demo,
            cna_graphics_device_create(
                0U, CNA_GRAPHICS_PROFILE_HI_DEF, &initial, &second),
            "create a second independent graphics device while the first remains live")) {
        (void)cna_graphics_device_destroy(first);
        return cna_demo_finish(&demo);
    }
    cna_demo_check(&demo, second != first, "the two owned devices have distinct handle identities");

    memset(&observed, 0, sizeof(observed));
    observed.struct_size = (uint32_t)sizeof(observed);
    observed.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_graphics_device_get_is_disposed(first, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_graphics_device_get_graphics_profile(first, &profile) == CNA_RESULT_SUCCESS &&
            profile == CNA_GRAPHICS_PROFILE_HI_DEF &&
            cna_graphics_device_get_presentation_parameters(first, &observed) ==
                CNA_RESULT_SUCCESS &&
            observed.back_buffer_width == initial.back_buffer_width &&
            observed.back_buffer_height == initial.back_buffer_height &&
            observed.depth_stencil_format == initial.depth_stencil_format,
        "the first owned device answers ordinary profile, status and presentation queries");

    cna_demo_check(
        &demo,
        cna_graphics_device_subscribe_event(
            first, CNA_GRAPHICS_DEVICE_EVENT_DEVICE_RESETTING, on_resetting, &state,
            &resetting) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_subscribe_event(
                first, CNA_GRAPHICS_DEVICE_EVENT_DEVICE_RESET, on_reset, &state, &reset) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_subscribe_event(
                first, CNA_GRAPHICS_DEVICE_EVENT_DEVICE_LOST, on_lost, &state, &lost) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_subscribe_event(
                first, CNA_GRAPHICS_DEVICE_EVENT_DISPOSING, on_disposing, &state,
                &state.disposing_registration) == CNA_RESULT_SUCCESS,
        "subscribe synchronous resetting, reset, lost and disposing callbacks");
    cna_demo_check(
        &demo,
        cna_graphics_device_subscribe_event(
            first, UINT32_C(4), on_reset, &state, &invalid_registration) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            invalid_registration == CNA_INVALID_HANDLE,
        "an undefined device-event identity is refused without a registration");

    cna_demo_check_result(
        &demo, cna_graphics_device_clear_rgba(first, 0.0F, 0.0F, 0.0F, 1.0F),
        "clear the caller-owned device before presentation");
    cna_demo_check_result(
        &demo, cna_graphics_device_present(first),
        "present one completed frame from the caller-owned device");

    cna_demo_check_result(
        &demo, cna_graphics_device_reset(first),
        "reset the caller-owned device with its current presentation parameters");
    cna_demo_check(
        &demo,
        state.sequence_count == 2 && state.sequence[0] == 1 && state.sequence[1] == 2 &&
            state.lost_count == 0 && state.disposing_count == 0 &&
            state.wrong_device_count == 0,
        "the first reset synchronously raises resetting then reset on the exact device");

    changed = initial;
    changed.back_buffer_width = 400;
    changed.back_buffer_height = 300;
    changed.presentation_interval = CNA_PRESENT_INTERVAL_ONE;
    cna_demo_check_result(
        &demo,
        cna_graphics_device_reset_with_parameters(first, &changed, &adapter_index),
        "reset the owned device with changed dimensions, interval and an explicit adapter");
    memset(&observed, 0, sizeof(observed));
    observed.struct_size = (uint32_t)sizeof(observed);
    observed.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        state.sequence_count == 4 && state.sequence[2] == 1 && state.sequence[3] == 2 &&
            state.lost_count == 0 && state.disposing_count == 0 &&
            cna_graphics_device_get_presentation_parameters(first, &observed) ==
                CNA_RESULT_SUCCESS &&
            parameters_equal_except_size(&observed, &changed),
        "the parameterized reset raises the same ordered pair and applies every requested field");

    sequence_before = state.sequence_count;
    cna_demo_check(
        &demo,
        cna_graphics_device_reset_with_parameters(first, NULL, NULL) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            state.sequence_count == sequence_before,
        "an invalid parameterized reset is refused before raising lifecycle events");

    cna_demo_check(
        &demo,
        cna_graphics_device_unsubscribe(resetting) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_unsubscribe(resetting) == CNA_RESULT_INVALID_HANDLE &&
            cna_graphics_device_unsubscribe(reset) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_unsubscribe(lost) == CNA_RESULT_SUCCESS,
        "release reset/lost subscriptions exactly once while their event source remains live");
    resetting = CNA_INVALID_HANDLE;
    reset = CNA_INVALID_HANDLE;
    lost = CNA_INVALID_HANDLE;

    cna_demo_check(
        &demo,
        cna_graphics_device_destroy(second) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_is_disposed(first, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE,
        "destroy the second owned device without affecting the first");
    second = CNA_INVALID_HANDLE;

    cna_demo_check_result(
        &demo, cna_graphics_device_destroy(first),
        "destroy the first owned device and synchronously dispose it");
    cna_demo_check(
        &demo,
        state.disposing_count == 1 && state.lost_count == 0 &&
            state.wrong_device_count == 0 &&
            state.disposing_unsubscribe_result == CNA_RESULT_SUCCESS &&
            state.disposing_registration == CNA_INVALID_HANDLE &&
            state.released_disposing_registration != CNA_INVALID_HANDLE,
        "the disposing callback receives the exact device and releases its live registration");
    first = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_graphics_device_unsubscribe(state.released_disposing_registration) ==
                CNA_RESULT_INVALID_HANDLE &&
            cna_graphics_device_destroy(state.device) == CNA_RESULT_INVALID_HANDLE &&
            cna_graphics_device_get_is_disposed(state.device, &disposed) ==
                CNA_RESULT_INVALID_HANDLE,
        "released registrations and destroyed device handles are stale immediately");

    if (lost != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_unsubscribe(lost);
    }
    if (reset != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_unsubscribe(reset);
    }
    if (resetting != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_unsubscribe(resetting);
    }
    if (state.disposing_registration != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_unsubscribe(state.disposing_registration);
    }
    if (second != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_destroy(second);
    }
    if (first != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_destroy(first);
    }
    return cna_demo_finish(&demo);
}
