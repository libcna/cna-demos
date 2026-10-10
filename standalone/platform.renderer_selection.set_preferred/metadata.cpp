// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "platform.renderer_selection.set_preferred",
    .title = "Choose the renderer at runtime",
    .description = "Lists the renderers compiled into the program, selects one with "
                   "GraphicsRendererSelection::SetPreferred before the first GraphicsDevice exists, and "
                   "reports which renderer became active.",
    .apis = {"CNA::GraphicsRendererSelection::GetAvailable", "CNA::GraphicsRendererSelection::SetPreferred",
             "CNA::GraphicsRendererSelection::GetActive", "CNA::GraphicsRendererSelection::IsLatched",
             "GraphicsDevice::GetGraphicsRendererName"},
    .tags = {"renderer", "multi-renderer"},
    .frames = 5,
    .restrictions = "Standalone: CNA latches the renderer choice when the first GraphicsDevice of a process "
                    "is created, so selection can only be shown in a fresh process.",
})
