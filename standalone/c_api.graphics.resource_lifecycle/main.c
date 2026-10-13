/* SPDX-License-Identifier: MS-PL */
/*
 * The generic GraphicsResource contract from C, demonstrated through one real device-owned
 * Texture2D. Device-level creation/destruction events and resource tracking surround the generic
 * resource's synchronous, observable and idempotent disposal while the C handle remains alive;
 * typed destruction later releases that handle. Names use validated UTF-8 and all string copies
 * are exact byte sequences without terminators.
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
    CNA_Handle expected_device;
    CNA_Handle expected_resource;
    uint64_t tracked_baseline;
    int resource_created_calls;
    int resource_created_payloads;
    int resource_created_saw_tracking;
    int resource_destroyed_calls;
    int resource_destroyed_payloads;
    int resource_destroyed_saw_tracking;
    int disposing_calls;
    int callback_saw_live_resource;
    int loads;
} ResourceLifecycleState;

static const char ResourceName[] = {
    'C', ' ', 't', 'e', 'x', 't', 'u', 'r', 'e', ' ',
    (char)0xE2, (char)0x98, (char)0x85};

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

static void resource_created(CNA_Handle device, const CNA_ResourceCreatedEventInfo* info,
                             void* context)
{
    ResourceLifecycleState* state = (ResourceLifecycleState*)context;
    uint64_t tracked = UINT64_MAX;
    ++state->resource_created_calls;
    if (device == state->expected_device && info != NULL &&
        info->struct_size == (uint32_t)sizeof(*info) && info->struct_version == UINT32_C(1) &&
        info->has_resource == CNA_TRUE &&
        all_bytes_are(info->reserved, sizeof(info->reserved), 0U)) {
        ++state->resource_created_payloads;
    }
    if (cna_graphics_device_get_tracked_resource_count(device, &tracked) == CNA_RESULT_SUCCESS &&
        tracked == state->tracked_baseline + UINT64_C(1)) {
        ++state->resource_created_saw_tracking;
    }
}

static void resource_destroyed(CNA_Handle device, const CNA_ResourceDestroyedEventInfo* info,
                               void* context)
{
    ResourceLifecycleState* state = (ResourceLifecycleState*)context;
    uint64_t tracked = UINT64_MAX;
    ++state->resource_destroyed_calls;
    if (device == state->expected_device && info != NULL &&
        info->struct_size == (uint32_t)sizeof(*info) && info->struct_version == UINT32_C(1) &&
        info->has_tag == CNA_FALSE && all_bytes_are(info->reserved, sizeof(info->reserved), 0U) &&
        info->name.byte_length == (uint64_t)sizeof(ResourceName) && info->name.data != NULL &&
        memcmp(info->name.data, ResourceName, sizeof(ResourceName)) == 0 &&
        state->disposing_calls == 1) {
        ++state->resource_destroyed_payloads;
    }
    if (cna_graphics_device_get_tracked_resource_count(device, &tracked) == CNA_RESULT_SUCCESS &&
        tracked == state->tracked_baseline + UINT64_C(1)) {
        ++state->resource_destroyed_saw_tracking;
    }
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    static const char EmbeddedNull[] = {'x', '\0', 'y'};
    ResourceLifecycleState* state = (ResourceLifecycleState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle owner = UINT64_C(91);
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle silent_texture = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo create_info;
    CNA_GraphicsResourceEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_GraphicsDeviceEventRegistrationHandle created_registration = CNA_INVALID_HANDLE;
    CNA_GraphicsDeviceEventRegistrationHandle destroyed_registration = CNA_INVALID_HANDLE;
    CNA_GraphicsResourceTag tag = UINT64_MAX;
    CNA_Bool is_disposed = CNA_TRUE;
    uint64_t bytes = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    char type_text[128];
    char short_name[sizeof(ResourceName)];
    char name_copy[sizeof(ResourceName) + 1U];
    char string_copy[sizeof(ResourceName) + 1U];
    CNA_Result borrow_result;
    CNA_Result create_result;
    CNA_Result copy_result = CNA_RESULT_INVALID_STATE;
    CNA_Result dispose_result;
    CNA_Result destroy_result;
    CNA_Result silent_create_result;
    CNA_Result silent_destroy_result = CNA_RESULT_INVALID_STATE;
    uint64_t tracked = UINT64_MAX;
    int ready;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    borrow_result = cna_game_get_graphics_device(game, &device);
    cna_demo_check(state->demo, borrow_result == CNA_RESULT_SUCCESS,
                   "borrow the live graphics device for this callback");
    state->expected_device = device;
    cna_demo_check(
        state->demo,
        borrow_result == CNA_RESULT_SUCCESS &&
            cna_graphics_device_get_tracked_resource_count(device, &state->tracked_baseline) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_subscribe_resource_created(
                device, resource_created, state, &created_registration) == CNA_RESULT_SUCCESS &&
            created_registration != CNA_INVALID_HANDLE &&
            cna_graphics_device_subscribe_resource_destroyed(
                device, resource_destroyed, state, &destroyed_registration) ==
                CNA_RESULT_SUCCESS &&
            destroyed_registration != CNA_INVALID_HANDLE,
        "record the tracked-resource baseline and subscribe both device resource events");
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
        goto cleanup;
    }
    state->expected_resource = texture;
    tracked = UINT64_MAX;
    cna_demo_check(
        state->demo,
        state->resource_created_calls == 1 && state->resource_created_payloads == 1 &&
            state->resource_created_saw_tracking == 1 &&
            cna_graphics_device_get_tracked_resource_count(device, &tracked) == CNA_RESULT_SUCCESS &&
            tracked == state->tracked_baseline + UINT64_C(1),
        "creation synchronously reports a complete payload after device tracking starts");

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
            texture, (CNA_StringView){ResourceName, (uint64_t)sizeof(ResourceName)}) ==
                CNA_RESULT_SUCCESS,
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
            bytes == sizeof(ResourceName) &&
            cna_graphics_resource_copy_name(
                texture, short_name, sizeof(ResourceName) - 1U, &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == sizeof(ResourceName) &&
            all_bytes_are(short_name, sizeof(short_name), 0x6B),
        "a short name buffer reports the UTF-8 byte count without a partial write");

    memset(name_copy, 0x7C, sizeof(name_copy));
    required = UINT64_C(0);
    cna_demo_check(
        state->demo,
        cna_graphics_resource_copy_name(
            texture, name_copy, sizeof(ResourceName), &required) == CNA_RESULT_SUCCESS &&
            required == sizeof(ResourceName) &&
            memcmp(name_copy, ResourceName, sizeof(ResourceName)) == 0 &&
            (unsigned char)name_copy[sizeof(ResourceName)] == 0x7C,
        "an exact name copy preserves all UTF-8 bytes and appends no terminator");

    memset(string_copy, 0x4D, sizeof(string_copy));
    bytes = UINT64_C(0);
    required = UINT64_C(0);
    cna_demo_check(
        state->demo,
        cna_graphics_resource_get_string_byte_count(texture, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(ResourceName) &&
            cna_graphics_resource_copy_string(
                texture, string_copy, sizeof(ResourceName), &required) == CNA_RESULT_SUCCESS &&
            required == sizeof(ResourceName) &&
            memcmp(string_copy, ResourceName, sizeof(ResourceName)) == 0 &&
            (unsigned char)string_copy[sizeof(ResourceName)] == 0x4D,
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
    tracked = UINT64_MAX;
    cna_demo_check(
        state->demo,
        state->resource_destroyed_calls == 1 && state->resource_destroyed_payloads == 1 &&
            state->resource_destroyed_saw_tracking == 1 &&
            cna_graphics_device_get_tracked_resource_count(device, &tracked) == CNA_RESULT_SUCCESS &&
            tracked == state->tracked_baseline,
        "device destruction reports the exact name and no native tag before untracking");
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
                CNA_RESULT_INVALID_HANDLE && state->resource_destroyed_calls == 1,
        "typed destruction releases the separate C handle after native disposal");
    if (destroy_result == CNA_RESULT_SUCCESS) {
        texture = CNA_INVALID_HANDLE;
        state->expected_resource = CNA_INVALID_HANDLE;
    }

    cna_demo_check(
        state->demo,
        cna_graphics_device_unsubscribe(created_registration) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_unsubscribe(created_registration) == CNA_RESULT_INVALID_HANDLE &&
            cna_graphics_device_unsubscribe(destroyed_registration) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_unsubscribe(destroyed_registration) == CNA_RESULT_INVALID_HANDLE,
        "both device registrations release once and refuse their stale handles");
    created_registration = CNA_INVALID_HANDLE;
    destroyed_registration = CNA_INVALID_HANDLE;

    silent_create_result = cna_texture2d_create(device, &create_info, &silent_texture);
    if (silent_create_result == CNA_RESULT_SUCCESS) {
        silent_destroy_result = cna_texture2d_destroy(silent_texture);
        if (silent_destroy_result == CNA_RESULT_SUCCESS) {
            silent_texture = CNA_INVALID_HANDLE;
        }
    }
    tracked = UINT64_MAX;
    cna_demo_check(
        state->demo,
        silent_create_result == CNA_RESULT_SUCCESS &&
            silent_destroy_result == CNA_RESULT_SUCCESS && state->resource_created_calls == 1 &&
            state->resource_destroyed_calls == 1 &&
            cna_graphics_device_get_tracked_resource_count(device, &tracked) == CNA_RESULT_SUCCESS &&
            tracked == state->tracked_baseline,
        "later resource lifetime stays silent after unsubscription and returns to the baseline");

cleanup:
    if (registration != CNA_INVALID_HANDLE) {
        (void)cna_graphics_resource_unsubscribe_disposing(registration);
    }
    if (created_registration != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_unsubscribe(created_registration);
    }
    if (destroyed_registration != CNA_INVALID_HANDLE) {
        (void)cna_graphics_device_unsubscribe(destroyed_registration);
    }
    if (silent_texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(silent_texture);
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
