/* SPDX-License-Identifier: MS-PL */
/*
 * Touch snapshots from C: CNA's public test hooks provide a deterministic pressed, moved and
 * released finger without requiring touch hardware. The program captures each frame as a
 * CNA_TouchState, finds the finger by id and reconstructs its previous location. A local value
 * snapshot then demonstrates pressure-independent identity, insertion-copy and collection edits.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static void blank_capabilities(CNA_TouchCapabilities* capabilities)
{
    memset(capabilities, 0, sizeof(*capabilities));
    capabilities->struct_size = (uint32_t)sizeof(*capabilities);
    capabilities->struct_version = UINT32_C(1);
}

static void blank_state(CNA_TouchState* state)
{
    memset(state, 0, sizeof(*state));
    state->struct_size = (uint32_t)sizeof(*state);
    state->struct_version = UINT32_C(1);
}

static int same_position(CNA_Vector2 value, float x, float y)
{
    return value.x == x && value.y == y;
}

int main(int argc, char** argv)
{
    static const int32_t finger_id = INT32_C(17);
    static const CNA_Vector2 first = {12.0f, 34.0f};
    static const CNA_Vector2 second = {56.0f, 78.0f};
    CnaDemo demo;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_GestureSample gesture;
    CNA_TouchCapabilities capabilities;
    CNA_TouchCapabilities value_capabilities;
    CNA_TouchState state;
    CNA_TouchState empty_state;
    CNA_TouchLocation location;
    CNA_TouchLocation plain;
    CNA_TouchLocation plain_previous;
    CNA_TouchLocation previous;
    CNA_TouchLocation low_pressure;
    CNA_TouchLocation high_pressure;
    CNA_TouchLocation second_touch;
    CNA_TouchLocation short_destination[3];
    CNA_TouchLocation short_before[3];
    CNA_TouchLocation destination[4];
    CNA_TouchLocation values_input[2];
    CNA_TouchState values;
    CNA_Bool found = CNA_FALSE;
    CNA_Bool equal = CNA_FALSE;
    CNA_Bool contains = CNA_FALSE;
    CNA_Bool read_only = CNA_FALSE;
    CNA_Bool removed = CNA_FALSE;
    CNA_Bool empty = CNA_FALSE;
    CNA_Bool device_exists = CNA_TRUE;
    CNA_Bool mouse_emulation = CNA_FALSE;
    CNA_Bool changed_emulation = CNA_FALSE;
    int32_t low_hash = 0;
    int32_t high_hash = 0;
    int32_t index = -1;
    uint64_t copied_count = 0U;
    uint64_t text_bytes = 0U;
    uint64_t original_window = UINT64_MAX;
    uint64_t window = UINT64_MAX;
    char text[64];
    char untouched_text[64];

    cna_demo_init(&demo, "c_api.input.touch_state", argc, argv, 0);
    memset(&gesture, 0xA5, sizeof(gesture));
    cna_demo_check(
        &demo,
        cna_gesture_sample_init(&gesture) == CNA_RESULT_SUCCESS &&
            gesture.struct_size == sizeof(gesture) && gesture.struct_version == UINT32_C(1) &&
            gesture.gesture_type == CNA_GESTURE_TYPE_NONE &&
            gesture.finger_id_ext == CNA_TOUCH_NO_FINGER &&
            gesture.finger_id2_ext == CNA_TOUCH_NO_FINGER && gesture.reserved == 0U &&
            gesture.timestamp_ticks == 0 && same_position(gesture.position, 0.0f, 0.0f) &&
            same_position(gesture.position2, 0.0f, 0.0f) &&
            same_position(gesture.delta, 0.0f, 0.0f) &&
            same_position(gesture.delta2, 0.0f, 0.0f),
        "the default gesture initializer writes the complete canonical empty value");
    cna_demo_check(
        &demo,
        cna_gesture_sample_init_from_values(
            CNA_GESTURE_TYPE_TAP | CNA_GESTURE_TYPE_FLICK, INT64_C(1234567),
            (CNA_Vector2){1.0f, 2.0f}, (CNA_Vector2){3.0f, 4.0f},
            (CNA_Vector2){5.0f, 6.0f}, (CNA_Vector2){7.0f, 8.0f}, &gesture) ==
                CNA_RESULT_SUCCESS &&
            gesture.gesture_type == (CNA_GESTURE_TYPE_TAP | CNA_GESTURE_TYPE_FLICK) &&
            gesture.finger_id_ext == CNA_TOUCH_NO_FINGER &&
            gesture.finger_id2_ext == CNA_TOUCH_NO_FINGER &&
            gesture.timestamp_ticks == INT64_C(1234567) &&
            same_position(gesture.position, 1.0f, 2.0f) &&
            same_position(gesture.position2, 3.0f, 4.0f) &&
            same_position(gesture.delta, 5.0f, 6.0f) &&
            same_position(gesture.delta2, 7.0f, 8.0f),
        "the value gesture initializer stores combined flags and leaves both fingers unset");
    memset(&value_capabilities, 0xA5, sizeof(value_capabilities));
    cna_demo_check(
        &demo,
        cna_touch_capabilities_init(&value_capabilities) == CNA_RESULT_SUCCESS &&
            value_capabilities.struct_size == sizeof(value_capabilities) &&
            value_capabilities.struct_version == UINT32_C(1) &&
            value_capabilities.is_connected == CNA_FALSE &&
            value_capabilities.maximum_touch_count == 0U &&
            cna_touch_capabilities_init_from_values_ext(
                CNA_TRUE, 4, &value_capabilities) == CNA_RESULT_SUCCESS &&
            value_capabilities.is_connected == CNA_TRUE &&
            value_capabilities.maximum_touch_count == 4U &&
            cna_touch_capabilities_init_from_values_ext(
                CNA_TRUE, -1, &value_capabilities) == CNA_RESULT_INVALID_ARGUMENT &&
            value_capabilities.is_connected == CNA_TRUE &&
            value_capabilities.maximum_touch_count == 4U,
        "capability initializers distinguish disconnected defaults and atomic authored values");
    memset(&plain, 0xA5, sizeof(plain));
    cna_demo_check(
        &demo,
        cna_touch_location_init(&plain) == CNA_RESULT_SUCCESS && plain.id == 0 &&
            plain.state == CNA_TOUCH_LOCATION_INVALID &&
            same_position(plain.position, 0.0f, 0.0f) &&
            plain.previous_state == CNA_TOUCH_LOCATION_INVALID &&
            same_position(plain.previous_position, 0.0f, 0.0f) && plain.pressure == 0.0f,
        "the default location uses id zero and invalid current and previous states");
    cna_demo_check(
        &demo,
        cna_touch_location_init_from_values(
            7, CNA_TOUCH_LOCATION_MOVED, (CNA_Vector2){3.0f, 4.0f}, &plain) ==
                CNA_RESULT_SUCCESS &&
            plain.id == 7 && plain.state == CNA_TOUCH_LOCATION_MOVED &&
            same_position(plain.position, 3.0f, 4.0f) &&
            plain.previous_state == CNA_TOUCH_LOCATION_INVALID && plain.pressure == 0.0f &&
            cna_touch_location_init_with_previous(
                7, CNA_TOUCH_LOCATION_MOVED, (CNA_Vector2){3.0f, 4.0f},
                CNA_TOUCH_LOCATION_PRESSED, (CNA_Vector2){1.0f, 2.0f}, &plain_previous) ==
                CNA_RESULT_SUCCESS &&
            plain_previous.previous_state == CNA_TOUCH_LOCATION_PRESSED &&
            same_position(plain_previous.previous_position, 1.0f, 2.0f) &&
            plain_previous.pressure == 0.0f,
        "plain location constructors distinguish absent and authored previous values");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_touch_location_get_string_size(&plain, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof("{Position:{X:3 Y:4}}") - 1U &&
            cna_touch_location_copy_string(&plain, text, text_bytes, &copied_count) ==
                CNA_RESULT_SUCCESS &&
            copied_count == text_bytes &&
            memcmp(text, "{Position:{X:3 Y:4}}", (size_t)text_bytes) == 0 &&
            (unsigned char)text[text_bytes] == 0xA5U,
        "location text reports only the exact position without appending a terminator");
    memset(text, 0xA5, sizeof(text));
    memset(untouched_text, 0xA5, sizeof(untouched_text));
    copied_count = 0U;
    cna_demo_check(
        &demo,
        cna_touch_location_copy_string(
            &plain, text, text_bytes - 1U, &copied_count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            copied_count == text_bytes && memcmp(text, untouched_text, sizeof(text)) == 0,
        "a short location-text destination is refused without a partial write");
    memset(&empty_state, 0xA5, sizeof(empty_state));
    cna_demo_check(
        &demo,
        cna_touch_state_init(&empty_state) == CNA_RESULT_SUCCESS &&
            empty_state.struct_size == sizeof(empty_state) &&
            empty_state.struct_version == UINT32_C(1) &&
            empty_state.is_connected == CNA_FALSE && empty_state.touch_count == 0U,
        "the empty touch-state initializer writes a disconnected versioned snapshot");

    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check(
        &demo,
        cna_touch_panel_get_mouse_touch_emulation_enabled_ext(game, &mouse_emulation) ==
                CNA_RESULT_SUCCESS &&
            cna_touch_panel_set_mouse_touch_emulation_enabled_ext(
                game, mouse_emulation == CNA_TRUE ? CNA_FALSE : CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_touch_panel_get_mouse_touch_emulation_enabled_ext(game, &changed_emulation) ==
                CNA_RESULT_SUCCESS &&
            changed_emulation != mouse_emulation &&
            cna_touch_panel_set_mouse_touch_emulation_enabled_ext(game, UINT8_C(2)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_touch_panel_get_mouse_touch_emulation_enabled_ext(game, &changed_emulation) ==
                CNA_RESULT_SUCCESS &&
            changed_emulation != mouse_emulation &&
            cna_touch_panel_set_mouse_touch_emulation_enabled_ext(game, mouse_emulation) ==
                CNA_RESULT_SUCCESS,
        "mouse-to-touch emulation toggles, rejects noncanonical booleans and restores its prior state");
    cna_demo_check(
        &demo,
        cna_touch_panel_get_window_handle(game, &original_window) == CNA_RESULT_SUCCESS &&
            cna_touch_panel_set_window_handle(game, UINT64_C(0x12345678)) == CNA_RESULT_SUCCESS &&
            cna_touch_panel_get_window_handle(game, &window) == CNA_RESULT_SUCCESS &&
            window == UINT64_C(0x12345678) &&
            cna_touch_panel_set_window_handle(game, original_window) == CNA_RESULT_SUCCESS &&
            cna_touch_panel_get_window_handle(game, &window) == CNA_RESULT_SUCCESS &&
            window == original_window,
        "the opaque touch window token round-trips exactly and restores the runtime binding");
    cna_demo_check(
        &demo,
        cna_touch_panel_get_mouse_touch_emulation_enabled_ext(game, 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_touch_panel_get_window_handle(game, 0) == CNA_RESULT_INVALID_ARGUMENT,
        "touch-panel process properties refuse null outputs");

    cna_demo_check_result(&demo, cna_touch_panel_reset_for_tests_ext(game),
                          "reset the process-wide touch fixture");
    if (cna_demo_check_result(
            &demo,
            cna_touch_panel_get_touch_device_exists_ext(game, &device_exists),
            "read the reset touch-device existence state")) {
        cna_demo_check(&demo, device_exists == CNA_FALSE,
                       "the reset fixture reports no live touch device");
    }
    cna_demo_check_result(&demo, cna_touch_panel_set_touch_device_exists_ext(game, CNA_TRUE),
                          "publish a synthetic touch device");
    device_exists = CNA_FALSE;
    if (cna_demo_check_result(
            &demo,
            cna_touch_panel_get_touch_device_exists_ext(game, &device_exists),
            "read the published touch-device existence state")) {
        cna_demo_check(&demo, device_exists == CNA_TRUE,
                       "the published fixture reports a live touch device");
    }
    blank_capabilities(&capabilities);
    if (cna_demo_check_result(&demo, cna_touch_get_capabilities(game, &capabilities),
                              "cna_touch_get_capabilities")) {
        cna_demo_check(&demo, capabilities.is_connected == CNA_TRUE && capabilities.maximum_touch_count == 4U,
                       "the published device reports CNA's four-touch XNA capability");
    }

    blank_state(&state);
    if (cna_demo_check_result(&demo, cna_touch_get_state(game, &state), "capture the initial touch state")) {
        cna_demo_check(&demo, state.is_connected == CNA_TRUE && state.touch_count == 0U,
                       "the connected touch device starts with an empty snapshot");
    }

    cna_demo_check_result(&demo, cna_touch_panel_set_finger_ext(game, 0, finger_id, first),
                          "press finger 17 in slot zero");
    cna_demo_check_result(&demo, cna_touch_panel_update_ext(game), "advance the pressed touch frame");
    blank_state(&state);
    if (cna_demo_check_result(&demo, cna_touch_get_state(game, &state), "capture the pressed touch")) {
        cna_demo_check(&demo,
                       state.touch_count == 1U && state.touches[0].id == finger_id &&
                           state.touches[0].state == CNA_TOUCH_LOCATION_PRESSED &&
                           same_position(state.touches[0].position, first.x, first.y),
                       "the snapshot contains the pressed finger and exact position");
    }
    memset(&location, 0x7F, sizeof(location));
    found = CNA_FALSE;
    cna_demo_check(&demo,
                   cna_touch_state_find_by_id(&state, finger_id, &location, &found) == CNA_RESULT_SUCCESS &&
                       found == CNA_TRUE && location.id == finger_id &&
                       location.state == CNA_TOUCH_LOCATION_PRESSED && same_position(location.position, first.x, first.y),
                   "find_by_id returns the pressed location");
    found = CNA_TRUE;
    memset(&location, 0x7F, sizeof(location));
    cna_demo_check(&demo,
                   cna_touch_state_find_by_id(&state, INT32_C(99), &location, &found) == CNA_RESULT_SUCCESS &&
                       found == CNA_FALSE && location.id == CNA_TOUCH_NO_FINGER &&
                       location.state == CNA_TOUCH_LOCATION_INVALID && same_position(location.position, 0.0f, 0.0f),
                   "a missing id returns false and the canonical invalid location");
    found = CNA_TRUE;
    memset(&previous, 0x7F, sizeof(previous));
    cna_demo_check(&demo,
                   cna_touch_location_try_get_previous(&state.touches[0], &previous, &found) == CNA_RESULT_SUCCESS &&
                       found == CNA_FALSE && previous.id == finger_id &&
                       previous.state == CNA_TOUCH_LOCATION_INVALID && same_position(previous.position, 0.0f, 0.0f),
                   "a new press has no previous location but still writes the invalid value");

    cna_demo_check_result(&demo, cna_touch_panel_set_finger_ext(game, 0, finger_id, second),
                          "move finger 17 in slot zero");
    cna_demo_check_result(&demo, cna_touch_panel_update_ext(game), "advance the moved touch frame");
    blank_state(&state);
    if (cna_demo_check_result(&demo, cna_touch_get_state(game, &state), "capture the moved touch")) {
        cna_demo_check(&demo,
                       state.touch_count == 1U && state.touches[0].state == CNA_TOUCH_LOCATION_MOVED &&
                           same_position(state.touches[0].position, second.x, second.y),
                       "the next snapshot contains the moved position");
        found = CNA_FALSE;
        memset(&previous, 0x7F, sizeof(previous));
        cna_demo_check(&demo,
                       cna_touch_location_try_get_previous(&state.touches[0], &previous, &found) ==
                               CNA_RESULT_SUCCESS &&
                           found == CNA_TRUE && previous.id == finger_id &&
                           previous.state == CNA_TOUCH_LOCATION_PRESSED &&
                           same_position(previous.position, first.x, first.y),
                       "the moved touch reconstructs its pressed previous location");
    }

    cna_demo_check_result(&demo,
                          cna_touch_panel_set_finger_ext(game, 0, CNA_TOUCH_NO_FINGER, second),
                          "release slot zero");
    cna_demo_check_result(&demo, cna_touch_panel_update_ext(game), "advance the released touch frame");
    blank_state(&state);
    if (cna_demo_check_result(&demo, cna_touch_get_state(game, &state), "capture the released touch")) {
        cna_demo_check(&demo,
                       state.touch_count == 1U && state.touches[0].state == CNA_TOUCH_LOCATION_RELEASED &&
                           same_position(state.touches[0].position, second.x, second.y),
                       "release is reported once at the last position");
        found = CNA_FALSE;
        memset(&previous, 0x7F, sizeof(previous));
        cna_demo_check(&demo,
                       cna_touch_location_try_get_previous(&state.touches[0], &previous, &found) ==
                               CNA_RESULT_SUCCESS &&
                           found == CNA_TRUE && previous.state == CNA_TOUCH_LOCATION_MOVED &&
                           same_position(previous.position, second.x, second.y),
                       "the released touch keeps its moved previous location");
    }

    cna_demo_check_result(&demo,
                          cna_touch_panel_set_finger_ext(game, 0, CNA_TOUCH_NO_FINGER, second),
                          "leave the released slot empty on the next frame");
    cna_demo_check_result(&demo, cna_touch_panel_update_ext(game), "advance beyond the released touch frame");
    blank_state(&state);
    if (cna_demo_check_result(&demo, cna_touch_get_state(game, &state), "capture after the release")) {
        cna_demo_check(&demo, state.touch_count == 0U,
                       "the released touch retires after its one reported frame");
    }

    cna_demo_check_result(&demo, cna_touch_panel_reset_for_tests_ext(game), "restore the touch fixture");

    cna_demo_check_result(
        &demo,
        cna_touch_location_init_with_previous_ext(
            INT32_C(7), CNA_TOUCH_LOCATION_MOVED, (CNA_Vector2){130.0f, 90.0f},
            CNA_TOUCH_LOCATION_PRESSED, (CNA_Vector2){100.0f, 50.0f}, 0.25f, &low_pressure),
        "construct a moved touch with previous location and low pressure");
    cna_demo_check_result(
        &demo,
        cna_touch_location_init_with_previous_ext(
            INT32_C(7), CNA_TOUCH_LOCATION_MOVED, (CNA_Vector2){130.0f, 90.0f},
            CNA_TOUCH_LOCATION_PRESSED, (CNA_Vector2){100.0f, 50.0f}, 0.75f, &high_pressure),
        "construct the same touch with higher pressure");
    equal = CNA_FALSE;
    if (cna_demo_check_result(&demo, cna_touch_location_equals(&low_pressure, &high_pressure, &equal),
                              "compare touch locations that differ only in pressure")) {
        cna_demo_check(&demo, equal == CNA_TRUE,
                       "canonical touch equality deliberately ignores the pressure extension");
    }
    if (cna_demo_check_result(&demo, cna_touch_location_get_hash_code(&low_pressure, &low_hash),
                              "hash the low-pressure touch") &&
        cna_demo_check_result(&demo, cna_touch_location_get_hash_code(&high_pressure, &high_hash),
                              "hash the high-pressure touch")) {
        cna_demo_check(&demo, low_hash == high_hash,
                       "pressure-equal touch locations have the same canonical hash");
    }
    cna_demo_check_result(
        &demo,
        cna_touch_location_init_from_values_ext(
            INT32_C(9), CNA_TOUCH_LOCATION_PRESSED, (CNA_Vector2){600.0f, 300.0f}, 1.0f, &second_touch),
        "construct a second pressured touch");

    values_input[0] = low_pressure;
    values_input[1] = second_touch;
    if (cna_demo_check_result(&demo, cna_touch_state_init_from_locations(values_input, 2U, &values),
                              "construct a local two-touch snapshot")) {
        cna_demo_check(&demo,
                       values.is_connected == CNA_FALSE && values.touch_count == 2U &&
                           values.touches[0].id == INT32_C(7) && values.touches[1].id == INT32_C(9),
                       "the local snapshot copies both touches and has no live connection");
    }
    if (cna_demo_check_result(&demo, cna_touch_state_get_is_read_only(&values, &read_only),
                              "query the collection read-only advisory")) {
        cna_demo_check(&demo, read_only == CNA_TRUE,
                       "the canonical collection describes itself as read-only");
    }
    if (cna_demo_check_result(&demo, cna_touch_state_contains(&values, &high_pressure, &contains),
                              "search the collection with different pressure")) {
        cna_demo_check(&demo, contains == CNA_TRUE,
                       "contains uses the same pressure-independent touch equality");
    }
    if (cna_demo_check_result(&demo, cna_touch_state_index_of(&values, &high_pressure, &index),
                              "find the pressure-equal touch index")) {
        cna_demo_check(&demo, index == 0,
                       "index_of finds the first pressure-equal touch at index zero");
    }

    short_destination[0] = second_touch;
    short_destination[1] = high_pressure;
    short_destination[2] = second_touch;
    memcpy(short_before, short_destination, sizeof(short_before));
    copied_count = UINT64_C(999);
    cna_demo_check(&demo,
                   cna_touch_state_copy_to(&values, short_destination, 2U, 3U, 1, &copied_count) ==
                           CNA_RESULT_BUFFER_TOO_SMALL &&
                       copied_count == 4U && memcmp(short_destination, short_before, sizeof(short_before)) == 0,
                   "a short insertion copy reports four required entries without partial writes");

    destination[0] = second_touch;
    destination[1] = high_pressure;
    destination[2] = high_pressure;
    destination[3] = high_pressure;
    copied_count = 0U;
    if (cna_demo_check_result(&demo, cna_touch_state_copy_to(&values, destination, 2U, 4U, 1, &copied_count),
                              "insert the touch snapshot into an exact destination")) {
        cna_demo_check(&demo,
                       copied_count == 4U && destination[0].id == INT32_C(9) &&
                           destination[1].id == INT32_C(7) && destination[1].pressure == 0.25f &&
                           destination[2].id == INT32_C(9) && destination[3].id == INT32_C(7) &&
                           destination[3].pressure == 0.75f,
                       "copy_to inserts both touches and shifts the existing tail");
    }

    removed = CNA_FALSE;
    if (cna_demo_check_result(&demo, cna_touch_state_remove(&values, &high_pressure, &removed),
                              "remove through pressure-independent equality")) {
        cna_demo_check(&demo,
                       removed == CNA_TRUE && values.touch_count == 1U &&
                           values.touches[0].id == INT32_C(9),
                       "remove deletes the pressure-equal first touch and shifts the second");
    }
    if (cna_demo_check_result(&demo, cna_touch_state_insert(&values, 0, &low_pressure),
                              "insert the first touch back at index zero")) {
        cna_demo_check(&demo,
                       values.touch_count == 2U && values.touches[0].id == INT32_C(7) &&
                           values.touches[1].id == INT32_C(9),
                       "insert restores the selected collection order");
    }
    if (cna_demo_check_result(&demo, cna_touch_state_remove_at(&values, 1),
                              "remove the second touch by index")) {
        cna_demo_check(&demo, values.touch_count == 1U && values.touches[0].id == INT32_C(7),
                       "remove_at leaves only the first touch");
    }
    if (cna_demo_check_result(&demo, cna_touch_state_add(&values, &second_touch),
                              "append the second touch")) {
        cna_demo_check(&demo,
                       values.touch_count == 2U && values.touches[1].id == INT32_C(9),
                       "add appends even though IsReadOnly reports true");
    }
    cna_demo_check_result(&demo, cna_touch_state_clear(&values), "clear the local touch collection");
    if (cna_demo_check_result(&demo, cna_touch_state_get_is_empty_ext(&values, &empty),
                              "query the cleared collection")) {
        cna_demo_check(&demo, empty == CNA_TRUE && values.touch_count == 0U,
                       "clear leaves an empty collection");
    }

    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
