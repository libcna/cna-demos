# Sprite texture binding (ISSUE-003)

## Current evidence boundary (2026-10-21)

**Latest checkpoint2026-10-22: native68394 ACCEPTED; sanitizer83125 LIVE.**
CNA status4f3ed89e5/demos35d2a3d; product003 remains uncommitted/open.
Native-unbound: build11472ms/no diagnostics, focused22=13PASS/9 compiledSKIP
2667ms, corrected DXT refillPASS342ms, graphics3012=2930PASS/82SKIP43532ms.
Full11027 completed1174074ms:10917PASS/110SKIP, no failures. Exact compare112ms
retains every original11005 identity/outcome, adds only13PASS/9 compiledSKIP;
no missing/changed old outcome. Inputs unchanged, peak3681579008 bytes, all
memory events0. Original47617 DXT failure preserved; fixture precondition repair
and its two added guard assertions are still own uncommitted003 work.

Old native scope inactive/dead, no native child remains. A transient host
ninja/cc1plus was observed before sanitizer launch and confirmed absent on the
next process check; nothing was killed and no second heavy job launched.
Sole heavy83125 scope cna-repair-sprite-binding-sanitizer, demos driver
build-probe/repair-sprite-binding-sanitizer.mjs, existing build-asan.
Native accepted audit is its hard prerequisite. Private GPU, one compiler,
shared cache, MemoryHigh4GiB/Max6GiB/no swap. Configuration6395ms exit0 enables
SOFTWARE compiled effects after exact shared-dependency safety checks; instrumented
library closure build now running. It requires original22 PASS/noSKIP under
ASan/UBSan/float-cast/leaks. No sanitizer result yet. Production/dependency
inputs frozen again; do not start another heavy job or edit frozen sources.

Next collect83125 and inspect diagnostics, all22 outcomes, memory and source
stability. Then external C API/demo ABI-consumer rebuild, focused/full external
regression deltas and visual comparisons, and available unbuilt renderer checks.
The separate082 C callback regressions remain unaccepted and owner choices
remain pending. Prepared027/066 source triage/six uncompiled witnesses are in
demos3e74913, outside the current frozen set; finish003 before that repair batch.
All156 IDs retained,105final/51pending. No product commit/closure/push/merge.


**Latest continuation: native68394 LIVE; older live notes below are historical.**
CNA0d1a9bdfb/demosc07f1bd verified; old campaign scope inactive/dead.
84495 terminal: build0/897240ms, then diagnostic guard1; no tests ran.
All45 warnings (44 nodiscard, one volatile) exactly match the original baseline
and13 HEAD-identical files; no suppression or warning-free claim.
Reviewed47617 terminal1: focus13PASS/9 compiledSKIP/906ms;
graphics3012=2929PASS/82SKIP/1FAIL/40304ms. Exact old2990 comparison has only
ClassicTextureFormat.DxtAlphaBlocksSampleTheirOwnEncoding PASS->FAIL.
No old case missing;22 additions13PASS/9SKIP. No full-native run was reached.
Frozen inputs unchanged; peak672497664 bytes/all memory events0.

Genuine local XNA SpriteBatch.RenderBatch leaves Textures[0] bound, and
Texture2D.CopyData refuses SetData while bound. The DXT fixture wrongly refilled
the sampled texture without unbinding. Added identity/refusal assertions and
explicit unbind before refill. All original RGB/alpha assertions, bytes and
tolerances remain unchanged; original failing XML retained. No product guard
weakened. This fixture is own003 work, not inherited011.

