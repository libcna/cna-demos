// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.devices.sensor_lifecycle",
    .title = "Device sensors in C: acquisition and events",
    .description = "A C game observes host availability, then uses CNA's public synthetic accelerometer, gyroscope, "
                   "compass and fused-motion support to exercise acquisition state, current readings, update "
                   "intervals, event order, calibration, unsubscribe, stop, disposal and handle ownership.",
    .apis = {"cna_sensors_get_last_error_id_ext", "cna_sensor_unsubscribe_ext",
             "cna_accelerometer_get_is_supported", "cna_accelerometer_create",
             "cna_accelerometer_get_state", "cna_accelerometer_start", "cna_accelerometer_stop",
             "cna_accelerometer_dispose", "cna_accelerometer_get_current_value",
             "cna_accelerometer_get_is_data_valid", "cna_accelerometer_get_time_between_updates_ticks",
             "cna_accelerometer_set_time_between_updates_ticks",
             "cna_accelerometer_subscribe_current_value_changed",
             "cna_accelerometer_inject_synthetic_update_ext",
             "cna_accelerometer_set_started_for_tests_ext",
             "cna_accelerometer_set_supported_for_tests_ext", "cna_accelerometer_destroy",
             "cna_accelerometer_subscribe_reading_changed",
             "cna_gyroscope_get_is_supported", "cna_gyroscope_create", "cna_gyroscope_get_state",
             "cna_gyroscope_start", "cna_gyroscope_stop", "cna_gyroscope_dispose",
             "cna_gyroscope_get_current_value", "cna_gyroscope_get_is_data_valid",
             "cna_gyroscope_get_time_between_updates_ticks",
             "cna_gyroscope_set_time_between_updates_ticks",
             "cna_gyroscope_subscribe_current_value_changed",
             "cna_gyroscope_inject_synthetic_update_ext", "cna_gyroscope_set_started_for_tests_ext",
             "cna_gyroscope_set_supported_for_tests_ext", "cna_gyroscope_destroy",
             "cna_compass_get_is_supported", "cna_compass_create", "cna_compass_get_state",
             "cna_compass_start", "cna_compass_stop", "cna_compass_dispose",
             "cna_compass_get_current_value", "cna_compass_get_is_data_valid",
             "cna_compass_get_time_between_updates_ticks", "cna_compass_set_time_between_updates_ticks",
             "cna_compass_subscribe_current_value_changed", "cna_compass_subscribe_calibrate",
             "cna_compass_set_test_backend_ext", "cna_compass_inject_synthetic_update_ext",
             "cna_compass_inject_calibration_request_ext", "cna_compass_destroy",
             "cna_motion_get_is_supported", "cna_motion_create", "cna_motion_get_state",
             "cna_motion_get_is_attitude_north_referenced_ext", "cna_motion_start", "cna_motion_stop",
             "cna_motion_dispose", "cna_motion_get_current_value", "cna_motion_get_is_data_valid",
             "cna_motion_get_time_between_updates_ticks", "cna_motion_set_time_between_updates_ticks",
             "cna_motion_subscribe_current_value_changed", "cna_motion_subscribe_calibrate",
             "cna_motion_set_test_backend_ext", "cna_motion_inject_synthetic_update_ext",
             "cna_motion_inject_calibration_request_ext", "cna_motion_destroy"},
    .tags = {"c", "binding", "devices", "sensors", "events", "lifecycle", "accelerometer", "gyroscope", "compass", "motion", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Host availability is reported, while deterministic supported paths use "
                    "CNA's documented synthetic/test backends; no physical measurement is claimed. ISSUE-062: "
                    "accelerometer and gyroscope CurrentValue remain readable after disposal.",
    .knownIssue = "CNA-DEMOS-ISSUE-062",
})
