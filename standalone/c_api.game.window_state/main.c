/* SPDX-License-Identifier: MS-PL */
/* GameWindow state, event and borrowed native identity through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView view(const char* text, uint64_t bytes)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = bytes;
    return result;
}

static void count_event(void* context)
{
    ++*(int*)context;
}

static int accepted_or_platform_refused(CNA_Result result)
{
    return result == CNA_RESULT_SUCCESS || result == CNA_RESULT_PLATFORM;
}

static int native_layout_is_valid(const CNA_NativeWindowHandle* native)
{
    switch (native->system) {
    case CNA_NATIVE_WINDOW_SYSTEM_X11:
        return native->display != 0 && native->window == 0 && native->surface == 0 &&
               native->window_id != 0U;
    case CNA_NATIVE_WINDOW_SYSTEM_WAYLAND:
        return native->display != 0 && native->surface != 0 && native->window_id == 0U;
    case CNA_NATIVE_WINDOW_SYSTEM_UNKNOWN:
    case CNA_NATIVE_WINDOW_SYSTEM_HEADLESS:
    case CNA_NATIVE_WINDOW_SYSTEM_TERMINAL:
        return native->display == 0 && native->window == 0 && native->surface == 0 &&
               native->window_id == 0U;
    case CNA_NATIVE_WINDOW_SYSTEM_WIN32:
    case CNA_NATIVE_WINDOW_SYSTEM_COCOA:
    case CNA_NATIVE_WINDOW_SYSTEM_ANDROID:
    case CNA_NATIVE_WINDOW_SYSTEM_UIKIT:
        return native->window != 0 && native->window_id == 0U;
    case CNA_NATIVE_WINDOW_SYSTEM_WEB:
        return native->window_id == 0U;
    default:
        return 0;
    }
}

int main(int argc, char** argv)
{
    static const char ExpectedType[] = "Microsoft.Xna.Framework.GameWindow";
    CnaDemo demo;
    CNA_GameCreateInfo info = {0};
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_GameEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_GameEventRegistrationHandle rejected = UINT64_MAX;
    CNA_Rectangle original_bounds = {0, 0, 0, 0};
    CNA_Rectangle changed_bounds = {0, 0, 0, 0};
    CNA_DisplayOrientation orientation = UINT32_MAX;
    CNA_NativeWindowHandle native;
    CNA_NativeWindowHandle uninitialized;
    CNA_Bool resize_allowed = UINT8_C(77);
    CNA_Bool borderless = UINT8_C(77);
    CNA_Bool observed = UINT8_C(77);
    CNA_Result change_result;
    uint64_t legacy_handle = UINT64_MAX;
    uint64_t screen_bytes = UINT64_MAX;
    uint64_t copied_bytes = UINT64_MAX;
    uint64_t type_bytes = UINT64_MAX;
    char screen_name[256];
    char short_name[256];
    char before_name[256];
    char type_name[64];
    char before_type[64];
    int size_events = 0;
    int size_changed = 0;

    cna_demo_init(&demo, "c_api.game.window_state", argc, argv, 1);
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.window_title = view("C API window state", UINT64_C(18));

    if (!cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the window-state game")) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check(
        &demo,
        cna_game_window_get_allow_user_resizing(game, &resize_allowed) == CNA_RESULT_SUCCESS &&
            (resize_allowed == CNA_FALSE || resize_allowed == CNA_TRUE),
        "the window exposes its current user-resizing policy");
    change_result = cna_game_window_set_allow_user_resizing(
        game, resize_allowed == CNA_TRUE ? CNA_FALSE : CNA_TRUE);
    cna_demo_check(
        &demo,
        accepted_or_platform_refused(change_result) &&
            cna_game_window_get_allow_user_resizing(game, &observed) == CNA_RESULT_SUCCESS &&
            (change_result == CNA_RESULT_PLATFORM || observed != resize_allowed) &&
            accepted_or_platform_refused(
                cna_game_window_set_allow_user_resizing(game, resize_allowed)),
        "a supported resizing-policy change round-trips and the original policy is restored");

    cna_demo_check(
        &demo,
        cna_game_window_get_client_bounds(game, &original_bounds) == CNA_RESULT_SUCCESS &&
            original_bounds.width >= 0 && original_bounds.height >= 0 &&
            cna_game_window_get_current_orientation(game, &orientation) == CNA_RESULT_SUCCESS &&
            (orientation == CNA_DISPLAY_ORIENTATION_DEFAULT ||
             orientation == CNA_DISPLAY_ORIENTATION_LANDSCAPE_LEFT ||
             orientation == CNA_DISPLAY_ORIENTATION_LANDSCAPE_RIGHT ||
             orientation == CNA_DISPLAY_ORIENTATION_PORTRAIT),
        "client bounds and current orientation form a valid window snapshot");

    memset(screen_name, 0xA5, sizeof(screen_name));
    cna_demo_check(
        &demo,
        cna_game_window_get_screen_device_name_size(game, &screen_bytes) == CNA_RESULT_SUCCESS &&
            screen_bytes < sizeof(screen_name) &&
            cna_game_window_copy_screen_device_name(
                game, screen_name, screen_bytes, &copied_bytes) == CNA_RESULT_SUCCESS &&
            copied_bytes == screen_bytes &&
            (unsigned char)screen_name[screen_bytes] == 0xA5U,
        "the screen-device name copies as exact unterminated UTF-8 bytes");
    memset(short_name, 0xA5, sizeof(short_name));
    memcpy(before_name, short_name, sizeof(short_name));
    cna_demo_check(
        &demo,
        screen_bytes == 0U ||
            (cna_game_window_copy_screen_device_name(
                 game, short_name, screen_bytes - 1U, &copied_bytes) ==
                 CNA_RESULT_BUFFER_TOO_SMALL && copied_bytes == screen_bytes &&
             memcmp(short_name, before_name, sizeof(short_name)) == 0),
        "a nonempty short screen-name destination is refused without a partial write");

    memset(type_name, 0xA5, sizeof(type_name));
    cna_demo_check(
        &demo,
        cna_game_window_get_type_name_size(game, &type_bytes) == CNA_RESULT_SUCCESS &&
            type_bytes == sizeof(ExpectedType) - 1U &&
            cna_game_window_copy_type_name(game, type_name, type_bytes, &copied_bytes) ==
                CNA_RESULT_SUCCESS && copied_bytes == type_bytes &&
            memcmp(type_name, ExpectedType, sizeof(ExpectedType) - 1U) == 0 &&
            (unsigned char)type_name[type_bytes] == 0xA5U,
        "the window reports the exact canonical GameWindow reflection name");
    memset(type_name, 0xA5, sizeof(type_name));
    memcpy(before_type, type_name, sizeof(type_name));
    cna_demo_check(
        &demo,
        cna_game_window_copy_type_name(game, type_name, type_bytes - 1U, &copied_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL && copied_bytes == type_bytes &&
            memcmp(type_name, before_type, sizeof(type_name)) == 0,
        "a short type-name destination is refused without a partial write");

    cna_demo_check(
        &demo,
        cna_native_window_handle_init(&native) == CNA_RESULT_SUCCESS &&
            native.struct_size == sizeof(native) && native.struct_version == 1U &&
            native.system == CNA_NATIVE_WINDOW_SYSTEM_UNKNOWN && native.display == 0 &&
            native.window == 0 && native.surface == 0 && native.window_id == 0U,
        "native-window initialization produces a versioned empty descriptor");
    memset(&uninitialized, 0, sizeof(uninitialized));
    cna_demo_check(
        &demo,
        cna_game_window_get_native_window_ext(game, &uninitialized) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_game_window_get_native_window_ext(game, &native) == CNA_RESULT_SUCCESS &&
            native.system <= CNA_NATIVE_WINDOW_SYSTEM_MAXIMUM && native_layout_is_valid(&native),
        "the native-window query requires initialization and fills only system-specific fields");
    cna_demo_check(
        &demo,
        cna_game_window_get_native_handle_ext(game, &legacy_handle) == CNA_RESULT_SUCCESS &&
            ((native.system == CNA_NATIVE_WINDOW_SYSTEM_X11 ||
              native.system == CNA_NATIVE_WINDOW_SYSTEM_WAYLAND ||
              native.system == CNA_NATIVE_WINDOW_SYSTEM_WIN32 ||
              native.system == CNA_NATIVE_WINDOW_SYSTEM_COCOA ||
              native.system == CNA_NATIVE_WINDOW_SYSTEM_ANDROID ||
              native.system == CNA_NATIVE_WINDOW_SYSTEM_UIKIT)
                 ? legacy_handle != 0U
                 : 1),
        "the legacy round-trip token is present whenever a native desktop/mobile window exists");

    cna_demo_check(
        &demo,
        cna_game_window_get_is_borderless_ext(game, &borderless) == CNA_RESULT_SUCCESS &&
            (borderless == CNA_FALSE || borderless == CNA_TRUE),
        "the window exposes its current border policy");
    change_result = cna_game_window_set_is_borderless_ext(
        game, borderless == CNA_TRUE ? CNA_FALSE : CNA_TRUE);
    cna_demo_check(
        &demo,
        accepted_or_platform_refused(change_result) &&
            cna_game_window_get_is_borderless_ext(game, &observed) == CNA_RESULT_SUCCESS &&
            (change_result == CNA_RESULT_PLATFORM || observed != borderless) &&
            accepted_or_platform_refused(
                cna_game_window_set_is_borderless_ext(game, borderless)),
        "a supported border-policy change round-trips and the original policy is restored");
    cna_demo_check(
        &demo,
        accepted_or_platform_refused(cna_game_window_minimize_ext(game)) &&
            accepted_or_platform_refused(cna_game_window_restore_ext(game)),
        "minimize and restore requests either succeed or report a platform refusal");

    cna_demo_check(
        &demo,
        CNA_GAME_WINDOW_EVENT_CLIENT_SIZE_CHANGED == 0U &&
            CNA_GAME_WINDOW_EVENT_MAXIMUM == CNA_GAME_WINDOW_EVENT_SCREEN_DEVICE_NAME_CHANGED &&
            cna_game_window_subscribe(
                game, CNA_GAME_WINDOW_EVENT_CLIENT_SIZE_CHANGED, count_event, &size_events,
                &registration) == CNA_RESULT_SUCCESS && registration != CNA_INVALID_HANDLE &&
            cna_game_window_subscribe(
                game, CNA_GAME_WINDOW_EVENT_MAXIMUM + 1U, count_event, &size_events,
                &rejected) == CNA_RESULT_INVALID_ARGUMENT && rejected == CNA_INVALID_HANDLE,
        "a client-size handler owns one registration and undefined events are refused");

    cna_demo_check(
        &demo,
        cna_game_window_begin_screen_device_change(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            accepted_or_platform_refused(cna_game_window_end_screen_device_change(
                game, view(screen_name, screen_bytes),
                original_bounds.width > 0 ? original_bounds.width + 8 : 0,
                original_bounds.height > 0 ? original_bounds.height + 8 : 0)) &&
            cna_game_window_get_client_bounds(game, &changed_bounds) == CNA_RESULT_SUCCESS,
        "a windowed screen-device transaction applies a requested positive client size");
    size_changed = changed_bounds.width != original_bounds.width ||
                   changed_bounds.height != original_bounds.height;
    cna_demo_check(
        &demo,
        (size_changed && size_events == 1 && changed_bounds.width == original_bounds.width + 8 &&
         changed_bounds.height == original_bounds.height + 8) ||
            (!size_changed && size_events == 0),
        "a realized client-size change raises one event while a windowless no-op raises none");

    cna_demo_check(
        &demo,
        cna_game_window_begin_screen_device_change(game, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            accepted_or_platform_refused(cna_game_window_end_screen_device_change(
                game, view(screen_name, screen_bytes), original_bounds.width,
                original_bounds.height)) &&
            cna_game_window_get_client_bounds(game, &changed_bounds) == CNA_RESULT_SUCCESS &&
            (!size_changed ||
             (changed_bounds.width == original_bounds.width &&
              changed_bounds.height == original_bounds.height && size_events == 2)) &&
            cna_game_unsubscribe(registration) == CNA_RESULT_SUCCESS &&
            cna_game_unsubscribe(registration) == CNA_RESULT_INVALID_HANDLE,
        "the original client size is restored and the event registration releases once");

    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the window-state game");
    return cna_demo_finish(&demo);
}
