// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.gesture_queue",
    .title = "Input in C: gesture queue",
    .description = "A C program constructs complete tap and free-drag values, round-trips the enabled mask, "
                   "enqueues both samples, proves mask changes preserve pending entries, reads FIFO order and "
                   "observes the empty-queue refusal without physical touch input.",
    .apis = {"cna_gesture_sample_init_from_values_ext", "cna_touch_panel_get_enabled_gestures",
             "cna_touch_panel_set_enabled_gestures", "cna_touch_panel_get_is_gesture_available",
             "cna_touch_panel_read_gesture", "cna_touch_panel_enqueue_gesture_ext", "CNA_GestureSample",
             "CNA_GestureType"},
    .tags = {"c", "binding", "input", "touch", "gesture", "queue", "fifo"},
    .automation = CnaDemos::Automation::Automated,
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Uses CNA's public gesture enqueue route; no physical touch device, "
                    "gesture detector timing or manual input is required.",
})
