/* SPDX-License-Identifier: MS-PL */
/*
 * Touch snapshots from C: CNA's public test hooks provide a deterministic pressed, moved and
 * released finger without requiring touch hardware. The program captures each frame as a
 * CNA_TouchState, finds the finger by id and reconstructs its previous location.
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
    CNA_TouchCapabilities capabilities;
    CNA_TouchState state;
    CNA_TouchLocation location;
    CNA_TouchLocation previous;
    CNA_Bool found = CNA_FALSE;

    cna_demo_init(&demo, "c_api.input.touch_state", argc, argv, 0);
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check_result(&demo, cna_touch_panel_reset_for_tests_ext(game),
                          "reset the process-wide touch fixture");
    cna_demo_check_result(&demo, cna_touch_panel_set_touch_device_exists_ext(game, CNA_TRUE),
                          "publish a synthetic touch device");
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
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
