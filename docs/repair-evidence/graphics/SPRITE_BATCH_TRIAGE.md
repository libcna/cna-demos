# SpriteBatch remaining findings — read-only triage

## Latest evidence (2026-10-21)

Broader132-case comparison70430 is complete, exit2:123PASS/8FAIL/1sameSKIP.
All four begin_end_rules cases retain the original accounting failure and gain
the callback-before-disposed failure; all four C resource_lifecycle cases fail
their existing callback-before-disposed assertion. Those eight082 deltas remain
unaccepted, with no assertion/marker change. The only other delta is156's SDL_GPU
depth improvement, independently committed7097e1684 after full native11005 exact
parity and focused pixel/bias/visual gates. No082 product commit or closure.
003 now reproduces fiveFAIL/onePASS on each of four renderers in61003; its separate
evidence is SPRITE_TEXTURE_BINDING_TRIAGE.md. Neither next batch settles082 policy.

Event sanitizer83569 accepted all6 original lifetime cases, no findings. Original
external17557:16PASS on four renderers. After the rebuilt event+156 candidates,
observation72013 reports12 C++PASS and four C resource_lifecycle FAIL, each only
"explicit disposal synchronously raises one callback before the disposed state
changes" (1/30 child checks). No new knownIssue marker, skip or oracle edit.
This is NOT accepted external regression. Public C timing choice and separate
SpriteBatch post-disposal guard choice remain pending; do not commit/close082
as though those contracts were settled. Independent156 focused20/bias72/exact
four visuals are accepted; sole heavy98308 now checks its native11005 parity.

Native-v2/88430 ACCEPTED:10899PASS/101unchangedSKIP1508686ms, exact10994 old
outcomes plus6 eventPASS, no frozen drift/OOM. Audio stress sanitizer75848 accepted
both original cases with ASan/UBSan/float-cast/leaks, no findings; test-only repair
committed CNA2af199aad4d11053a7549bee01f9efff272cd9e0. Original failed30524 retained.
Sole heavy83569 is now original6 event sanitizer. No graphics candidate commit,
C contract/oracle correction or082 closure yet. After acceptance preserve original
external16, then execute prepared156 before20 against still-old external004 library.

Original30524 is now TERMINAL1:10898PASS/101unchangedSKIP/one audio coverageFAIL,
1442390ms, frozenunchanged/peak4297887744/maxOOM0. Only the recorded stress assertion
regressed; original6 added event cases PASS. After source-backed test-only repair,
sole heavy88430/native-v2 has buildPASS11796ms,20/20 stressPASS804ms,audio771PASS/
4unchangedSKIP9070ms,focus6PASS218ms,graphics2912PASS/73SKIP41221ms. Full11000 LIVE.
After full acceptance run newly prepared audio stress sanitizer2 FIRST, then
event sanitizer6 (now requires that audio audit), then original external16 capture
(now requires native-v2, never failed30524). Prepared drivers are syntax checked,
UNRUN. Private-display check uses private Wayland/runtime identity, not a display
number: the isolated Xwayland may legitimately be named:0. Original failure and
all assertions retained; no new issue closure, no audio production change.

Native30524 is still live, but the full log has a real failed coverage assertion
in the unchanged dynamic-audio alternating-format stress test (`callsThrown==0`).
Do not accept this run or start the prepared sanitizer as though it passed.
`../audio/DYNAMIC_STRESS_SCHEDULING.md` records the exact original failure,
previous passes and source-backed missing rendezvous. The proposed test-only
repair preserves free-running stress and adds deterministic live-format coverage.
Prepared `repair-resource-event-native-v2.mjs` is syntax/plan checked, UNRUN:
it requires original30524's terminal failed audit with exactly that one failure,
permits only the audio test source to change, requires all20 fixed scheduling
iterations plus the audio module, original6 event cases, graphics2985 and exact
full11000 parity. It preserves the original failed logs/audit and forbids extra
production changes. No native test source has been changed yet.

Prepared `repair-resource-event-external-before.mjs` is syntax/plan checked,
UNRUN. After native and sanitizer acceptance it verifies all319 canonical external
artifacts still match the004 final checkpoint, then captures16 additional original
resource-lifecycle/C observations on SW/GLES/GL33/SDL_GPU without a rebuild or
oracle change. Its frozen-key inventory was checked (319 present, none missing).
This preserves old C contract evidence before the external library is replaced.
30524 has advanced past the genuine XNA differential build test and is now in
XnaErrorParityTest; original process still live, no full acceptance yet.

