// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.touch_state",
    .title = "Input in C: touch snapshots",
    .description = "A C program uses CNA's public touch fixture routes to create a deterministic press, move and "
                   "release, captures each CNA_TouchState, finds the finger by id and reconstructs its previous "
                   "location without requiring touch hardware.",
    .apis = {"cna_touch_get_capabilities", "cna_touch_get_state", "cna_touch_state_find_by_id",
             "cna_touch_location_try_get_previous", "cna_touch_panel_set_touch_device_exists_ext",
             "cna_touch_panel_set_finger_ext", "cna_touch_panel_update_ext",
             "cna_touch_panel_reset_for_tests_ext", "CNA_TouchCapabilities", "CNA_TouchState",
             "CNA_TouchLocation"},
    .tags = {"c", "binding", "input", "touch", "snapshot", "lifecycle"},
    .automation = CnaDemos::Automation::Automated,
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Uses CNA's public test fixture routes; no physical touch device or "
                    "manual input is required.",
})
