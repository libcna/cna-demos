/* SPDX-License-Identifier: MS-PL */
/*
 * The generic GraphicsResource contract from C, demonstrated through one real device-owned
 * Texture2D. Native resource disposal is synchronous, observable and idempotent while the C handle
 * remains alive; typed destruction later releases that handle. Names use validated UTF-8 and all
 * string copies are exact byte sequences without terminators.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ResourceLifecycleState {
    CnaDemo* demo;
    CNA_Handle expected_resource;
    int disposing_calls;
    int callback_saw_live_resource;
    int loads;
} ResourceLifecycleState;

static int all_bytes_are(const void* data, size_t count, unsigned char value)
{
    const unsigned char* bytes = (const unsigned char*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

static void disposing(CNA_Handle resource, void* context)
{
    ResourceLifecycleState* state = (ResourceLifecycleState*)context;
    CNA_Bool is_disposed = CNA_TRUE;
    ++state->disposing_calls;
    if (resource == state->expected_resource &&
        cna_graphics_resource_get_is_disposed(resource, &is_disposed) == CNA_RESULT_SUCCESS &&
        is_disposed == CNA_FALSE) {
        ++state->callback_saw_live_resource;
    }
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    static const char Name[] = {
        'C', ' ', 't', 'e', 'x', 't', 'u', 'r', 'e', ' ',
        (char)0xE2, (char)0x98, (char)0x85};
    static const char EmbeddedNull[] = {'x', '\0', 'y'};
    ResourceLifecycleState* state = (ResourceLifecycleState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle owner = UINT64_C(91);
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo create_info;
    CNA_GraphicsResourceEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_GraphicsResourceTag tag = UINT64_MAX;
    CNA_Bool is_disposed = CNA_TRUE;
    uint64_t bytes = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    char type_text[128];
    char short_name[sizeof(Name)];
    char name_copy[sizeof(Name) + 1U];
    char string_copy[sizeof(Name) + 1U];
    CNA_Result borrow_result;
    CNA_Result create_result;
    CNA_Result copy_result = CNA_RESULT_INVALID_STATE;
    CNA_Result dispose_result;
    CNA_Result destroy_result;
    int ready;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    borrow_result = cna_game_get_graphics_device(game, &device);
    cna_demo_check(state->demo, borrow_result == CNA_RESULT_SUCCESS,
                   "borrow the live graphics device for this callback");
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.width = 2U;
    create_info.height = 2U;
    create_info.format = CNA_SURFACE_FORMAT_COLOR;
    create_result = borrow_result == CNA_RESULT_SUCCESS
        ? cna_texture2d_create(device, &create_info, &texture)
        : CNA_RESULT_INVALID_STATE;
    ready = cna_demo_check(
        state->demo, create_result == CNA_RESULT_SUCCESS && texture != CNA_INVALID_HANDLE,
        "create a real device-owned Texture2D graphics resource");
    if (!ready) {
        return CNA_RESULT_SUCCESS;
    }
    state->expected_resource = texture;

    cna_demo_check(
        state->demo,
        cna_graphics_resource_get_graphics_device(texture, &owner) == CNA_RESULT_SUCCESS &&
            owner == device,
        "the generic resource view returns its callback-scoped owning graphics device");
    cna_demo_check(
        state->demo,
        cna_graphics_resource_get_is_disposed(texture, &is_disposed) == CNA_RESULT_SUCCESS &&
            is_disposed == CNA_FALSE &&
            cna_graphics_resource_get_tag(texture, &tag) == CNA_RESULT_SUCCESS &&
            tag == UINT64_C(0),
        "a fresh resource is live and carries the null C-owned tag token");

    bytes = UINT64_MAX;
    required = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_graphics_resource_get_name_byte_count(texture, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0) &&
            cna_graphics_resource_copy_name(texture, NULL, UINT64_C(0), &required) ==
                CNA_RESULT_SUCCESS &&
            required == UINT64_C(0),
        "the default resource name is the exact empty byte sequence");

    memset(type_text, 0x5A, sizeof(type_text));
    bytes = UINT64_C(0);
    if (cna_graphics_resource_get_string_byte_count(texture, &bytes) == CNA_RESULT_SUCCESS &&
        bytes > UINT64_C(1) && bytes < (uint64_t)sizeof(type_text)) {
        required = UINT64_C(0);
        copy_result = cna_graphics_resource_copy_string(
            texture, type_text, UINT64_C(1), &required);
    }
    cna_demo_check(
        state->demo,
        copy_result == CNA_RESULT_BUFFER_TOO_SMALL && required == bytes &&
            all_bytes_are(type_text, sizeof(type_text), 0x5A),
        "the unnamed resource's type string reports its size without a partial write");
    if (bytes < (uint64_t)sizeof(type_text)) {
        required = UINT64_C(0);
        copy_result = cna_graphics_resource_copy_string(texture, type_text, bytes, &required);
        if (copy_result == CNA_RESULT_SUCCESS && required == bytes) {
            type_text[bytes] = '\0';
        }
    }
    cna_demo_check(
        state->demo,
        copy_result == CNA_RESULT_SUCCESS && required == bytes &&
            strlen(type_text) == (size_t)bytes,
        "an exact buffer receives the unnamed resource's complete native type string");
    if (copy_result == CNA_RESULT_SUCCESS) {
        printf("  unnamed resource string: %s\n", type_text);
    }

    cna_demo_check(
        state->demo,
        cna_graphics_resource_set_name(
            texture, (CNA_StringView){Name, (uint64_t)sizeof(Name)}) == CNA_RESULT_SUCCESS,
        "set a validated non-ASCII UTF-8 resource name");
    cna_demo_check(
        state->demo,
        cna_graphics_resource_set_name(
            texture, (CNA_StringView){EmbeddedNull, (uint64_t)sizeof(EmbeddedNull)}) ==
                CNA_RESULT_ENCODING,
        "an embedded NUL is refused without replacing the resource name");

    memset(short_name, 0x6B, sizeof(short_name));
    bytes = UINT64_C(0);
    required = UINT64_C(0);
    cna_demo_check(
        state->demo,
        cna_graphics_resource_get_name_byte_count(texture, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(Name) &&
            cna_graphics_resource_copy_name(
                texture, short_name, sizeof(Name) - 1U, &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == sizeof(Name) &&
            all_bytes_are(short_name, sizeof(short_name), 0x6B),
        "a short name buffer reports the UTF-8 byte count without a partial write");

    memset(name_copy, 0x7C, sizeof(name_copy));
    required = UINT64_C(0);
    cna_demo_check(
        state->demo,
        cna_graphics_resource_copy_name(
            texture, name_copy, sizeof(Name), &required) == CNA_RESULT_SUCCESS &&
            required == sizeof(Name) && memcmp(name_copy, Name, sizeof(Name)) == 0 &&
            (unsigned char)name_copy[sizeof(Name)] == 0x7C,
        "an exact name copy preserves all UTF-8 bytes and appends no terminator");

    memset(string_copy, 0x4D, sizeof(string_copy));
    bytes = UINT64_C(0);
    required = UINT64_C(0);
    cna_demo_check(
        state->demo,
        cna_graphics_resource_get_string_byte_count(texture, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(Name) &&
            cna_graphics_resource_copy_string(
                texture, string_copy, sizeof(Name), &required) == CNA_RESULT_SUCCESS &&
            required == sizeof(Name) && memcmp(string_copy, Name, sizeof(Name)) == 0 &&
            (unsigned char)string_copy[sizeof(Name)] == 0x4D,
        "a named resource's string representation is exactly its name without a terminator");

    tag = UINT64_C(0);
    cna_demo_check(
        state->demo,
        cna_graphics_resource_set_tag(
            texture, UINT64_C(0x0123456789abcdef)) == CNA_RESULT_SUCCESS &&
            cna_graphics_resource_get_tag(texture, &tag) == CNA_RESULT_SUCCESS &&
            tag == UINT64_C(0x0123456789abcdef),
        "the full-width opaque C tag token round-trips unchanged");

    cna_demo_check(
        state->demo,
        cna_graphics_resource_subscribe_disposing(
            texture, disposing, state, &registration) == CNA_RESULT_SUCCESS &&
            registration != CNA_INVALID_HANDLE,
        "subscribe an owned registration to the resource's Disposing event");
    dispose_result = cna_graphics_resource_dispose(texture);
    cna_demo_check(
        state->demo,
        dispose_result == CNA_RESULT_SUCCESS && state->disposing_calls == 1 &&
            state->callback_saw_live_resource == 1,
        "explicit disposal synchronously raises one callback before the disposed state changes");
    is_disposed = CNA_FALSE;
    cna_demo_check(
        state->demo,
        cna_graphics_resource_get_is_disposed(texture, &is_disposed) == CNA_RESULT_SUCCESS &&
            is_disposed == CNA_TRUE,
        "the still-live C handle observes the native resource as disposed after the callback");
    cna_demo_check(
        state->demo,
        cna_graphics_resource_dispose(texture) == CNA_RESULT_SUCCESS &&
            state->disposing_calls == 1,
        "repeated explicit disposal is an idempotent no-op with no second event");
    cna_demo_check(
        state->demo,
        cna_graphics_resource_unsubscribe_disposing(registration) == CNA_RESULT_SUCCESS &&
            cna_graphics_resource_unsubscribe_disposing(registration) == CNA_RESULT_INVALID_HANDLE,
        "the event registration releases once and its stale handle is refused");
    registration = CNA_INVALID_HANDLE;

    destroy_result = cna_texture2d_destroy(texture);
    cna_demo_check(
        state->demo,
        destroy_result == CNA_RESULT_SUCCESS &&
            cna_graphics_resource_get_is_disposed(texture, &is_disposed) ==
                CNA_RESULT_INVALID_HANDLE,
        "typed destruction releases the separate C handle after native disposal");
    if (destroy_result == CNA_RESULT_SUCCESS) {
        texture = CNA_INVALID_HANDLE;
        state->expected_resource = CNA_INVALID_HANDLE;
    }

    if (registration != CNA_INVALID_HANDLE) {
        (void)cna_graphics_resource_unsubscribe_disposing(registration);
    }
    if (texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(texture);
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
    ResourceLifecycleState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.resource_lifecycle", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.expected_resource = CNA_INVALID_HANDLE;
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
        "create the graphics device that owns the resource");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the resource lifecycle ran in exactly one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
