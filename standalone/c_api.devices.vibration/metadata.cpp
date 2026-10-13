// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.devices.vibration",
    .title = "Device services in C: vibration requests",
    .description = "A C game reports the real vibration capability without actuating it, then uses CNA's public "
                   "test backend to verify device-name buffers, duration bounds, intensity and dual-motor "
                   "clamping, stop requests, backend replacement and restoration.",
    .apis = {"cna_vibrate_controller_start", "cna_vibrate_controller_start_with_intensity_ext",
             "cna_vibrate_controller_start_left_right_ext", "cna_vibrate_controller_stop",
             "cna_vibrate_controller_get_is_supported_ext",
             "cna_vibrate_controller_get_device_name_size_ext",
             "cna_vibrate_controller_copy_device_name_ext",
             "cna_vibrate_controller_set_test_backend_ext",
             "cna_vibrate_controller_get_test_log_ext"},
    .tags = {"c", "binding", "devices", "vibration", "haptics", "duration", "clamping", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = false,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). The real backend is queried but never actuated; request delivery is "
                    "proved through CNA's documented in-memory test backend, so no physical vibration is claimed.",
})
