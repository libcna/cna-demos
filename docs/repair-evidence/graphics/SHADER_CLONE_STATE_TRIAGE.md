# ISSUE-055 — ShaderEffect clone state repair and evidence

## Final disposition — FIXED_VALIDATED

Product commit `1ba26ed18b520b3469a8250a8a6fe032e664c84e` contains only the
ShaderEffect implementation/header, four native regressions and the strict-C
EffectSmoke extension. Inherited011 changes and C derived metadata were not staged.
Final gate15342 accepted0: original9 checks PASS on each OPENGLES3/OPENGL33,
SOFTWARE honest0-check SKIP77, three catalog CTests PASS/one declared shader SKIP.
All other catalog entries and frozen inputs unchanged; only055 knownIssue removed.
Peak2335895552, all memory events0; no push/merge. Earlier status notes below are
historical. The global full-campaign sweep remains future work.

Final61523 built successfully0/7961ms without compiler diagnostics, then its
verifier incorrectly treated a relative alias of the same rebuilt executable as
an unrelated artifact. Direct hash review found only the expected demo source
and that executable (two path spellings) changed. The corrected verifier resolves
paths, pins the reviewed executable SHA256
`e536f4c4ee4e0d5d82db2b215b3d5cae314e0a4dc8c897a82748d37c075c6ed8`,
and resumed without rebuilding. Original failure/build log retained. Resume GL
104ms skip/458ms GLES/424ms GL33, CTests23190ms, inspect24ms; no assertion weakened.

## Current checkpoint — repair validated, final marker gate running

The chronology below is retained as historical evidence; its older LIVE/UNRUN
statements are superseded by this checkpoint. External79676 accepted:39 paired
cases, only the exact three055 failures become PASS on OPENGLES3 and OPENGL33.
The same strict C binary proves the old clone-World failure and passes against
the repaired library on SOFTWARE/OPENGLES3/OPENGL33. Original structural4 pass on
SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU/HEADLESS/STUB (no shader-execution inference).
Build266991ms; frozen unchanged; peak4301348864, high7545, no max/OOM events.

All four actual GL before/after screenshots were inspected. Before cyan area24000
pixels, bounding box[500,121,699,359]; after2160,[520,187,579,257]. No pixels change
outside the triangle union; both final profile images are exactly RGBA-identical.
`repair-shader-clone-visual-review.json` records hashes and observations. The
original nine demo checks separately prove first-frame viewport equality and
survival after original disposal; a final screenshot alone does not prove those.

Inventory96218 failed because the derived inventory was stale; no runtime failure.
The original generator --write refreshed it, then inventory74105/v2 accepted all
seven original gates. Full inventory diff is exactly six ShaderEffect source line
numbers, with no changed symbols/classifications/approvals. Totals remain467headers,
8122symbols,7011implemented,15partial,643planned,453not-applicable; release NOT READY
remains explicit. Inherited011 mappings/derived docs remain outside055's commit.
Peak1075458048, high18864, no max/OOM. Original failed logs remain preserved.

Final before97106 accepted the frozen inputs and catalog. Exactly one knownIssue
line removed; final61523 running the unchanged GL9 twice, honest SOFTWARE skip,
incremental one-worker build and four focused/catalog CTests. No closure yet.

Sanitizer43739 accepted5PASS, actual SOFTWARE implementation instrumentation,
no ASan/UBSan/float-cast/leak findings or suppressions. Libraries12,713ms, strict
fixture compile6,128ms, run105ms, inspect25ms;frozen unchanged,peak833,327,104/events0.
External79676 is LIVE. Its extended pure-C EffectSmoke compiled284ms and failed
exactly the new clone-World retention assertion against the old external library
(63ms/exit1/stage3), with no earlier failure. All39 pre-repair shader/C captures
complete; only clone_state fails on both GL profiles, expected SOFTWARE shader
capability skips retained. Incremental build now underway. No055 closure yet.

Native73916 completed0: focused4PASS440ms, graphics2,902PASS/73SKIP59,418ms,
full10,889PASS/101sameSKIP1,688,536ms. Exact comparison135ms: all previous10,986
identities/outcomes preserved, only the four expected newPASS cases added.
Frozen unchanged;peak2,705,068,032/all memory events0. After this completed gate,
the existing pure-C EffectSmoke was extended only with exact World/View/Projection
clone retention and independent World mutation. This separate C test is UNRUN;
no production code changed after native validation. Sanitizer43739 is now LIVE.
The external driver will first require the exact new clone-World assertion to
fail against the old external C library, then use that same C binary with the
rebuilt library on SOFTWARE/OPENGLES3/OPENGL33. It retains all original C assertions.

Continuation: native60373 terminated after successful146,150ms build, before any
test. Its diagnostic check found three existing unused-result warnings in
EXPECT_THROW expressions (CnjEffectTests.cpp185; CnjStockEffectTests.cpp295/332).
Both files equal HEAD exactly. The discarded nodiscard value is intentional in
exception assertions; no ShaderEffect source diagnostic exists. Vendored Draco
also reports its existing CMP0148 developer warning. No warning flags, vendor,
product source or assertion was changed. Do not call this a warning-free build.
Exact diagnostics and completed artifacts are pinned by
`repair-shader-clone-diagnostics-review.json`; its571-entry input snapshot is
checked except for the explicit rebuilt targets, whose new hashes are recorded.
Initial snapshot attempt correctly refused changed build outputs before writing
anything; the review explicitly updates only the three targets just rebuilt.
Resume73916 uses these artifacts without another build, original focused4 now
PASS440ms. Graphics2975/full10986+4 outcome comparison still pending. Original
60373 files remain untouched; private GPU/network/zero caps/one heavy job/6GiB
limits retained. No closure until native, sanitizer, real GL and C gates finish.

