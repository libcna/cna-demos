// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "platform.renderer_selection.set_preferred",
    .title = "Choose the renderer at runtime",
    .description = "Lists the renderers compiled into the program, requests an absent renderer by name, "
                   "falls back to an available renderer through an explicit chain, inspects the rejection history and "
                   "proves that selection settings are immutable after the first GraphicsDevice exists.",
    .apis = {"CNA::GraphicsRendererSelection::GetAvailable", "CNA::GraphicsRendererSelection::SetPreferred",
             "CNA::GraphicsRendererSelection::GetSelected", "CNA::GraphicsRendererSelection::IsAvailable",
             "CNA::GraphicsRendererSelection::GetActive", "CNA::GraphicsRendererSelection::IsLatched",
             "CNA::GraphicsRendererSelection::SetFallbackChain",
             "CNA::GraphicsRendererSelection::EnableAutomaticFallback",
             "CNA::GraphicsRendererSelection::IsFallbackEnabled",
             "CNA::GraphicsRendererSelection::GetFallbackHistory",
             "CNA::GraphicsRendererFallbackRecord", "CNA::GraphicsRendererFallbackReason",
             "CNA::getGraphicsRendererFallbackReasonName",
             "GraphicsDevice::GetGraphicsRendererName"},
    .tags = {"renderer", "multi-renderer", "fallback", "lifecycle"},
    .frames = 5,
    .restrictions = "Standalone: CNA latches the renderer choice when the first GraphicsDevice of a process "
                    "is created, so selection can only be shown in a fresh process. The configured build must "
                    "contain at least one usable renderer and omit at least one public renderer identity.",
})
