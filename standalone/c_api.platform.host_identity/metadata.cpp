// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.platform.host_identity",
    .title = "Platform and product identity in C",
    .description = "A renderer-independent C program correlates target platform, Apple/mobile and "
                   "desktop identities, then owns and restores the process assembly title.",
    .apis = {"cna_platform_get_current", "cna_platform_get_is_apple_ext",
             "cna_platform_get_is_mobile_ext", "cna_platform_get_current_name_size_ext",
             "cna_platform_copy_current_name_ext", "cna_desktop_os_get_current",
             "cna_assembly_set_title_ext", "cna_assembly_copy_title_ext", "CNA_Platform",
             "CNA_DesktopOS"},
    .tags = {"c", "binding", "platform", "identity", "desktop", "assembly"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Standalone compile-time host identity and process title workflow. Values describe "
                    "the current build target; the previous process title is restored. The passing "
                    "embedded-NUL check records CNA-DEMOS-ISSUE-074's header mismatch.",
})
