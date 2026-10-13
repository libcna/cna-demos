// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.devices.camera_frames",
    .title = "Device services in C: camera frames",
    .description = "A C game enumerates camera descriptors without opening hardware, then uses CNA's public "
                   "synthetic camera to drive permission/loss states and copy exact RGBA frames into "
                   "caller-owned textures with size, empty-frame and lifetime checks.",
    .apis = {"cna_camera_device_info_init", "cna_camera_get_is_supported_ext",
             "cna_camera_get_count_ext", "cna_camera_get_info_at_ext",
             "cna_camera_get_name_size_at_ext", "cna_camera_copy_name_at_ext",
             "cna_camera_create_with_test_backend_ext", "cna_camera_get_state_ext",
             "cna_camera_get_frame_width_ext", "cna_camera_get_frame_height_ext",
             "cna_camera_try_acquire_frame_ext", "cna_camera_set_test_state_ext",
             "cna_camera_set_test_frame_ext", "cna_camera_destroy"},
    .tags = {"c", "binding", "devices", "camera", "texture2d", "frames", "lifecycle", "cnaext"},
    .needs = CnaDemos::Need::CApi,
    .deterministic = false,
    .restrictions = "Standalone C program (one C game per process); built only when CNA has its C API and device "
                    "extension (CNA_BUILD_C_API=ON, CNA_BUILD_DEVICES=ON). Hardware is enumerated but never "
                    "opened; all state/frame behavior uses CNA's synthetic camera. ISSUE-063 records stale C "
                    "header wording for the actual Opening/no-frame state.",
})