Sole heavy68394, scope cna-repair-sprite-binding-native-unbound, demos driver
build-probe/repair-sprite-binding-native-unbound.mjs. One worker/shared cache/
existing debug tree/private GPU+network/caps0/6GiB/no swap. Exact fixture-only
diff verified and other inputs frozen. Incremental build11472ms exit0 without
diagnostics; original22 focus2667ms exit0; corrected refill342ms PASS.
Graphics3012 completed43532ms exit0:2930PASS/82SKIP, no failure. Full11027
and exact old11005 comparison are still running/pending. Host scope verified
active/running, CnaTests PID2417036, peak2646577152 bytes under6GiB.
Inventory audit:156 unique IDs exactly match all156 issue headings; no missing
or extra entry. Final dispositions:71 fixed,23 documentation,8 already fixed,
one platform-validation pending, one demo correction, one duplicate;51 pending.
Continuation2026-10-22:68394 still verified live; the long differential build
case PASS543003ms and the run has reached effect-compiler tests. Host peak
3176636416 bytes under6GiB. Prepared sanitizer prerequisite now points to
native-unbound-audit.json, with an unchanged hard requirement for successful
final compare/no drift; host syntax/transform checkPASS, actual run still UNRUN.
Next collect68394 and inspect the complete audit before launching sanitizer.
Sanitizer/external ABI rebuild+regression/visuals/unbuilt renderer checks remain.
Preserve011/082 and owner choices.003 open,105final/51pending; no push/merge.


Native84495 still live, build passed251/347. Its zero-diagnostic guard will
require review of13 `nodiscard` warnings in unchanged
`GamerServicesServiceTests.cpp`. Read-only comparison confirms all13 exact lines
already occur in `repair-baseline-native-build.log`; no new diagnostic is present
at this checkpoint. No test/source/assertion change or warning suppression.
Prepared `build-probe/repair-sprite-binding-native-observe.mjs` is syntax/transform
checked, **UNRUN**. Only after84495 is terminal: verify completed build exit0,
exact13 diagnostics and unchanged HEAD/source hashes, then run the original
native22/graphics3012/full11027 with exact old11005 parity, without rebuilding.
The first wrapper preparation had a JavaScript quoting syntax error, detected
before executing any test/build and corrected; no product failure inferred.
Sanitizer prerequisite now names this future observed native audit, which does
not exist yet. If any additional diagnostic appears, review it rather than
bypassing the exact check. No native acceptance or sanitizer result claimed.

Continuation audit while84495 remains live: exact identities across the21-case
baseline plus pixel baseline and repaired22-case gate reconcile88 observations:
**70PASS/9FAIL/9SKIP -> 79PASS/9SKIP**, exactly nineFAIL->PASS and no other delta.
This includes every renderer/case identity, not just aggregate counts; all three
source-stability audits are unchanged. All five platform-boundary gates pass:
inventory matches,956 SDL identifiers/16 areas, renderer allowlist exactly
fna3d+sdl-gpu, ratchet0/0, hot-path181 methods across1769 production sources.

Prepared `build-probe/repair-sprite-binding-sanitizer.mjs` passes host syntax/
transformation checks, **UNRUN**. Requires accepted native84495. Reuses existing
build-asan and safely enables SOFTWARE compiled effects, verifies shared patched
dependency hashes/133-patch no-op stamp before configure, instruments actual
SpriteBatch/Texture2D/GraphicsResource/SpriteTextureBinding/Software compiled
paths and Sharp Runtime, then requires original22PASS/noSKIP with ASan/UBSan/
float-cast/leak detection. No vendor reset or warning suppression. Only the exact
original16 Draco diagnostics may be documented if rebuilt; any other diagnostic
fails the gate. No sanitizer or compiled-lifetime memory-safety result yet.

**Retry candidate focused gate ACCEPTED45982; native84495 LIVE.** Build of
`cna_shared`148080ms, strict original22 compilation10008ms. SOFTWARE1452ms,
OPENGLES3832ms, OPENGL33814ms each22PASS; SDL_GPU2267ms13PASS/9 compiledSKIP.
Every original before-failure assertion remains unchanged, including exact64-pixel
control/retry equality and the nontransparent control. The six failing compiled
lifecycle observations and three failing pixel observations now PASS; all earlier
passing controls retained. Frozen inputs unchanged; peak1653633024, all memory
events0. This is focused acceptance only, not native/external/sanitizer closure.