Continuation confirms native30524 still live in its original process/scope,
running the long genuine XNA differential corpus. No restart or concurrent build.
Prepared `SpriteDepthProjectionTests.cpp` is UNCOMPILED/UNRUN and unregistered:
five ISSUE-156 witnesses for Immediate depth order in both submission orders,
FrontToBack depth versus no-depth control, numeric depth agreement with ordinary
BasicEffect geometry (four near/far comparisons), and Z translation/scale.
HiDef and an explicit Depth24Stencil8 buffer avoid the earlier readback setup
pitfall. APIs mirror existing native depth tests. Future baseline must run on
SDL_GPU and SW/GLES/GL33 before modifying projection code; preparation is not
evidence of runtime coverage. This file is outside082's candidate and frozen build.

30524 candidate native build55531ms/focused6PASS350ms/graphics2912PASS+73SKIP
54259ms accepted; full11000 remains LIVE, not accepted yet. Prepared actual
SOFTWARE sanitizer6 driver is transform/syntax checked, UNRUN. The SDL_GPU depth
residual is now permanent ISSUE-156: original155+one new issue,104final/52pending.
C audit also found an explicit contract conflict: GRAPHICS_RESOURCES.md promises
callback before disposed state changes, GraphicsResourceSmoke.c checks false in
both declaration and texture callbacks, and the external C resource_lifecycle
demo asserts false too. Separate owner clarification requested; no C doc/test or
demo expectation changed. Snapshot these old external cases BEFORE rebuilding
external CNA; original callback/resource lifecycle baselines are still executable.

Native event baseline65198 completed0: original six-case witness on each actual
SOFTWARE/OPENGLES3/OPENGL33 gives five callback/retirement FAIL and finalizer PASS.
Strict fixture build6750ms; runs191/446/401ms; frozen unchanged, peak3603423232,
all memory events0. Candidate now edits only non-shared GraphicsResource::Dispose:
disposed flag/diagnostic retirement before callbacks, exception-safe device
unregistration and original exception rethrow. Original six witnesses registered
byte-for-byte.30524 native focused6/graphics2985/full10994+6 exact gate is LIVE,
not yet accepted. No sanitizer/external pass or event fix commit is claimed.
Owner question is pending about SpriteBatch's stricter post-disposal guards;
it does not block this independent base-resource callback repair. Keep082's
marker/expectation intact until its complete contract disposition is established.

004 is now closed: CNA product9359a26b6/status7ad1f8470, demos99f4069. All004
native/sanitizer/external/final sessions accepted; see IMMEDIATE_SAMPLER_REPAIR.md.
This document's older004 LIVE entries are historical. Inventory155/104final/51pending.
082 genuine callback reference97591 completed0: all8 managed checks PASS against
the pinned original assemblies/MVID. This includes exact original exception
identity, IsDisposed-before-event, no second callback on reentry or after a throw.
Build6685ms/run1026ms, frozen unchanged, peak807870464/high303/max0/OOM0. No native
XNA GPU teardown claim. Prepared six-case native fixture now records the actual
renderer for its two attached-device cases. `repair-resource-event-before.mjs`
is syntax/plan checked and runs those original six cases against the pinned
current external CNA on SW/GLES/GL33 before any event repair. Expected five
callback/retirement failures and one finalizer control PASS; do not infer that
outcome until the actual audit completes. Inherited011 and demo assertions intact.

New renderer residual observed during004 external86378's **pre-rebuild** SDL_GPU
baseline: `graphics.sprite_batch.layer_depth_sorting` fails its existing
"enabled depth testing makes nearer layerDepth win over painter order" assertion.
All other assertions in that demo pass; no knownIssue exists for it. Original
SW/GLES/GL33 baseline passes it. Preserve the raw before report/log and investigate
after the current sampler batch; do not weaken the depth oracle, classify this as
a sampler regression, or introduce a skip. Its root cause and final disposition
remain unverified. Actual SDL_GPU sampler-before image was inspected and both
sprites are point-filtered; its passing blend sibling image was also inspected.
The external exact comparator requires the pre-existing depth failure to stay
visible while measuring004's isolated change. This residual needs its own follow-up.

Continuation verified native42945 still live via its session, CnaTests process and
active systemd scope; full tests advanced to AvatarServiceTest after passing the
long XNA corpus. No second heavy job was started. Prepared and syntax/plan checked
`repair-sprite-sampler-final.mjs` (UNRUN) requires accepted native, sanitizer,
external116 and actual four-renderer image review, then validates the exact
one-line004 marker removal, original Immediate siblings on four renderers and
five focused/catalog CTests. This is not a marker removal or a passing result.

