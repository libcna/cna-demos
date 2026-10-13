/* SPDX-License-Identifier: MS-PL */
/*
 * Graphics-device vertex/index buffer bindings from C. Single and multi-stream bindings retain
 * exact owned-handle identities, offsets and instance frequencies; copying is atomic and destroying
 * a bound resource detaches the affected device state.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct BufferBindingDemo {
    CnaDemo* demo;
    int loads;
} BufferBindingDemo;

static int binding_equal(CNA_VertexBufferBinding left, CNA_VertexBufferBinding right)
{
    return left.vertex_buffer == right.vertex_buffer &&
        left.vertex_offset == right.vertex_offset &&
        left.instance_frequency == right.instance_frequency;
}

static int create_resources(CNA_Handle device, CNA_VertexDeclarationHandle* out_declaration,
                            CNA_VertexBufferHandle* out_first,
                            CNA_VertexBufferHandle* out_second,
                            CNA_IndexBufferHandle* out_index)
{
    CNA_VertexElement elements[8];
    uint64_t element_count = UINT64_C(0);
    uint32_t stride = 0U;
    CNA_VertexBufferCreateInfo vertex_info;
    CNA_IndexBufferCreateInfo index_info;
    if (cna_vertex_type_get_stride(CNA_VERTEX_TYPE_POSITION_COLOR, &stride) !=
            CNA_RESULT_SUCCESS ||
        cna_vertex_type_copy_elements(
            CNA_VERTEX_TYPE_POSITION_COLOR, elements,
            (uint64_t)(sizeof(elements) / sizeof(elements[0])), &element_count) !=
            CNA_RESULT_SUCCESS ||
        cna_vertex_declaration_create_with_stride(
            (int32_t)stride, elements, element_count, out_declaration) != CNA_RESULT_SUCCESS) {
        return 0;
    }

    memset(&vertex_info, 0, sizeof(vertex_info));
    vertex_info.struct_size = (uint32_t)sizeof(vertex_info);
    vertex_info.struct_version = UINT32_C(1);
    vertex_info.vertex_declaration = *out_declaration;
    vertex_info.vertex_count = 6;
    vertex_info.buffer_usage = CNA_BUFFER_USAGE_NONE;
    vertex_info.dynamic = CNA_FALSE;
    if (cna_vertex_buffer_create(device, &vertex_info, out_first) != CNA_RESULT_SUCCESS ||
        cna_vertex_buffer_create(device, &vertex_info, out_second) != CNA_RESULT_SUCCESS) {
        return 0;
    }

    memset(&index_info, 0, sizeof(index_info));
    index_info.struct_size = (uint32_t)sizeof(index_info);
    index_info.struct_version = UINT32_C(1);
    index_info.index_count = 6;
    index_info.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    index_info.buffer_usage = CNA_BUFFER_USAGE_NONE;
    index_info.dynamic = CNA_FALSE;
    return cna_index_buffer_create(device, &index_info, out_index) == CNA_RESULT_SUCCESS;
}

static int read_vertex_bindings(CNA_Handle device, CNA_VertexBufferBinding* destination,
                                uint64_t capacity, uint64_t* out_count)
{
    return cna_graphics_device_copy_vertex_buffers(
               device, destination, capacity, out_count) == CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    BufferBindingDemo* state = (BufferBindingDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_VertexDeclarationHandle declaration = CNA_INVALID_HANDLE;
    CNA_VertexBufferHandle first = CNA_INVALID_HANDLE;
    CNA_VertexBufferHandle second = CNA_INVALID_HANDLE;
    CNA_IndexBufferHandle index = CNA_INVALID_HANDLE;
    CNA_VertexBufferHandle first_slot = UINT64_C(77);
    CNA_IndexBufferHandle active_index = UINT64_C(88);
    CNA_VertexBufferBinding copied[2];
    CNA_VertexBufferBinding guard[2];
    CNA_VertexBufferBinding requested[2];
    CNA_VertexBufferBinding invalid[2];
    uint64_t count = UINT64_MAX;
    CNA_Result first_destroy_result;
    CNA_Result second_destroy_result;
    CNA_Result index_destroy_result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    cna_demo_check(
        state->demo,
        cna_game_get_graphics_device(game, &device) == CNA_RESULT_SUCCESS &&
            device != CNA_INVALID_HANDLE,
        "borrow the live graphics device for this callback");
    cna_demo_check(
        state->demo,
        cna_graphics_device_get_vertex_buffer_count(device, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_graphics_device_get_vertex_buffer(device, &first_slot) == CNA_RESULT_SUCCESS &&
            first_slot == CNA_INVALID_HANDLE &&
            cna_graphics_device_get_index_buffer(device, &active_index) == CNA_RESULT_SUCCESS &&
            active_index == CNA_INVALID_HANDLE,
        "the fresh device begins with no vertex or index buffer bound");

    cna_demo_check(
        state->demo,
        create_resources(device, &declaration, &first, &second, &index) &&
            declaration != CNA_INVALID_HANDLE && first != CNA_INVALID_HANDLE &&
            second != CNA_INVALID_HANDLE && index != CNA_INVALID_HANDLE,
        "create one declaration, two vertex buffers and one index buffer");
    if (first == CNA_INVALID_HANDLE || second == CNA_INVALID_HANDLE ||
        index == CNA_INVALID_HANDLE) {
        goto cleanup;
    }

    memset(copied, 0xA5, sizeof(copied));
    count = UINT64_MAX;
    first_slot = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_vertex_buffer(device, first) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_vertex_buffer(device, &first_slot) == CNA_RESULT_SUCCESS &&
            first_slot == first &&
            cna_graphics_device_get_vertex_buffer_count(device, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) && read_vertex_bindings(device, copied, UINT64_C(2), &count) &&
            count == UINT64_C(1) &&
            binding_equal(copied[0], (CNA_VertexBufferBinding){first, 0, 0}),
        "the single-buffer route binds the first owned handle at offset zero");

    memset(copied, 0xA5, sizeof(copied));
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_vertex_buffer_offset(device, second, 2) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_vertex_buffer(device, &first_slot) == CNA_RESULT_SUCCESS &&
            first_slot == second && read_vertex_bindings(device, copied, UINT64_C(2), &count) &&
            count == UINT64_C(1) &&
            binding_equal(copied[0], (CNA_VertexBufferBinding){second, 2, 0}),
        "the offset overload replaces the binding and preserves its vertex offset");

    requested[0] = (CNA_VertexBufferBinding){first, 1, 0};
    requested[1] = (CNA_VertexBufferBinding){second, 2, 3};
    memset(copied, 0xA5, sizeof(copied));
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_vertex_buffers(device, requested, UINT64_C(2)) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_vertex_buffer_count(device, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_graphics_device_get_vertex_buffer(device, &first_slot) == CNA_RESULT_SUCCESS &&
            first_slot == first && read_vertex_bindings(device, copied, UINT64_C(2), &count) &&
            count == UINT64_C(2) && binding_equal(copied[0], requested[0]) &&
            binding_equal(copied[1], requested[1]),
        "the multi-stream route preserves order, offsets and instance frequencies");

    guard[0] = (CNA_VertexBufferBinding){UINT64_C(91), 92, 93};
    guard[1] = (CNA_VertexBufferBinding){UINT64_C(94), 95, 96};
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_graphics_device_copy_vertex_buffers(device, guard, UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) &&
            binding_equal(guard[0], (CNA_VertexBufferBinding){UINT64_C(91), 92, 93}) &&
            binding_equal(guard[1], (CNA_VertexBufferBinding){UINT64_C(94), 95, 96}),
        "a short binding destination reports the full count without a partial copy");

    invalid[0] = requested[0];
    invalid[1] = (CNA_VertexBufferBinding){second, -1, 3};
    memset(copied, 0xA5, sizeof(copied));
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_vertex_buffers(device, invalid, UINT64_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            read_vertex_bindings(device, copied, UINT64_C(2), &count) &&
            count == UINT64_C(2) && binding_equal(copied[0], requested[0]) &&
            binding_equal(copied[1], requested[1]),
        "an invalid later binding is refused before the applied array changes");

    active_index = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_index_buffer(device, index) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_index_buffer(device, &active_index) == CNA_RESULT_SUCCESS &&
            active_index == index,
        "the index-buffer getter returns the exact owned handle applied to the device");
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_vertex_buffers(device, NULL, UINT64_C(0)) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_index_buffer(device, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_vertex_buffer_count(device, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_graphics_device_get_vertex_buffer(device, &first_slot) == CNA_RESULT_SUCCESS &&
            first_slot == CNA_INVALID_HANDLE &&
            cna_graphics_device_get_index_buffer(device, &active_index) == CNA_RESULT_SUCCESS &&
            active_index == CNA_INVALID_HANDLE,
        "empty vertex bindings and an invalid index handle clear the complete device state");

    cna_demo_check(
        state->demo,
        cna_graphics_device_set_vertex_buffers(device, requested, UINT64_C(2)) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_index_buffer(device, index) == CNA_RESULT_SUCCESS,
        "rebind both vertex streams and the index buffer before destroying their owners");
    first_destroy_result = cna_vertex_buffer_destroy(first);
    if (first_destroy_result == CNA_RESULT_SUCCESS) {
        first = CNA_INVALID_HANDLE;
    }
    index_destroy_result = cna_index_buffer_destroy(index);
    if (index_destroy_result == CNA_RESULT_SUCCESS) {
        index = CNA_INVALID_HANDLE;
    }
    count = UINT64_MAX;
    first_slot = UINT64_C(77);
    active_index = UINT64_C(88);
    cna_demo_check(
        state->demo,
        first_destroy_result == CNA_RESULT_SUCCESS &&
            index_destroy_result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_vertex_buffer_count(device, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_graphics_device_get_vertex_buffer(device, &first_slot) == CNA_RESULT_SUCCESS &&
            first_slot == CNA_INVALID_HANDLE &&
            cna_graphics_device_get_index_buffer(device, &active_index) == CNA_RESULT_SUCCESS &&
            active_index == CNA_INVALID_HANDLE,
        "destroying bound owners detaches the whole vertex set and the index binding");

cleanup:
    if (index != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_set_index_buffer(device, CNA_INVALID_HANDLE);
        (void)cna_index_buffer_destroy(index);
    }
    if (first != CNA_INVALID_HANDLE || second != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_set_vertex_buffers(device, NULL, UINT64_C(0));
    }
    if (first != CNA_INVALID_HANDLE) {
        (void)cna_vertex_buffer_destroy(first);
    }
    second_destroy_result = second == CNA_INVALID_HANDLE
        ? CNA_RESULT_SUCCESS
        : cna_vertex_buffer_destroy(second);
    cna_demo_check(
        state->demo, second_destroy_result == CNA_RESULT_SUCCESS,
        "destroy the remaining unbound vertex buffer");
    if (declaration != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            state->demo, cna_vertex_declaration_destroy(declaration),
            "destroy the shared vertex declaration");
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
    BufferBindingDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.buffer_bindings", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
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
        "create the graphics device whose buffer bindings are audited");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the buffer-binding audit ran in exactly one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
