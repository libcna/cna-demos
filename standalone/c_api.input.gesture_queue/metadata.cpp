// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.gesture_queue",
    .title = "Input in C: gesture queue and scaling",
    .description = "A C program constructs complete tap and free-drag values, round-trips the enabled mask, "
                   "enqueues both samples, proves mask changes preserve pending entries, reads FIFO order, then "
                   "scales a normalized raised free drag with published display metrics while keeping the touch "
                   "snapshot separate.",
    .apis = {"cna_gesture_sample_init_from_values_ext", "cna_touch_panel_get_enabled_gestures",
             "cna_touch_panel_set_enabled_gestures", "cna_touch_panel_get_is_gesture_available",
             "cna_touch_panel_read_gesture", "cna_touch_panel_enqueue_gesture_ext",
             "cna_touch_panel_get_display_width", "cna_touch_panel_set_display_width",
             "cna_touch_panel_get_display_height", "cna_touch_panel_set_display_height",
             "cna_touch_panel_get_display_orientation", "cna_touch_panel_set_display_orientation",
             "cna_touch_panel_raise_touch_event_ext", "cna_touch_get_state", "CNA_GestureSample",
             "CNA_GestureType"},
    .tags = {"c", "binding", "input", "touch", "gesture", "queue", "fifo", "scaling", "orientation"},
    .automation = CnaDemos::Automation::Automated,
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Uses public authored and raised-gesture routes; no physical touch "
                    "device, detector timing boundary or manual input is required.",
})