Native75809/v3 accepted three actual filter/address pixel failures and one Deferred
control PASS, all on OPENGLES3 HiDef (build3069ms/run383ms/inspect31ms,
peak609665024, all memory events0, frozen unchanged). Candidate18 lines in common
SpriteBatch::flushSingle refresh the private renderer channel from the live slot
only for Immediate, carrying all seven properties through the existing hook.
No Begin device-state reapplication, renderer-specific change or Deferred change.
Original four witness tests registered byte-for-byte.42945 now validates native
focused4/graphics2979/full10990+4 under private GPU/network/caps0/one worker/6GiB;
still no accepted repair, marker removal or commit for004. The original87 external
baseline and all original assertions remain available for exact after comparison.
42945 buildPASS42759ms/focus4PASS331ms/graphics2906PASS+73SKIP55342ms;
full10994 is now LIVE, not accepted yet. No new compiler warning diagnostics.
The full run has since passed genuine XNA differential763587ms and error parity
130170ms. Later tests remain live. Prepared `repair-sprite-sampler-sanitizer.mjs`
requires full acceptance, reuses the existing SOFTWARE build-asan closure, verifies
actual SpriteBatch/Texture2D/GraphicsResource and Runtime instrumentation, and runs
the original four pixel cases with ASan/UBSan/float-cast/leaks. Syntax checked,
UNRUN, no suppressions. Run it serially before the external gate, which now requires
its accepted audit; no untested renderer or fully instrumented vendor claim.
Prepared `repair-sprite-sampler-external.mjs` is syntax/plan checked only: after
native acceptance it captures29 original SDL_GPU observations against the old
external build, builds once, compares116 exact before/after results, and runs the
four unchanged native pixel witnesses on SOFTWARE/OPENGLES3/OPENGL33. SDL_GPU
coverage is from real demo execution, not falsely claimed native tests whose
existing rasterizer gate excludes it. Actual repaired images still need inspection.

Prepared `GraphicsResourceEventOrderTests.cpp` is UNCOMPILED/UNRUN and deliberately
not registered during42945. Non-shared base-resource checks cover published state,
original sender, bounded reentrant Dispose, a throwing callback, and finalizer
behavior. Genuine13 confirms the normal event-order premise; throwing/reentrant
reference execution still needs its own evidence before changing that layer.
No inherited011 code or original lifecycle expectation changed. These follow-up
tests are separate from004's commit and must not be mistaken for executed coverage.
`GraphicsResourceEventOrderOracle.cs` is a separate prepared, UNCOMPILED/UNRUN
eight-check genuine managed callback probe, preserving the original13 accounting
oracle byte-for-byte. Run only after the native differential corpus releases Wine.

Further source review found a lifetime constraint on the eventual event repair:
non-shared GraphicsResource::~GraphicsResource calls Dispose(false), which returns
immediately when isDisposed_ is true. Device registration currently retires AFTER
Disposing and ResourceDestroyed callbacks. Merely moving the flag before callbacks
would therefore leave a dangling registered pointer if either callback throws;
the destructor would no longer retire it. A correct local repair needs exception-
safe registration retirement, preserving normal callback ordering and the original
exception, with attached-device and sanitizer regressions. Do not apply a one-line
flag move based only on the constructor-free tests. Inherited011 metadata hunks
are separate from this old non-shared Dispose body and must remain unstaged when
committing a future local event-order hunk. No event implementation changed yet.
The prepared native event fixture now includes two attached-device checks in
addition to its four callback/finalizer checks. These assert registration retires
even if Disposing or ResourceDestroyed throws, then destroy the wrapper/device;
the second callback throw is bounded so the old destructor path can be diagnosed
without terminating the probe. Added the required concrete GetTypeName override
after checking Object's pure-virtual declaration. Still UNCOMPILED/UNRUN; only the
first four tests are GPU-independent. All six need appropriate lifetime validation.

After055 closure: CNA d2bef7020 (product1ba26ed18), demos a445241; no SpriteBatch
production, original demo assertion or marker changed. Reference91995 completed0:
all13 actual managed XNA checks PASS (strict C# build3504ms, run644ms,
peak751284224, all memory events0). The two assemblies' hashes/MVID and constructor-
free limitation are in repair-sprite-accounting-reference-audit.json/run.log.
This independently confirms the existing SOFTWARE-351 ledger/native regression:
open-batch accounting survives Dispose. Thus082's accounting expectation is wrong.
The probe also exposes separate old demo positive controls that differ from XNA
(disposed-before-event and repeated Begin exception). Do not hide those by simply
inverting one assertion and claiming all lifecycle behavior is XNA-compatible.
GraphicsResource has inherited011 edits; preserve them and isolate any future
event-order repair.082 remains without final disposition pending that contract audit.

Original87-case baseline94606 completed0:29 sprite/sampler/C/texture demos on each
SOFTWARE/OPENGLES3/OPENGL33. Exactly004 and082 fail on all three; one genuine GLES
LOD-bias capability skip, other80PASS. All frozen inputs unchanged. Peak1612640256,
high6889/no max/OOM. Screenshots of both Immediate siblings captured, still awaiting
visual inspection at capture time; all three sampler images were subsequently
inspected and show both sprites point-filtered, as their exact row checks report.
003's SetData demos pass but do not by themselves assert the
missing post-Draw texture binding; no003 closure inferred from them.

