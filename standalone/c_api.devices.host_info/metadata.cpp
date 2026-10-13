// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.devices.host_info",
    .title = "Device services in C: host information",
    .description = "A C game reports CNA's device identity and extension availability, reads a coherent host-power "
                   "snapshot, CPU and RAM capacity, copies every preferred language/country code with exact UTF-8 "
                   "bounds, and compares windowless/windowed display scale and safe-area answers.",
    .apis = {"cna_environment_get_device_type", "cna_devices_ext_is_available",
             "cna_power_get_state_ext", "cna_power_get_battery_percent_ext",
             "cna_power_get_seconds_remaining_ext", "cna_system_info_get_logical_cpu_core_count_ext",
             "cna_system_info_get_system_ram_megabytes_ext", "cna_locale_get_preferred_count_ext",
             "cna_locale_get_language_size_at_ext", "cna_locale_copy_language_at_ext",
             "cna_locale_get_country_size_at_ext", "cna_locale_copy_country_at_ext",
             "cna_display_info_get_content_scale_ext", "cna_display_info_get_safe_area_ext"},
    .tags = {"c", "binding", "devices", "host", "power", "system", "locale", "display", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = false,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API and device "
                    "extension (CNA_BUILD_C_API=ON, CNA_BUILD_DEVICES=ON). Values describe the executing host and "
                    "renderer window, so exact counts, locales, power and display geometry are environment-specific.",
})
