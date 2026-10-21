# ISSUE-156: SDL_GPU stock sprite depth projection

## Status

**Resolved by CNA7097e1684cc7d4956b94224ab51f62f11970210d.** The commit contains
only the SDL_GPU Z/W fix, unchanged five-test native regression, bias fixture
coplanarity correction and156 reference row. No082 or inherited011 code was
staged. No demo assertion or knownIssue was changed.

External70430 terminated2, deliberately **not accepted globally**:132 cases,
123PASS/8FAIL/1unchangedSKIP; frozen inputs unchanged, peak164077568bytes,
all memory events0. Reviewed every one of nine delta rows: SDL_GPU's original
layer-depth demo changes FAIL-to-PASS; four begin_end_rules cases gain the
callback-state failure from082; four C resource_lifecycle cases change PASS-to-FAIL
for that same pending callback-contract correction. The other123 renderer/case
rows retain their status, check count, knownIssue and reported assertion data.
No additional156 regression was found. No C contract choice is implied.

The raw comparator flags nine rows, not eight, because the C++ demo log prints
failed checks but not successful checks: fixing156 removes its sole logged
assertion label. This is a reviewed harness false positive, not a product
regression or an oracle edit. The original raw audit remains unchanged and
unaccepted; all eight082 regressions remain visible. No blanket green claim.

Linux SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU runtime coverage only. The arithmetic
repair changes no lifetime/ownership code; no new projection-specific sanitizer
run, genuine-XNA GPU run or other SDL_GPU platform runtime is claimed.

The earlier candidate checkpoints below are retained as chronological evidence;
this closure supersedes their LIVE/pending/no-commit wording.

**Native98308 ACCEPTED; broader external70430 LIVE.** Full11005 completed0 in
1323219ms:10904PASS/101unchangedSKIP, all11000 old outcomes unchanged and exactly
five added depthPASS (comparison134ms). Graphics2917PASS/73unchangedSKIP;
frozen inputs unchanged, peak4301795328bytes, max/OOM/kill0. Sole heavy70430
now captures132 original external observations with every assertion/marker delta,
including the pending082 C failures; no full external acceptance is implied.

**Actual repaired focused gates accepted.** External96584
build98700ms, original20 pixel cases allPASS (SW828/GLES493/GL33451/SDL1517ms),
frozenunchanged/peak4300664832/maxOOM0. Only cna-content/libcna/libcna_c_api changed
in the319-artifact inventory. Bias61930 strict4564ms/run668ms: all72 labels and
outcomes byte-identical to the original baseline, frozenunchanged/no memory events.
Original demo98930: seven checks PASS on each renderer; every repaired RGBA pixel
equals the original correct SOFTWARE image. All four repaired images actually
viewed. No tolerance or golden update was used. Native five-test/graphics2990/
full11005 regression is accepted above; no156 product commit/closure yet.

Read-only platform boundary gates also passed on this candidate (2026-10-21):
`sdl_inventory.py --check` matches the measured plan inventory;
`sdl_classify.py --check` classifies all956 identifiers into16 areas;
`sdl_ratchet.py --check` stays at0 files/0 references outside its budget;
`renderer_sdl_audit.py --check` retains exactly fna3d/sdl-gpu as SDL renderer
edges; `hot_path_lint.py` finds no violation while watching181 contract methods
across1767 production sources. These static checks do not replace runtime tests.

Separate event candidate external observations72013 retain12 C++ passes but expose
four C lifecycle failures, one original callback-state assertion each out of30.
They are not projection failures and are not hidden or converted to expected
failures. The explicit public C contract choice remains pending, so the overall
external checkpoint is not accepted. See SPRITE_BATCH_TRIAGE.md.

CONFIRMED_OPEN; candidate repair has passed the focused/native gates above and
awaits broader external comparison. The original source was unchanged
when all baselines below were captured. No issue closure is claimed yet.

Measured minimized baseline49609: strict compile3141ms; original five pixel
witnesses PASS on each SOFTWARE/OPENGLES3/OPENGL33, FAIL on SDL_GPU. Runs640/431/
382/1064ms, frozenunchanged/OOM0. Original demo visual16525: seven checks on all
four renderers, exactly one existing depth assertion fails on SDL_GPU. All four
800x480 images were actually viewed. SOFTWARE/GLES/GL33 images are pixel-identical;
SDL_GPU differs in exactly16000 pixels, rectangle x560..759/y20..99, blue instead
of red. No other pixels differ. Original reports/images remain unchanged.

