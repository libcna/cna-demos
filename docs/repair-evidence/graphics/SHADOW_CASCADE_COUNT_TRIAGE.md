# ISSUE-102 — shadow cascade count boundary

## Current validation (supersedes preparation notes below)

Resolved by CNA `ce88b5f9bf4a6771d4650d182c014944837452a0`. The commit contains
only the four guards, six contract headers,20 native witnesses and campaign
handoff. Inherited content/video work was excluded. No push/merge or assertion
weakening. No knownIssue marker existed for this issue. All older running and
preparation states below are historical; the completed evidence supersedes them.

All required gates completed on2026-10-21. External build458,110ms with zero
diagnostics; strict witness build2,933ms. All20 tests PASS on each actual
SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU(Vulkan)/HEADLESS/STUB renderer (identity
recorded and asserted per test).112 original audio/video scenarios have exact
pre-shadow outcomes, check counts, markers, failing assertions and C labels.
The same C VideoPlayer failures and pure-C smoke exit2 remain globally
unaccepted; parity is not conversion of those failures or video closure.
External peak4,302,151,680 bytes/high1113/max0/OOM0; source hashes unchanged.

Stock66 AFTER exactly matches BEFORE52PASS/12rendererSKIP/2known070FAIL;
peak193,142,784/events0. Four original visual runs PASS (16cascade+6PBR checks
per GL profile), each800x480 capture has zero differing pixels against its
same-renderer baseline. Runs500/450/481/484ms,peak197,160,960/events0. The two
OPENGLES3 after-images were inspected; corresponding OPENGL33 files are
byte-identical. No approved visual reference or SOFTWARE shadow-image support
is claimed. No oracle assertion/marker changed. Product commit follows this
validation; preserve unrelated011 and video candidates outside that commit.

Sanitizer89823 completed: all20 boundary witnesses plus7 existing shadow
receiver tests PASS with actual four effect implementations and Sharp Runtime
instrumented by ASan/UBSan/float-cast/leak detection. No findings/suppressions;
third-party libraries are not claimed instrumented. Libraries30,047ms, strict
fixture6,386ms, run136ms, XML audit19ms; frozen hashes unchanged,
peak851,980,288 bytes/events0. External45102 is now running the existing
linux-full incremental build, one worker/6GiB/no swap, before six actual-renderer
fixture runs and112 exact outcome comparisons. The first external wrapper
refused an ambiguous C++/C compiler-flag replacement before building; its fix
targets only the C++ command and leaves strict C17 flags unchanged.

Stock66 BEFORE completed: SOFTWARE20PASS/12SKIP/1known070FAIL; OPENGLES3
32PASS/1known070FAIL. Exact source/artifact hashes unchanged,peak191,201,280
bytes/events0. First driver stopped before running tests because its absolute
artifact lookup missed a relative key in the prior audit. Canonicalizing paths
with duplicate-hash conflict rejection fixes that harness error; actual old
binary hashes match the prior video and visual audits. No assertion changed.
Sanitizer27 now running in existing build-asan, one worker/6GiB/no swap.

Native-v2 completed successfully: focused20PASS498ms, graphics2890PASS/73SKIP
37,458ms, full10867PASS/101sameSKIP1,259,563ms. Exact previous10948 outcomes
unchanged plus20newPASS; comparison177ms. No frozen source changes;
peak1,357,451,264 bytes, all memory events0. Stock66 before, sanitizer27,
external120/112 parity, stock66 after and four exact image comparisons remain
pending. No final disposition or product commit yet.

15:30 CEST: native-v2 job45760 now passes all20 original boundary witnesses
(498ms), after incremental build460ms. Full graphics module and full10968 parity
are running/pending. First native build93255 succeeded364819ms, then the gate
stopped before tests to review five existing nodiscard warnings in EXPECT_THROW
calls in unchanged content tests (ModelContentTypeReaderTests2, CnjModelTests1,
CnjStockEffectTests2). No content source or native warning flags changed. V2
pins every first-build source hash and explicitly records reused owned artifacts.
The first log is preserved; this is not a retry or suppression of a failed test.

Safe baseline93365 completed:20 cases,12PASS and8 boundaryFAIL across all four
effect types. Strict build3873ms, run679ms, audit30ms; frozen hashes unchanged,
peak473071616 bytes, all memory events0. No invalid count reached FillGpuDrawParams.
The first standalone compile failed on existing renderer virtual fallback unused
parameters under an added Wextra policy. Native ShadowReceiverTests enables
neither Wall nor Wextra; the corrected probe keeps Wall/Werror without that
extra policy. Original failed log and unchanged witness are retained. No
assertion suppression, source workaround or actual memory-corruption run.

