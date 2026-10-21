# ISSUE-004 — live Immediate sampler propagation

## Scope and contract

Validated repair commit: CNA `9359a26b61ded101eff35ee4008fbee3a490e9dc`.
Pre-repair CNA `d2bef7020cf23a72e44834f1c955908dc5b4963f`;
external evidence baseline demos `a445241795fc9bcb971043d0536657b89cbdcb89`.
The product change is confined to `SpriteBatch::flushSingle`: send the current
device slot-zero sampler through the existing sprite-renderer state hook before
each ordinary Immediate draw. All seven sampler properties are forwarded with
the same normalization as the existing Begin path. Deferred state timing and
the stock-effect path are unchanged. No backend-specific workaround is added.

The genuine XNA Windows decompilation in sibling `xna4-decomp` shows `Begin`
calling `SetRenderState` for Immediate, `End` doing so only for other modes, and
`RenderBatch` drawing without resetting the sampler. The historical XNA
`SpriteSortMode.Immediate` specification and original demo therefore agree:
device state changes between Draw calls must affect subsequent sprites.
This source inspection is not a claim of running native XNA GPU rendering.

## Before repair

`repair-sprite-baseline-*`: 29 original sprite/sampler/C/texture demos on SOFTWARE,
OPENGLES3 and OPENGL33: 80 PASS, six FAIL (004 and082 on each renderer), one GLES
LOD-bias capability SKIP. Original assertions and all frozen inputs unchanged.
All three original sampler screenshots were actually inspected: both sprites
were point-filtered. The passing blend-state sibling remains a regression control.

`repair-sprite-sampler-before-v3-*`: strict native HiDef pixel witness reproduces
three failures on OPENGLES3; its Deferred control passes. These four tests cover
live filter change, point/linear/point sequencing, address change against an
independent wrap reference, and Deferred sampler application at End. The fixture
was copied byte-for-byte to native graphics tests before repairing CNA.

Earlier `before` and `before-v2` runs are retained as harness setup failures:
the first forced unavailable SOFTWARE in the native OPENGLES3 build, the second
used Reach where backbuffer readback is refused. Neither establishes a product
defect. The accepted third run explicitly uses HiDef without changing assertions.

## Validation checkpoint

Native session42945 completed0 and was accepted:
build PASS (42,759ms), focused four PASS (331ms), graphics 2,906 PASS /73 SKIP
(55,342ms). Full10,893 PASS /101 unchanged SKIP (1,655,437ms). Exact comparison
retains all10,990 previous outcomes and adds exactly the four new pixel PASS.
No frozen source drift; memory peak4,299,186,176bytes, high8179, max/OOM events0.
No compiler warning diagnostics were found. The run uses one compiler worker,
shared ccache, a private GPU display and network namespace, repository-owner UID,
zero Linux capabilities, and a6GiB memory limit with no swap.

Sanitizer93821 completed0: original four SOFTWARE pixels PASS with actual
SpriteBatch/Texture2D/GraphicsResource and Runtime ASan/UBSan/float-cast/leak
instrumentation, no suppression and no findings. Closure build20,836ms, strict
fixture build6,387ms, run298ms, inspect35ms. Frozen inputs unchanged;
peak4,297,469,952bytes/high5156/max0/OOM0. No vendor instrumentation claim.

External86378 completed its build (82,903ms), all116 observations and the original
four native pixel witnesses on SOFTWARE/OPENGLES3/OPENGL33 (all12 PASS). Its final
evaluator exited1 because it incorrectly required a previously failed game
assertion label to remain in stdout after PASS. `DemoContext::Check` increments
every check but prints only failures; C demos have a different successful-label
logger. The original audit/logs remain untouched. Read-only reevaluation83834
checked the original frozen source hash, all raw JSON/logs, unchanged check counts,
all build/native stage exits and all116 outcomes. Corrected `external-v2-audit`
accepts exactly four004 FAIL-to-PASS transitions and no other delta, without a
rebuild/rerun or weakened assertion. Frozen inputs unchanged; peak4,298,768,384
bytes/high8406/max0/OOM0. No untested native SDL_GPU pixel-fixture claim: its
coverage comes from the real original demos.

The pre-rebuild SDL_GPU baseline additionally exposes one unmarked existing
`layer_depth_sorting` failure: the depth-test assertion fails, the other six
controls pass. It remains byte-for-byte the same failure after the sampler fix.
This is explicitly retained for a separate investigation, not hidden or marked
expected. ISSUE-082's failure also remains on all four renderers; GLES's LOD-bias
capability skip remains. Aggregate116 before:106PASS/9FAIL/1SKIP; after:
110PASS/5FAIL/1SKIP. These residuals do not constitute a global green sweep.

Actual repaired sampler and blend screenshots were inspected on all four
renderers. Left sprite remains point-filtered; right sprite becomes linearly
filtered. Exact RGBA checks preserve each left rectangle and the entire blend
control framebuffer. GLES and GL33 sampler images are RGBA-identical. SOFTWARE's
minor edge irregularities were already present and remain unchanged. Image hashes
and observations are recorded in `repair-sprite-sampler-visual-review.json`.

Final before1478 and after58657 both completed0. Exactly one knownIssue004 line
was removed, no assertion edited; catalog otherwise identical. Incremental build
12,230ms, eight original Immediate sibling runs PASS, five focused/catalog CTests
PASS (23,928ms), audit44ms. Frozen sources and non-catalog artifacts unchanged;
peak4,296,110,080bytes/high3266/max0/OOM0. No source/manifest suppression.

KnownIssue004 is removed and all demo assertions retained. Final disposition:
FIXED_VALIDATED, limited to the actual tests/renderers above. Reports are under
`build/linux-full/reports/repair-sprite-*`.
Separate003 binding/lifetime and082 disposal/reference findings are tracked in
`SPRITE_BATCH_TRIAGE.md`; they are not silently closed by this sampler repair.