Original SDL_GPU depth-bias regression27937/v2:72/72 PASS, strict4566ms/run1154ms,
no validation warning/error, frozenunchanged/peak3960516608/all events0. The first
standalone compile27588 failed because backend-only flags lacked Game.hpp's
include directory. Its failed audit/log remain; v2 merges actual public consumer
includes/compatible definitions with renderer ABI definitions, leaving the test
source unchanged. The existing multi tree defaults to GLES and does not register
this SDL-only example, so no eight-renderer rebuild/reconfiguration was performed.

Candidate: QueueSprite now preserves transformedZ/transformedW. The separate
compiled path is unchanged. The existing bias helper explicitly defaults sprite
depth to0.5 (matching reference geometry0.5); all72 expected outcomes and bias
values are unchanged. Original five witnesses were copied byte-for-byte into
native graphics tests, not yet built there. The XNA decision row is recorded.
Sole heavy96584 performs one external build then all20 unchanged pixel witnesses.
It also incorporates the separately native/sanitizer-validated082 event candidate;
the pending explicit C callback contract prevents claiming full external closure.
Next: repaired bias72, original four visuals/exact pixels, native regression and
explicit lifecycle/C comparison. Do not weaken any pre-existing assertion.

The original preparation order was: finish the independent native-v2 gate88430,
audio stress sanitizer2, event sanitizer6 and original external16 lifecycle
capture first (all now accepted). `build-probe/repair-sprite-depth-before.mjs`
then collected20 actual case outcomes before the external library was rebuilt. It
verifies the old004 library hash, strict-compiles the unchanged witness and checks
actual SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU identity. It retains all failures and
refuses skips/setup errors; it does not assume the three control renderers pass.

## Read-only source/contract audit

Genuine Windows XNA decompilation:
`../xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework.Graphics/Microsoft.Xna.Framework.Graphics/SpriteBatch.cs`,
`SetRenderState`, lines877-928. Its sprite projection initializes M33=1 and M44=1
with no Z translation; the caller transform is multiplied by that matrix. The
half-pixel adjustments affect X/Y, not Z. Thus, for an identity transform, the
normalized sprite depth is layerDepth, not0.5-0.5*layerDepth. This is shipped-code
evidence, not a claim that a genuine XNA GPU reference was executed here.

Current renderer source:

- `SdlGpuRenderer::QueueSprite`, around5925: CPU-expanded stock sprites use
  `(0.5*transformedW-0.5*transformedZ)/transformedW`. Its comment incorrectly
  attributes a near=-1/far=1 projection to the reference.
- The same renderer's `ApplyCompiledSpriteVertexShaderEXT`, around13583,
  already uses `CreateOrthographicOffCenter(...,0,-1)`, the equivalent of
  M33=1/no Z translation. Avoid changing this already-correct separate path.
- EasyGL's stock sprite projection also uses near=0/far=-1 and documents its
  subsequent D3D-to-GL clip-depth conversion. Numerical cross-renderer pixel
  controls remain necessary; source similarity is not runtime validation.

## Existing depth-bias regression must be preserved

`modules/renderers/sdl-gpu/examples/sdlgpu_depth_bias_test.cpp` deliberately
compares normalized constant/slope bias, exact coplanarity and cache cardinality.
Its `RenderSprite` helper defaults to reference geometry depth0.5 but sprite
layerDepth0.0, relying on the incorrect projection to make them coplanar. Under
the correct XNA projection, explicitly setting the sprite depth to0.5 restores
the intended coplanar fixture without changing any expected color, bias factor,
nearby reference depth0.49975, tolerance or cache-cardinality assertion. This
fixture correction is implemented in the candidate and passed all72 original
checks. It does not replace the independent five-case pixel witness.

The explicit layerDepth0.5/referenceDepth0.1 control remains meaningful. The
sloped sprite pair uses M13=0.0025 and must also be exercised after repair.

## Separate source concern: not yet a confirmed finding

QueueSprite's CNAEXT ShaderEffect custom pipeline explicitly disables depth test
and writes regardless of the captured depth state (around6010). Its existing
depth-bias fixture intentionally expects green even with positive bias, unlike
the stock coplanar case. Do not silently expand the projection patch to this
different pipeline contract, and do not claim the stock fix validates custom
depth semantics. Review public extension documentation and reproduce this
separately; record a permanent new finding if the intended contract is violated.