Native84495 is the sole heavy job, scope `cna-repair-sprite-binding-native`, driver
`build-probe/repair-sprite-binding-native.mjs`. Existing debug build, one compiler,
shared cache/private GPU+isolated broadcast network, owner UID/all capabilities
dropped, MemoryHigh4GiB/Max6GiB/no swap. It checks native22, graphics3012 and
full11027 against the previous11005 outcome identities. Native compiled support
is deliberately unchanged/OFF, so the new native cases must be13PASS/9 explicit
compiledSKIP; enabled compiled routes have actual external evidence above.
Inputs frozen again. External ABI consumers, sanitizer, visuals and additional
renderer paths remain pending. No product commit/marker change/003 closure.

**Retry candidate now validating as45982.** Reproductions are committed in
demos2fdf525. The correction adds `FlushPendingSpritesEXT` and noexcept
`DiscardPendingSpritesEXT` to the internal sprite-renderer contract. The common
End submits pending work before entering renderer-shutdown recovery; on failure
it discards backend staging but retains the original deferred queue/Begin pair.
Immediate Draw discards failed staging and rethrows, leaving no queue for End
to replay. All six affected renderer families delegate to their existing flush
and clear only unsubmitted geometry. Already submitted side effects remain.
Existing synthetic renderer-End failure behavior is preserved by the separate
boundary. The original pixel witness is native-registered byte-identically.

Sole heavy45982: `cna-repair-sprite-binding-retry-candidate`, driver
`build-probe/repair-sprite-binding-retry-candidate.mjs`. Existing external shared
runtime target `cna_shared` only, one worker/shared cache/private GPU/6GiB/no swap;
then all22 unchanged witnesses on four renderers. This is NOT a rebuild of the
C API/demo ABI consumers; rebuild those before external regression. Reports
`repair-sprite-binding-retry-candidate-*` freeze source inputs. No result yet,
no product commit, marker change or003 closure. Do not edit frozen sources.

**Enabled compiled baseline now captured; no heavy job remains.**48754
terminated1 only at the diagnostic gate: configure7428ms/build721899ms both
exit0, frozen sources unchanged, peak4304211968/high4145/max0/OOM0/kill0.
The16 Draco deprecated implicit-this-capture warnings are all present verbatim
(apart from path prefix) in `repair-baseline-native-build.log`. No warning was
suppressed or vendor source edited. The first continuation9598 failed before
compilation because Draco has a gitlink but no nested Git metadata; its headers
cannot be read with parent `git show HEAD:path`. No clean-submodule claim is made.

Corrected observation82479 terminated1 with complete unchanged-source results:
strict21 compilation6437ms; SOFTWARE1365ms, OPENGLES3752ms, OPENGL33738ms each
**19PASS/2FAIL**, SDL_GPU2069ms **13PASS/8 compiledSKIP**. Peak472776704,
all memory events0. The original public-state/stock/compiled binding controls
pass. Both failure paths are genuine: disposed-before-flush calls OnApply twice
inside the first End (cleanup calls SetCustomEffect, which flushes again), and
failed Immediate Draw remains in the private queue and is replayed by End.
Original test sources/assertions are unchanged.

Pixel baseline44928 terminated0 as an observation, **not a product pass**:
strict2073ms; SW58/GLES252/GL33193ms eachFAIL, SDL168ms capabilitySKIP.
Every enabled-renderer nontransparent control passes. Injected failure calls
OnApply twice, clears the Begin pair, rejects the retry and leaves transparent
pixels instead of the control. Exact64-pixel comparison fails as intended.
Peak446980096/all events0/frozen unchanged. All raw failures are preserved in
`repair-sprite-binding-retry-pixels-before-*`;21-case evidence is
`repair-sprite-binding-compiled-observed-*`, with explicit diagnostic review.

Next repair: separate pending sprite submission from renderer End, discard
unsubmitted backend staging on a failed submission, preserve the front-end
deferred queue/Begin pair for retry, and discard failed Immediate staging without
replaying it. Preserve existing native renderer-End recovery and retained-texture
lifetime tests. Do not roll back already rendered side effects or weaken the
original22 witnesses. Native/full external/visual/sanitizer validation remains.

