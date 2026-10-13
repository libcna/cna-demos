// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.graphics.device_states",
    .title = "Graphics-device states in C",
    .description = "A C game enumerates every blend, depth/stencil, rasterizer and sampler preset, "
                   "round-trips distinctive complete object, sampler, scalar and geometric states through the live "
                   "graphics device, verifies identity and ownership, then restores every original value.",
    .apis = {"cna_blend_state_init", "cna_depth_stencil_state_init",
             "cna_rasterizer_state_init", "cna_sampler_state_init",
             "cna_graphics_device_get_blend_state", "cna_graphics_device_set_blend_state",
             "cna_graphics_device_get_depth_stencil_state",
             "cna_graphics_device_set_depth_stencil_state",
             "cna_graphics_device_get_rasterizer_state",
             "cna_graphics_device_set_rasterizer_state",
             "cna_graphics_device_get_sampler_state", "cna_graphics_device_set_sampler_state",
             "cna_graphics_device_get_is_disposed", "cna_graphics_device_get_status",
             "cna_graphics_device_get_adapter_index", "cna_graphics_device_get_graphics_profile",
             "cna_graphics_device_dispose", "cna_graphics_device_get_scissor_rectangle",
             "cna_graphics_device_set_scissor_rectangle", "cna_graphics_device_get_blend_factor",
             "cna_graphics_device_set_blend_factor", "cna_graphics_device_get_multi_sample_mask",
             "cna_graphics_device_set_multi_sample_mask", "cna_graphics_device_get_reference_stencil",
             "cna_graphics_device_set_reference_stencil",
             "CNA_BlendState", "CNA_DepthStencilState", "CNA_RasterizerState",
             "CNA_SamplerState"},
    .tags = {"c", "binding", "graphics", "state", "blend", "depth", "rasterizer", "sampler"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .restrictions = "Standalone C program; live graphics-device state is restored before the load "
                    "callback returns. No drawing or visual result is claimed. Built only with "
                    "CNA_BUILD_C_API=ON.",
})