Prepared SpriteImmediateSamplerTests.cpp is an unregistered four-case native
pixel witness: live filter change, point/linear/point sequencing, live address
change versus independent wrap control, and unchanged Deferred behavior. Strict
baseline18808 compiled2906ms then selected SOFTWARE in an OPENGLES3-only build;
all four constructor refusals are harness errors, not sampler reproductions.
36001/v2 reused that binary with OPENGLES3 but its default Reach profile refused
backbuffer readback. Preserve both audits/logs. Corrected fixture explicitly
constructs HiDef, as existing rasterization tests do, retaining all behavioral
assertions.75809/v3 now running serially; no real pixel result inferred yet.
Next collect it, then minimally repair004's common private sampler propagation.

2026-10-21, CNA `61baeb155df02ea809374df096789b871fd22e88`, while the
ISSUE-055 full native validation runs. No SpriteBatch source/test/oracle changed.
These are source observations, not current runtime reproduction or final dispositions.

## ISSUE-082 needs reference verification before any repair

The historical demo expects disposing an open deferred batch to free device-level
accounting so another batch can Begin Immediate. Current CNA Dispose clears its
own queue/begun state without decrementing the device counters; End is the only
decrement path. This supports the reported symptom, but does not settle the oracle.

The genuine Windows XNA decompilation in sibling `xna4-decomp/reference/xna4/
decompiled/windows/Microsoft.Xna.Framework.Graphics/Microsoft.Xna.Framework.Graphics/`
shows a potential contradiction in that expectation:

- `SpriteBatch.cs` Begin increments the device counters and sets inBeginEndPair.
- Its Dispose(bool) disposes spriteEffect and platform buffers and delegates to
  GraphicsResource; it does not clear the pair or decrement either counter.
- `GraphicsResource.cs` disposal publishes isDisposed and the event, with no
  SpriteBatch accounting operation.
- The next Immediate Begin checks the same device count before native state setup.

Therefore do NOT simply add a decrement to make the demo green. Next verify the
actual shipped XNA methods (a narrowly disclosed uninitialized-object managed
state seam can test the counter/early-refusal path without claiming GPU runtime).
Inspect the intended CNA disposal extension and existing guards separately.
If XNA confirms the retained counter, classify/correct the demo expectation with
evidence and history, or explicitly resolve any documented intentional divergence;
do not silently impose a more convenient non-XNA behavior.

Prepared `SpriteBatchDisposeAccountingOracle.cs` and
`build-probe/repair-sprite-accounting-reference.mjs` are UNCOMPILED/UNRUN. The
driver is JavaScript-syntax checked only. It pins the existing genuine Framework
and Graphics assemblies and proposes13 managed checks using constructor-free
objects: real public deferred Begin, Dispose/event ordering, retained counters,
the next Immediate Begin's early refusal, and the still-allowed deferred control.
There is deliberately no native GPU, buffer/shader destruction, draw or End claim.
The decompilation also suggests IsDisposed is published before Disposing, and a
disposed open batch's Begin hits the pair check rather than a disposal guard;
measure these controls rather than assuming the demo's positive controls are XNA.
Do not run this shared Wine-prefix probe beside the active native XNA differential
tests. Run serially after055's gates under the existing1GiB/private-display policy.

## ISSUE-003: texture binding also intersects C++ wrapper lifetime

Genuine XNA SpriteBatch.RenderBatch assigns device Textures[0] after each custom
effect pass, and before its default draw. Current CNA flushSingle passes only the
texture renderer through GpuDrawParams; drawThroughStockEffect likewise assigns
the internal draw parameter without publishing the public collection slot.
The queued SpriteInfo deliberately retains a shared renderer after a temporary
Texture2D wrapper dies. A naive raw wrapper pointer in the queue would regress
that supported lifetime. Review TextureCollection ownership and inherited011
changes before designing a minimal repair. Preserve sorted-last-texture order,
custom-pass ordering, Immediate timing, empty-batch behavior and SetData refusal.
No current reproduction or implementation has been performed in this triage.

## ISSUE-004: Immediate sampler channel remains a candidate common-layer defect

Current Begin/applyRenderState sends all sampler properties to the sprite
renderer. flushSingle does not refresh that private channel from current device
SamplerStates[0]. The stock-effect path does call applySamplerStatesToRenderer;
the ordinary sprite path does not. Existing native tests cover Begin publication,
deferred mutation timing and fallback normalization, not necessarily inter-Draw
Immediate mutation. Reproduce the unchanged pixel demo and preserve its passing
blend-state sibling before changing code. Use the reference state boundaries;
do not reapply Begin's sampler over an intentional caller update.

Finish ISSUE-055's native/sanitizer/external/C gates first. This document is only
preparation; none of003/004/082 is closed, suppressed, or newly marked reproduced.
