/* SPDX-License-Identifier: MS-PL */
/*
 * Dynamic 16-bit IndexBuffer transfer behavior from C. Source and destination descriptors select
 * windows in caller arrays, while set_data_at's byte offset selects a window in the buffer.
 * ContentLost is renderer-specific and remains silent during ordinary transfers on current Linux
 * renderers.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct IndexBufferState {
    CnaDemo* demo;
    CNA_IndexBufferHandle expected_buffer;
    int content_lost_calls;
    int matching_content_lost_calls;
    int loads;
} IndexBufferState;

static CNA_IndexBufferTransfer make_transfer(uint64_t start_index, uint64_t element_count)
{
    CNA_IndexBufferTransfer transfer;
    memset(&transfer, 0, sizeof(transfer));
    transfer.struct_size = (uint32_t)sizeof(transfer);
    transfer.struct_version = UINT32_C(1);
    transfer.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    transfer.options = CNA_SET_DATA_NONE;
    transfer.start_index = start_index;
    transfer.element_count = element_count;
    return transfer;
}

static void content_lost(CNA_IndexBufferHandle index_buffer, void* context)
{
    IndexBufferState* state = (IndexBufferState*)context;
    ++state->content_lost_calls;
    if (index_buffer == state->expected_buffer) {
        ++state->matching_content_lost_calls;
    }
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    static const uint16_t InitialSource[] = {
        UINT16_C(999), UINT16_C(10), UINT16_C(20), UINT16_C(30), UINT16_C(40)};
    static const uint16_t InitialDestination[] = {
        UINT16_C(901), UINT16_C(902), UINT16_C(903),
        UINT16_C(904), UINT16_C(905), UINT16_C(906)};
    static const uint16_t PatchSource[] = {
        UINT16_C(777), UINT16_C(70), UINT16_C(80)};
    IndexBufferState* state = (IndexBufferState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_IndexBufferHandle buffer = CNA_INVALID_HANDLE;
    CNA_IndexBufferEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_IndexBufferCreateInfo create_info;
    CNA_IndexBufferInfo info;
    CNA_IndexBufferTransfer upload;
    CNA_IndexBufferTransfer readback;
    CNA_IndexBufferTransfer patch;
    uint16_t destination[6];
    uint16_t after_patch[5];
    uint64_t required = UINT64_MAX;
    CNA_Result unsubscribe_result;
    CNA_Result destroy_result;
    int ready;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    ready = cna_demo_check_result(
        state->demo, cna_game_get_graphics_device(game, &device),
        "borrow the live graphics device for this callback");

    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.index_count = 4;
    create_info.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    create_info.buffer_usage = CNA_BUFFER_USAGE_NONE;
    create_info.dynamic = CNA_TRUE;
    ready = cna_demo_check(
        state->demo,
        ready && cna_index_buffer_create(device, &create_info, &buffer) == CNA_RESULT_SUCCESS &&
            buffer != CNA_INVALID_HANDLE,
        "create a four-index dynamic 16-bit buffer") && ready;
    if (!ready) {
        return CNA_RESULT_SUCCESS;
    }
    state->expected_buffer = buffer;

    memset(&info, 0xA5, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        cna_index_buffer_get_info(buffer, &info) == CNA_RESULT_SUCCESS &&
            info.index_count == 4 &&
            info.index_element_size == CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS &&
            info.buffer_usage == CNA_BUFFER_USAGE_NONE && info.dynamic == CNA_TRUE &&
            info.is_content_lost == CNA_FALSE && info.has_renderer == CNA_TRUE &&
            info.reserved == 0U,
        "report the complete applied dynamic buffer state and live renderer allocation");

    cna_demo_check(
        state->demo,
        cna_index_buffer_subscribe_content_lost(
            buffer, content_lost, state, &registration) == CNA_RESULT_SUCCESS &&
            registration != CNA_INVALID_HANDLE,
        "subscribe an owned registration to the dynamic buffer's ContentLost event");

    upload = make_transfer(UINT64_C(1), UINT64_C(4));
    cna_demo_check(
        state->demo,
        cna_index_buffer_set_data(
            buffer, &upload, InitialSource,
            (uint64_t)(sizeof(InitialSource) / sizeof(InitialSource[0]))) == CNA_RESULT_SUCCESS,
        "upload four indices from a nonzero caller-array window");

    memcpy(destination, InitialDestination, sizeof(destination));
    readback = make_transfer(UINT64_C(2), UINT64_C(4));
    required = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_index_buffer_get_data(
            buffer, &readback, destination, UINT64_C(5), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == UINT64_C(4) &&
            memcmp(destination, InitialDestination, sizeof(destination)) == 0,
        "a short destination reports the element count without a partial write");

    required = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_index_buffer_get_data(
            buffer, &readback, destination,
            (uint64_t)(sizeof(destination) / sizeof(destination[0])), &required) ==
                CNA_RESULT_SUCCESS &&
            required == UINT64_C(4) && destination[0] == InitialDestination[0] &&
            destination[1] == InitialDestination[1] && destination[2] == UINT16_C(10) &&
            destination[3] == UINT16_C(20) && destination[4] == UINT16_C(30) &&
            destination[5] == UINT16_C(40),
        "an exact destination preserves its prefix and receives all indices at the selected start");

    patch = make_transfer(UINT64_C(1), UINT64_C(2));
    cna_demo_check(
        state->demo,
        cna_index_buffer_set_data_at(
            buffer, UINT64_C(2) * sizeof(uint16_t), &patch, PatchSource,
            (uint64_t)(sizeof(PatchSource) / sizeof(PatchSource[0]))) == CNA_RESULT_SUCCESS,
        "patch two caller-selected values at the buffer's four-byte offset");

    memset(after_patch, 0xCC, sizeof(after_patch));
    readback = make_transfer(UINT64_C(1), UINT64_C(4));
    required = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_index_buffer_get_data(
            buffer, &readback, after_patch,
            (uint64_t)(sizeof(after_patch) / sizeof(after_patch[0])), &required) ==
                CNA_RESULT_SUCCESS &&
            required == UINT64_C(4) && after_patch[0] == UINT16_C(0xCCCC) &&
            after_patch[1] == UINT16_C(10) && after_patch[2] == UINT16_C(20) &&
            after_patch[3] == UINT16_C(70) && after_patch[4] == UINT16_C(80),
        "partial upload preserves the two indices outside its buffer window");

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        cna_index_buffer_get_info(buffer, &info) == CNA_RESULT_SUCCESS &&
            info.is_content_lost == CNA_FALSE && info.has_renderer == CNA_TRUE &&
            state->content_lost_calls == 0 && state->matching_content_lost_calls == 0,
        "ordinary transfers leave content live and raise no renderer-specific loss event");

    unsubscribe_result = cna_index_buffer_unsubscribe_content_lost(registration);
    cna_demo_check(
        state->demo,
        unsubscribe_result == CNA_RESULT_SUCCESS &&
            cna_index_buffer_unsubscribe_content_lost(registration) == CNA_RESULT_INVALID_HANDLE,
        "the ContentLost registration releases once and its stale handle is refused");
    if (unsubscribe_result == CNA_RESULT_SUCCESS) {
        registration = CNA_INVALID_HANDLE;
    }

    destroy_result = cna_index_buffer_destroy(buffer);
    cna_demo_check(
        state->demo, destroy_result == CNA_RESULT_SUCCESS,
        "destroy the owned dynamic index buffer after its transfers complete");
    if (destroy_result == CNA_RESULT_SUCCESS) {
        buffer = CNA_INVALID_HANDLE;
        state->expected_buffer = CNA_INVALID_HANDLE;
    }

    if (registration != CNA_INVALID_HANDLE) {
        (void)cna_index_buffer_unsubscribe_content_lost(registration);
    }
    if (buffer != CNA_INVALID_HANDLE) {
        (void)cna_index_buffer_destroy(buffer);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    IndexBufferState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.index_buffer", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.expected_buffer = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_FALSE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }

    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the graphics device that owns the index buffer");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the index-buffer workflow ran in exactly one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
