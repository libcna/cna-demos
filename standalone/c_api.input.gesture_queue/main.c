/* SPDX-License-Identifier: MS-PL */
/*
 * Gesture queues from C: construct two complete gesture values, enable a detection mask, enqueue
 * both values, change the mask without dropping pending samples, and read them back in FIFO order.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static void blank_sample(CNA_GestureSample* sample)
{
    memset(sample, 0, sizeof(*sample));
    sample->struct_size = (uint32_t)sizeof(*sample);
    sample->struct_version = UINT32_C(1);
}

static int same_vector(CNA_Vector2 value, float x, float y)
{
    return value.x == x && value.y == y;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_GestureType enabled = CNA_GESTURE_TYPE_ALL;
    CNA_Bool available = CNA_TRUE;
    CNA_GestureSample tap;
    CNA_GestureSample drag;
    CNA_GestureSample read_back;
    CNA_GestureSample before_refusal;

    cna_demo_init(&demo, "c_api.input.gesture_queue", argc, argv, 0);
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check_result(&demo, cna_touch_panel_reset_for_tests_ext(game),
                          "reset the process-wide gesture fixture");
    if (cna_demo_check_result(&demo, cna_touch_panel_get_enabled_gestures(game, &enabled),
                              "read the default enabled-gesture mask")) {
        cna_demo_check(&demo, enabled == CNA_GESTURE_TYPE_NONE,
                       "gesture detection starts disabled after reset");
    }
    cna_demo_check_result(
        &demo,
        cna_touch_panel_set_enabled_gestures(
            game, CNA_GESTURE_TYPE_TAP | CNA_GESTURE_TYPE_FREE_DRAG),
        "enable tap and free-drag detection");
    if (cna_demo_check_result(&demo, cna_touch_panel_get_enabled_gestures(game, &enabled),
                              "read the enabled-gesture mask")) {
        cna_demo_check(&demo,
                       enabled == (CNA_GESTURE_TYPE_TAP | CNA_GESTURE_TYPE_FREE_DRAG),
                       "the enabled mask preserves both requested bits");
    }
    if (cna_demo_check_result(&demo, cna_touch_panel_get_is_gesture_available(game, &available),
                              "query the initially empty queue")) {
        cna_demo_check(&demo, available == CNA_FALSE,
                       "no gesture is available before enqueue");
    }

    cna_demo_check_result(
        &demo,
        cna_gesture_sample_init_from_values_ext(
            CNA_GESTURE_TYPE_TAP, INT64_C(10000000),
            (CNA_Vector2){10.0f, 20.0f}, (CNA_Vector2){0.0f, 0.0f},
            (CNA_Vector2){1.0f, 2.0f}, (CNA_Vector2){0.0f, 0.0f},
            INT32_C(7), CNA_TOUCH_NO_FINGER, &tap),
        "construct an explicit tap sample");
    cna_demo_check(&demo,
                   tap.gesture_type == CNA_GESTURE_TYPE_TAP &&
                       tap.timestamp_ticks == INT64_C(10000000) &&
                       tap.finger_id_ext == INT32_C(7) && tap.finger_id2_ext == CNA_TOUCH_NO_FINGER &&
                       same_vector(tap.position, 10.0f, 20.0f) && same_vector(tap.delta, 1.0f, 2.0f),
                   "the tap sample stores its timestamp, finger and primary vectors");
    cna_demo_check_result(
        &demo,
        cna_gesture_sample_init_from_values_ext(
            CNA_GESTURE_TYPE_FREE_DRAG, INT64_C(15000000),
            (CNA_Vector2){30.0f, 40.0f}, (CNA_Vector2){50.0f, 60.0f},
            (CNA_Vector2){3.0f, 4.0f}, (CNA_Vector2){5.0f, 6.0f},
            INT32_C(8), INT32_C(9), &drag),
        "construct an explicit free-drag sample");
    cna_demo_check(&demo,
                   drag.gesture_type == CNA_GESTURE_TYPE_FREE_DRAG &&
                       drag.timestamp_ticks == INT64_C(15000000) &&
                       drag.finger_id_ext == INT32_C(8) && drag.finger_id2_ext == INT32_C(9) &&
                       same_vector(drag.position, 30.0f, 40.0f) &&
                       same_vector(drag.position2, 50.0f, 60.0f) &&
                       same_vector(drag.delta, 3.0f, 4.0f) && same_vector(drag.delta2, 5.0f, 6.0f),
                   "the drag sample stores both fingers and all four vectors");

    cna_demo_check_result(&demo, cna_touch_panel_enqueue_gesture_ext(game, &tap),
                          "enqueue the tap first");
    cna_demo_check_result(&demo, cna_touch_panel_enqueue_gesture_ext(game, &drag),
                          "enqueue the drag second");
    if (cna_demo_check_result(&demo, cna_touch_panel_get_is_gesture_available(game, &available),
                              "query the populated queue")) {
        cna_demo_check(&demo, available == CNA_TRUE,
                       "a gesture is available after enqueue");
    }
    cna_demo_check_result(&demo,
                          cna_touch_panel_set_enabled_gestures(game, CNA_GESTURE_TYPE_NONE),
                          "disable future gesture detection while samples are pending");
    if (cna_demo_check_result(&demo, cna_touch_panel_get_is_gesture_available(game, &available),
                              "query pending samples after disabling detection")) {
        cna_demo_check(&demo, available == CNA_TRUE,
                       "changing the enabled mask does not clear the queue");
    }

    blank_sample(&read_back);
    if (cna_demo_check_result(&demo, cna_touch_panel_read_gesture(game, &read_back),
                              "read the first queued gesture")) {
        cna_demo_check(&demo, memcmp(&read_back, &tap, sizeof(tap)) == 0,
                       "FIFO order returns the exact tap sample first");
    }
    if (cna_demo_check_result(&demo, cna_touch_panel_get_is_gesture_available(game, &available),
                              "query the queue after one read")) {
        cna_demo_check(&demo, available == CNA_TRUE,
                       "the second queued gesture remains available");
    }
    blank_sample(&read_back);
    if (cna_demo_check_result(&demo, cna_touch_panel_read_gesture(game, &read_back),
                              "read the second queued gesture")) {
        cna_demo_check(&demo, memcmp(&read_back, &drag, sizeof(drag)) == 0,
                       "FIFO order returns the exact drag sample second");
    }
    if (cna_demo_check_result(&demo, cna_touch_panel_get_is_gesture_available(game, &available),
                              "query the drained queue")) {
        cna_demo_check(&demo, available == CNA_FALSE,
                       "the queue is unavailable after the last read");
    }

    read_back = drag;
    before_refusal = read_back;
    cna_demo_check(&demo,
                   cna_touch_panel_read_gesture(game, &read_back) == CNA_RESULT_INVALID_STATE &&
                       memcmp(&read_back, &before_refusal, sizeof(read_back)) == 0,
                   "an empty read is refused without changing the output sample");

    cna_demo_check_result(&demo, cna_touch_panel_reset_for_tests_ext(game),
                          "restore the process-wide gesture fixture");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