Continuation revalidated48754 through its live tool handle and host systemd
scope: active/running, MemoryCurrent4263153664/Peak4301078528 bytes, hard cap
6442450944. No second heavy job or production edit. The existing build has
passed step1461/1878; that is progress, not test acceptance.

`SpriteCompiledRetryPixelTests.cpp` and the syntax-checked serial driver
`build-probe/repair-sprite-binding-retry-pixels.mjs` are prepared, **UNCOMPILED/
UNRUN**, outside48754's frozen21. The pixel witness compares a successful
two-pass compiled draw with a retry after the first OnApply throws. The existing
conformance effect's second pass uses SrcAlpha blending, so a retained backend
quad replayed twice changes the image. The control must draw nontransparent
pixels; no reference-image regeneration or tolerance is involved. This prevents
a shallow retry repair that preserves the Begin pair but duplicates queued work.
Run only after48754 is terminal and its complete source-stable audit is inspected;
the driver deliberately permits recorded product failures in that preceding
audit but refuses incomplete observations or source drift. No new runtime result
or closure is claimed.

**Latest actual result:**78015 TERMINAL0. External build1678789ms, strict
original16 witness build4948ms, SW/GLES/GL33/SDL_GPU106/678/598/1662ms. Each
renderer has13PASS/3 compiledSKIP: all11 originally failing tests per renderer
now PASS, and both original passing controls stay PASS. All frozen inputs unchanged.
Peak4298645504 bytes, high1262/max0/OOM0/kill0. No native/full-external/sanitizer
acceptance, no003 closure. Exact original test sources remain unchanged.

The FNA3D identifier-only correction and parameter documentation are applied;
that renderer is not yet compiled. The five new compiled-lifetime/error tests
are native-registered byte-identically, still uncompiled/unrun. Sole heavy48754
now runs `repair-sprite-binding-compiled-enabled.mjs`: configuration7428ms/exit0,
actual SOFTWARE/EasyGL compiled options ON, SDL_GPU OFF. Shared dependency
helpers both explicitly reported already applied/skipping. Serial existing-tree
build is live under6GiB/no swap/private GPU. Collect its21-case observations on
each renderer before editing frozen sources; no successful compiled runtime
claim until actual results. Below, the78015 launch/preparation notes are history.

### Candidate now in progress (not accepted)

Demos evidence commit `82ee978` preserves the following baselines. A subsequent
uncommitted CNA candidate adds `SpriteTextureBinding`, a lazy per-Texture2D
movable/invalidatable public identity, common stock-effect publication and narrow
binding hooks in SOFTWARE/EasyGL/SDL_GPU/FNA3D/D3D9/WebGPU. Renderer-held runs flush
before changing public identity; effect paths publish after Apply. Native copies
of the original6+7+3 witnesses are byte-identical. Inherited011 ownership hunks
and pending082 event changes are not part of this candidate.

First validation is live as tool78015, scope `cna-repair-sprite-binding-candidate`,
driver `build-probe/repair-sprite-binding-candidate.mjs`. Its source manifest and
progress log are `build/linux-full/reports/repair-sprite-binding-candidate-*`.
It uses the existing external tree, one compiler, shared cache and a private GPU
display, MemoryHigh4GiB/MemoryMax6GiB/no swap. Texture2D/interface layout changes
require rebuilding consumers and the probes; do not run the old probe executables
against the changed ABI. No focused result is claimed before the audit completes.

`SpriteCompiledBindingLifetimeTests.cpp` adds five **UNCOMPILED/UNRUN** witnesses
outside that frozen input set: deferred failure/retry, distinct copied-wrapper
runs, destroyed wrappers, moved wrappers and no replay of failed Immediate Draw.
The copy test describes CNA's C++
extension, not a claim of XNA value-copy textures. Read-only analysis identifies
a further risk: compiled flush exceptions originating inside renderer End enter
SpriteBatch's renderer-End recovery branch, whereas genuine XNA End clears its
Begin pair only after Flush succeeds. Establish actual enabled compiled-runtime
evidence before deciding the correction; do not weaken the repeated-End assertion.
An enabled configuration is still required. Existing `cna/cmake-build-multi`
already references shared `/home/robertvokac/deps/FNA3D`; reuse that dependency if
enabling SOFTWARE/EasyGL compiled support, never clone another copy. Do not
reconfigure or launch a second heavy job while78015 is live. Native regression,
all external deltas, lifetime sanitizers and unbuilt renderer paths remain pending.

