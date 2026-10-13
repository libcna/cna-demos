// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.device_capabilities",
    .title = "Graphics device capabilities in C",
    .description = "A C game borrows its live graphics device and audits its renderer identity, legacy and "
                   "detailed capability profiles, numeric limits, surface-format support, shader dialect, "
                   "English diagnostic report and logical backbuffer information.",
    .apis = {"cna_graphics_device_get_renderer_info", "cna_graphics_device_copy_renderer_name",
             "cna_graphics_device_supports_capability",
             "cna_graphics_device_get_renderer_feature_support_ext",
             "cna_graphics_device_get_renderer_limit_ext",
             "cna_graphics_device_get_surface_format_support_ext",
             "cna_graphics_device_get_capability_report_size_ext",
             "cna_graphics_device_copy_capability_report_ext",
             "cna_graphics_device_get_shader_dialect_ext",
             "cna_graphics_device_get_backbuffer_info", "CNA_RendererInfo", "CNA_BackBufferInfo",
             "CNA_RendererFeature", "CNA_RendererLimit", "CNA_RendererFormatUsageFlags",
             "CNA_ShaderDialect"},
    .tags = {"c", "binding", "graphics", "device", "renderer", "capabilities", "formats", "diagnostics"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; capability values are renderer and host specific. The English report "
                    "is diagnostic text rather than a machine protocol. No rendering or visual result is claimed. "
                    "Built only with CNA_BUILD_C_API=ON.",
})
