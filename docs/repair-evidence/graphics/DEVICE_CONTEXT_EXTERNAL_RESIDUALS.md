# Additional renderer findings from the ISSUE-086/112 regression

**Resolved 2026-10-24 on the owner's decisions.** DC-R1: CNA `51fb39d5a` binds RenderTargetCube
faces in any slot of an SDL_GPU target set; `c_api.graphics.render_target_bindings` passes its 25
checks on SDL_GPU, and CNA's `cube_mrt_binding` conformance passes M1-M4 there. DC-R2: CNA
`c7f6c68e5` makes SDL_GPU apply a mask keeping every sample or none exactly (a no-sample draw is
dropped, which writes nothing) and refuse every other mask, and `RendererFeature::MultiSampleMask`
reports Restricted there; `graphics.render_target.multisampling` passes on SDL_GPU. Arbitrary
masks remain impossible on SDL 3.5 and are refused, not emulated. DC-R3, found in that SDL_GPU
sweep: `c_api.graphics.shader_effect_state` created its effect from a GLSL placeholder under
SDL_GPU's SPIR-V dialect; this repository's `77444d7` supplies a compiled SPIR-V pair, and CNA
`9ef292993` stops `cna_shader_effect_create` refusing SPIR-V modules as non-UTF-8 text. The text
below is the original DC-R1/DC-R2 triage, kept as written.

These findings remain **open for contract/coverage triage**. They are not passes,
new expected failures, or changes to the historical156-ID inventory. They must be
disposed of before final campaign closure. No demo assertions/metadata changed.

The completed external49730 run has132 exact previous outcomes (123PASS,8 separate
ISSUE-082 FAIL,1SKIP),8 DDS/C controls PASS, and192 newly selected device/target/
texture executions:183PASS,7 declaredSKIP,2FAIL. All original086/112 reproductions
pass. The gate correctly stopped before visual comparison. No input drift/OOM;
peak186814464 bytes, all memory.events zero.

## DC-R1: SDL_GPU C API mixed cube/2D MRT

`c_api.graphics.render_target_bindings`: four of25 checks fail, starting with the
mixed2D/cube-face bind; the three collection-copy checks then fail consequentially.
`SdlGpuRenderer::SetRenderTargets` explicitly throws NotSupportedException for a
cube face in a multi-target set. Its current source is unchanged from CNA HEAD.
`plans/plan_sdlgpu.md` (SDLGPU-75/90, MRT binding/clear ledger) explicitly documents
2D-only MRT and cube-face refusal. Existing native tests skip this unsupported
case. This is evidence of a pre-existing documented implementation boundary, not
proof that the C demo's renderer scope is correct. Review the public C contract
and its capability reporting; preserve positive MRT ordering/atomicity checks on
supported renderers. Do not quietly skip or delete the failing assertion.

## DC-R2: SDL_GPU sample masks ignored

`graphics.render_target.multisampling`: two of6 checks fail. Both BlendState and
GraphicsDevice zero masks render5 grey levels instead of only black. Current
`SdlGpuRenderer::ApplyBlendState` explicitly omits sample masks, citing SDL's
reserved sample_mask/enable_mask fields. `plans/plan_sdlgpu.md` SDLGPU-58 explicitly
classifies arbitrary masks as unsupported, explaining why arbitrary shader
rewriting does not preserve depth/stencil side effects. This source is unchanged
from HEAD. The comment claiming EasyGL also lacks masks is stale relative to
campaign repairs and should be corrected during this residual's contract audit.
Determine whether CNA adequately exposes this deliberate limitation and whether
the demo overclaims its renderer scope. No zero-mask-only hack is an acceptable
implementation of arbitrary masks. Do not claim the demo passed.

## Why these are independent of the context candidate

The complete production candidate adds only calls to
GraphicsDevice::AcquireRendererThreadContextLease and bounds their lifetime.
That helper delegates to IGraphicsRenderer::AcquireThreadContextLeaseEXT.
SdlGpuRenderer does not override it; the inherited implementation returns nullptr
without changing state. Thus the new calls cannot change either SDL_GPU path.
This is a source-based independence argument, not a pre-patch runtime rerun.
The rejection/ignored-mask source and its public plan predate this batch. All
runtime failures remain in the original external audit and must remain visible
in the final report until their contracts are resolved.

Next: finish the independent six exact visual comparisons and scoped086/112
metadata validation. Carry DC-R1/DC-R2 explicitly in the campaign handoff/matrix;
do not equate scoped batch acceptance with a green full regression campaign.
