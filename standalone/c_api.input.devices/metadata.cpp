// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.devices",
    .title = "Input in C: host devices",
    .description = "A C program compares sensor and device descriptor values, enumerates host mice, keyboards, "
                   "touch devices and sensors, checks unavailable motion readings, exercises all four synchronous "
                   "mouse/keyboard hot-plug events and reads one coherent power snapshot.",
    .apis = {"cna_sensor_info_init", "cna_sensor_info_equals", "cna_sensors_get_count",
             "cna_sensors_get_accelerometer", "cna_sensors_get_gyroscope", "cna_input_device_info_init",
             "cna_input_device_info_equals", "cna_input_devices_get_mouse_count",
             "cna_input_devices_get_mouse_info_at", "cna_input_devices_get_mouse_name_size_at",
             "cna_input_devices_copy_mouse_name_at", "cna_input_devices_get_keyboard_count",
             "cna_input_devices_get_keyboard_info_at", "cna_input_devices_get_keyboard_name_size_at",
             "cna_input_devices_copy_keyboard_name_at", "cna_input_devices_get_touch_device_count",
             "cna_input_devices_subscribe_mouse_connected_ext",
             "cna_input_devices_subscribe_mouse_disconnected_ext",
             "cna_input_devices_subscribe_keyboard_connected_ext",
             "cna_input_devices_subscribe_keyboard_disconnected_ext", "cna_input_devices_unsubscribe_ext",
             "cna_input_devices_raise_mouse_connected_ext", "cna_input_devices_raise_mouse_disconnected_ext",
             "cna_input_devices_raise_keyboard_connected_ext",
             "cna_input_devices_raise_keyboard_disconnected_ext", "cna_input_devices_reset_for_tests_ext",
             "cna_power_get_info"},
    .tags = {"c", "binding", "input", "devices", "sensors", "hot-plug", "power", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Enumeration and power values are host-dependent. Verification covers "
                    "empty host sensors/touch devices; their populated metadata branches require suitable hardware.",
})