Continuation preparation while78015 remains live: genuine local XNA SpriteBatch
InternalDraw calls RenderBatch directly in Immediate mode and returns without
incrementing spriteQueueCount; End only clears the pair after Flush succeeds.
That source evidence motivates the two error-path assertions; no new genuine-XNA
GPU execution is claimed. The new five-case file is not yet native-registered,
compiled or run, and is separate from the frozen original16.

Prepared `build-probe/repair-sprite-binding-compiled-enabled.mjs` passes its
syntax/transformation-only `--check-plan` on the host; it is **UNRUN**. A sandbox
child-Node EPERM was not treated as a syntax failure or a runtime result. The
driver requires78015's accepted13-case gate and unchanged inputs (except the
reviewed FNA3D identifier fix), then enables actual SOFTWARE/EasyGL compiled
effects in the existing external tree and runs21 cases on each renderer. Only
SDL_GPU's eight compiled cases may SKIP. This does not make any regression or
sanitizer acceptance claim. Register the five unchanged witnesses natively after
78015 terminates before starting this driver. Do not alter the original16.

Dependency safety preflight: shared FNA3D HEAD32401479a3ab5bd6b2e7f786e87bf4166aa03b0f,
MojoShader6333f74dbd5644789a63e903816441b16c1e8b60. Both have existing patch changes;
do not erase them. CNA's MojoShader helper has a destructive checkout fallback.
The current133-patch series hash is
`a6a5be0734d6fd0b320f17746a393126116ee30ede89296dd9c794ff8e90bcb1` and tracked diff hash
`3e62f014f5564c9b5bf40780fed78f754b0b9ed3e8d0f67952c565ec45a5d568`; both exactly match
the helper's stored no-op stamp. The final FNA3D patch reverse-check also passes.
The prepared driver rechecks this before configure and refuses a changed stamp,
then freezes the shared sources. It must never invoke the destructive fallback.

Supplementary baseline collected after continuation (CNA `3dcf72f35`, demos
`f42b305`, inherited011/082 work unchanged):

- `SpriteTextureBindingLifetimeTests.cpp`: seven cases per renderer, each of
  SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU gives six FAIL and one PASS. The existing
  destroyed-wrapper retained-renderer control passes; stock-effect pass ordering,
  explicitly disposed live wrappers, move construction/assignment, Immediate slot
  publication and independent copied-wrapper identity fail. Strict C++23
  `-Wall -Wextra -Werror` compilation2153ms; runs73/414/382/934ms.
  `repair-sprite-texture-lifetime-before-audit.json` records unchanged frozen
  inputs, peak427597824 bytes and zero memory events. These are defect
  reproductions, not a passing product checkpoint.
- `SpriteCompiledTextureBindingTests.cpp`: the first compiler attempt correctly
  refused an ambiguous GTest-macro `else` under `-Werror`; its build log remains.
  Adding braces only allowed v2 to compile in2707ms. All three cases SKIP on
  all four renderers because their current CMake configurations disable compiled
  effects (`CNA_{SOFTWARE,EASYGL,SDL_GPU}_COMPILED_EFFECTS=OFF`). Runs76/294/248/568ms;
  peak445546496 bytes, zero memory events, unchanged frozen inputs. The v2 audit
  correctly says `reproduced:false`; this is **no compiled-effect runtime proof**.
  The previous tool handle28520 is now absent and its systemd scope is
  inactive/dead, Result=success; no job was restarted. Child exit0 here means
  skipped tests, not successful behavior. Reports are in `build/linux-full/reports/`
  under `repair-sprite-compiled-binding-before[-v2]-*`.