Resume73916 graphics module completed: 2,975 cases = 2,902 PASS / 73 SKIP,
zero failures/errors, 59,418ms driver wall time. Full native comparison is LIVE;
module success alone is not repair acceptance. Prepared (syntax checked, UNRUN)
`build-probe/repair-shader-clone-sanitizer.mjs` requires the accepted full audit,
instruments actual ShaderEffect/Effect/GraphicsResource/runtime and runs original4
plus the existing structural ShaderEffect test on SOFTWARE. It makes no GPU
shader-execution claim. `repair-shader-clone-external.mjs` requires native+SAN,
captures39 paired shader/C demo cases on SOFTWARE/OPENGLES3/OPENGL33 around one
incremental external build, permitting only055's exact three GL failures to pass.
It also checks original4 structural cases on six actual renderers and the original
strict-C17 EffectSmoke on three. GL screenshots still need human/model inspection;
C metadata gates and marker-only closure remain separate work. No script syntax
check is counted as test execution.
The prepared `repair-shader-clone-inventory.mjs` is likewise syntax-checked/UNRUN;
after accepted external39 it runs seven original C gates under2GiB/no swap.
No C declaration or approval changes are inferred from Clone's documentation.
Inherited011 mapping/derived summaries remain outside the055 commit boundary.
Prepared `repair-shader-clone-final.mjs` is syntax-checked/UNRUN. Its before phase
requires accepted external39, the seven C gates, and an explicit hash-pinned review
of both actual GL screenshots. Its after phase allows exactly one knownIssue-line
removal, checks every other source/artifact/catalog entry unchanged, repeats GL9
on both profiles and the honest SOFTWARE skip, and runs the focused/catalog CTests.
No marker is removed yet. This does not replace the eventual full campaign sweep.

After video closure: CNA61baeb155 (productabbd8a4ea), demosb325b6a6120aded1f7962e1d31e41fc0a99a29f6.
Baseline45813 `repair-shader-clone-before-*` reproduces exactly3FAIL/1PASS in
the four unchanged structural tests (strict compile3,755ms/run335ms/audit23ms).
Original clone_state demo executes9 checks on each OPENGLES3/OPENGL33, with the
same three failures: matrices, exact viewport pixels and disposed Clone refusal.
SOFTWARE skips honestly (exit77, unavailable custom-effects). Both800x480 captures
were visually inspected: the surviving cyan triangle is enlarged/centered because
the cloned matrices reset. No screenshot is treated as proof beyond those frames.
Frozen baseline unchanged;peak1,593,364,480/events0. No source edits before this run.

Candidate changes only ShaderEffect::Clone's existing disposal guard plus three
matrix assignments, and documents the ownership/state contract. No renderer or
uniform/texture-binding behavior changes. The four baseline witnesses are copied
byte-for-byte into native graphics tests. Native60373 is LIVE: one worker/private
GPU and network/caps0/6GiB max/no swap, focused4, graphics2975 and exact old10986
outcomes plus4newPASS required. Do not edit frozen sources or start a second heavy
job. After native acceptance: sanitizer, external real GL pixels/program lifetime,
other renderer capability controls, C API metadata regression and closure.

The intended pre-edit source snapshot command hit sandbox child-process EPERM.
`repair-shader-clone-source-baseline.json` therefore explicitly records HEAD bytes
recovered afterward, not an invented pre-edit snapshot; the current source is
verified exactly HEAD plus the four-line candidate/documented comment change,
with both Effect base files unchanged. Previously observed worktree state was clean
for those files and actual pre-edit linked binaries remain frozen in the baseline.

## Earlier preparation (historical)

2026-10-21: read-only review while070 native72714 runs. No055 production edits.
The prepared ShaderCloneStateTests.cpp is outside registered native sources,
UNCOMPILED/UNRUN. No original demo or marker changed.

ShaderEffect::Clone still allocates a fresh program and copies only its source
language/labels. Its own World/View/Projection stay at identity. It also omits
the inherited protected ThrowIfDisposedForCloneInternal guard that Effect::Clone
already uses. The public Effect contract requires independent current values and
ObjectDisposedException after disposal. ShaderEffect's intentional fresh-program
ownership contract does not promise to reset matrices; this is a CNA extension,
not a genuine XNA custom-GLSL API inference.

Proposed narrow repair: reuse that guard before device access/allocation and copy
three matrix values. Preserve fresh independent programs, source labels/language
and the absence of a raw-uniform/texture-binding copy promise. Clarify the public
override's matrix/disposal documentation. No renderer redesign is needed.

Four prepared structural tests cover nondefault state after source destruction,
independent mutation, exact disposed refusal and default/identity controls.
They deliberately reuse the existing native ShaderEffectTest placeholder source
strings: no shader-compilation or pixel coverage is claimed. Expected3FAIL/1PASS
is a hypothesis until run. Actual compilation/geometry/program-lifetime evidence
must come from the unchanged graphics.shader_effect.clone_state demo, on supported
OPENGLES3/OPENGL33 paths, preserving its nine assertions and screenshot inspection.
SOFTWARE's declared shader capability skip must remain a skip, not a fake pass.

Next after070 gates: reproduce both structural and actual public GL witnesses,
repair locally, native/sanitizer/external regressions, then close by repair SHA.
