// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.input.touch_state",
    .title = "Input in C: touch snapshots and values",
    .description = "A C program uses CNA's public touch fixture routes to create a deterministic press, move and "
                   "release, captures each CNA_TouchState, finds the finger by id and reconstructs its previous "
                   "location. Local touch values then show pressure-independent identity, insertion-copy and "
                   "collection edits without requiring touch hardware.",
    .apis = {"cna_touch_get_capabilities", "cna_touch_get_state", "cna_touch_state_find_by_id",
             "cna_touch_location_try_get_previous", "cna_touch_panel_set_touch_device_exists_ext",
             "cna_touch_panel_set_finger_ext", "cna_touch_panel_update_ext",
             "cna_touch_panel_reset_for_tests_ext", "CNA_TouchCapabilities", "CNA_TouchState",
             "CNA_TouchLocation", "cna_touch_location_init_with_previous_ext",
             "cna_touch_location_init_from_values_ext", "cna_touch_location_equals",
             "cna_touch_location_get_hash_code", "cna_touch_state_init_from_locations",
             "cna_touch_state_get_is_read_only", "cna_touch_state_get_is_empty_ext",
             "cna_touch_state_contains", "cna_touch_state_index_of", "cna_touch_state_copy_to",
             "cna_touch_state_add", "cna_touch_state_clear", "cna_touch_state_remove",
             "cna_touch_state_remove_at", "cna_touch_state_insert"},
    .tags = {"c", "binding", "input", "touch", "snapshot", "lifecycle", "collection", "pressure"},
    .automation = CnaDemos::Automation::Automated,
    .needs = CnaDemos::Need::CApi,
    .deterministic = true,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API "
                    "(CNA_BUILD_C_API=ON). Uses CNA's public test fixture routes; no physical touch device or "
                    "manual input is required.",
})