The original six-case source remains unchanged. No003 production repair, marker
change or closure has been made. Source inspection still requires binding after
each custom pass, including renderer-owned compiled passes, not merely before
renderer Draw or after End. Existing configurations cannot validate the compiled
route; obtain an enabled configuration or clearly retain that validation gap.

**Confirmed by actual baseline61003 after156 commit7097e1684:** strict compile
2210ms, six original tests on each SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU. Every
renderer gives the same fiveFAIL/onePASS: deferred/immediate publication, both
sorted final bindings and bound SetData refusal fail; empty-batch preservation
passes. Runs78/387/338/909ms, frozen inputs unchanged, peak415399936bytes and all
memory events0. The observation driver exits0 because it captured the expected
reproduction; the test processes each exit1 and all failures remain in XML/audit.
No003 production change or closure. All previous prepared/unrun wording below
describes the state before this baseline, not its current validation status.

Read-only source audit at CNA `2af199aad4d11053a7549bee01f9efff272cd9e0`,
with the separately documented uncommitted011/082/156 candidates present.
No production edit, new runtime pass or issue closure is claimed here.
`SpriteTextureBindingTests.cpp` contains six prepared, **uncompiled/unrun**
public-state witnesses. Run them unchanged after the current native98308 and
the prepared132-case external156 comparison finish; no overlapping heavy job.
`build-probe/repair-sprite-texture-binding-before.mjs` is syntax/transformation
checked, **UNRUN**. It reuses the actual native consumer compile flags and links
the current external library, then records all six original outcomes on each of
SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU. It requires the132-case observation audit to
be complete and sources/artifacts unchanged; it does not treat that audit's
separate C callback failures as accepted regressions.

The historical inventory's null binding is still explained by current source:

- `SpriteBatch::pushSprite` retains only `shared_ptr<ITextureRenderer>` in each
  queued `SpriteInfo`, not a safely tracked public `Texture2D` identity.
- `flushSingle` sends the retained renderer to its sprite renderer, without
  assigning `GraphicsDevice::getTexturesProperty()`.
- `drawThroughStockEffect` similarly overrides `GpuDrawParams::texture0` after
  applying each effect pass, but does not publish the corresponding public slot.
- `TextureCollection::operator()` already provides disposed/device/render-target
  validation. `Texture::ThrowIfDataTransferResourceInUseEXT` relies on the public
  slot; silently using only the renderer bypasses that observable binding.

## Genuine XNA source evidence

The existing pinned local XNA assembly decompilation is:
`../xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework.Graphics/`
`Microsoft.Xna.Framework.Graphics/SpriteBatch.cs`.
Its `RenderBatch` assigns `_parent.Textures[0] = texture` immediately before
`PlatformRenderBatch`, once per custom-effect pass and after that pass's Apply;
the default-effect branch also assigns it. `SortSprites` orders the queue before
rendering, so the final slot follows rendered order, not original submission order.
This is source evidence, **not a new genuine-XNA GPU runtime execution**.

The six prepared witnesses cover deferred publication timing, immediate per-draw
publication, empty-batch preservation, both depth-sort directions with distinct
depths, and refusal/restoration of SetData around explicit unbinding.
They do not yet establish custom-effect pass ordering or wrapper lifetime safety.

## Required lifetime audit before implementing

Do not add an unguarded `Texture2D*` to the queue. Current SpriteBatch explicitly
retains renderer ownership so destroying a temporary before End is safe; tests
around `SpriteBatchTests.cpp`'s retained-texture renderer seam protect that route.
Current End also documents a deliberate disposal-before-End difference from XNA.
The inherited011 shared content identity changes must remain isolated.

A repair must preserve safe destruction/move handling, distinguish an explicitly
disposed live wrapper from one whose C++ lifetime ended, and avoid leaving a
dangling public slot. Inspect the current texture detach hooks, resource identity
and tests before selecting the smallest correct representation. Add focused
lifetime/sanitizer and custom-effect-order controls alongside the public-state
regression. Do not claim those controls are covered by the six prepared cases.