Original visual baseline45372 also passed: cascades16 and PBR6 checks on each
OPENGLES3/OPENGL33. Runs683/590/600/582ms,peak194887680/events0,unchanged hashes.
The two OPENGLES3 screenshots were visually inspected; corresponding OPENGL33
screenshots have identical SHA256 bytes. Authored near/far/split/blend panels,
four cascade tints and rigid/skinned lit-shadow pairs remain visible. These are
before-repair captures, not approved references.

Minimal candidate now changes only the four setter guards (before assignment),
their documented0..kMaxCascades refusal contract and the same20 native witnesses.
No class layout/API signature/backend/array capacity change. Native job93255
is RUNNING: existing debug tree, one compiler worker/shared cache, private GPU
and network/capabilities dropped,4GiB high/6GiB hard/no swap. Source hashes frozen.
Next native graphics/full parity, sanitizer27, actual external renderer/public
demo regressions and exact before/after visual comparison. No closure/commit yet.

2026-10-21, source inspection against CNA c04f80d65 plus the recorded unrelated
uncommitted011/099/100/101 candidates. No shadow production source changed.
Native video full regression59093 and sanitizer97309 have passed. External
video gate68581 is still running; no overlapping compiler/test job was launched.
`ShadowCascadeCountTests.cpp` is **UNCOMPILED / UNRUN**.

The historical source diagnosis remains present:

- ShadowCascadeStateEXT.hpp declares Count as int, kMaxCascades4 and four-element
  WorldToAtlas/SplitDistance arrays. Count0 means the single-map fallback.
- BasicEffect.cpp523, SkinnedEffect.cpp642, PbrEffect.cpp724 and
  SkinnedPbrEffect.cpp779 assign the public state without a count guard.
- Their FillGpuDrawParams loops at268/371/483/540 index up to that stored count.
  IGraphicsRenderer.hpp1291/1293 provides64 matrix floats and four split floats.
  Validation after this copy in a renderer cannot protect these accesses.
- Existing ShadowReceiverTests.cpp180 lines cover ordinary shadow properties and
  default/enabled single-map draw parameters, but no cascade-count boundary.

This is a **CNA extension**, not an XNA contract inference. The proposed narrow
repair is a pre-assignment ArgumentOutOfRangeException for Count outside0..4,
preserving every field of the prior state, consistent across all four receivers.
No renderer-specific workaround, array growth, API redesign, split/matrix/BlendBand
validation or change to valid counts is proposed. The existing issue explicitly
recommends refusal before mutation. Public receiver/setter comments must state it.

The prepared typed fixture has20 cases: five per effect family. It covers all
valid counts0..4, negative -1/INT_MIN, oversized5/INT_MAX, assigning the receiver's
own valid state, and four-slot propagation plus Count0 fallback. Invalid setter
tests never call FillGpuDrawParams or draw with an accepted-invalid count; thus
the failing baseline can demonstrate the boundary without invoking known UB.
It also checks all source fields survive a refusal, not just Count. Expected
baseline12PASS/8FAIL is a hypothesis until the binary actually runs.

Existing public visual oracle: graphics.skinned_effect.shadow_cascades uses
Count0/2/4 and all four atlas slots/debug tints. Its own supported scope is EasyGL
OPENGLES3/OPENGL33; SOFTWARE deliberately skips it. Do not report a SOFTWARE
visual pass. graphics.pbr_effect.shadow_receivers supplies a separate public
shadow sampling regression. Preserve their source/expectations and compare exact
pixels on applicable available renderers after repair. Native setter/draw-parameter
checks and sanitizers can cover SOFTWARE without claiming shadow-image support.

Next after the active video gates: compile/run the safe fixture against the
current implementation, preserve exact failures; then make the small receiver
boundary repair with native regression coverage, run existing graphics tests,
instrumented fixed-capacity copy checks and applicable public visual demos.
No final disposition, knownIssue change, production commit or hardware claim yet.

Prepared ignored drivers, all unrun: repair-shadow-count-before.mjs (safe20
case reproduction), repair-shadow-native.mjs (original20, full graphics module,
exact full native parity), repair-shadow-sanitizer.mjs (original20 plus7 existing
ShadowReceiverTest cases, actual four effect implementations instrumented), and
repair-shadow-visual.mjs before/after (four original public demo executions on
OPENGLES3/OPENGL33, exact same-renderer image comparison). No approved image
reference exists for these demos; do not label the planned before/after comparison
as an approved-reference pass. None of these tools creates a build directory.

Read-only follow-up search finds no shadow-cascade setter route in modules/c-api;
do not invent a C API extension as part of this boundary repair. Existing C
graphics regression scenarios will still be run against the underlying repair.
