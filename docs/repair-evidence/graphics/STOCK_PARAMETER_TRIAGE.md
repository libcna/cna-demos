# ISSUE-034/035 stock parameter semantics: next-batch source audit

## Full-aggregate follow-up discovered during086/112 (2026-10-22)

**Validated follow-up:** CNA commit0a94a68aca7b3b482918306ec5426bc0ff74d6e9
corrects only GltfDrawParamsOracleEXTTests.cpp and updates the campaign handoff;
no runtime change or assertion suppression. Native-v2/session98260 completed0:
full11090=10972PASS/118 unchangedSKIP in1249784ms, exact old aggregate/graphics
outcome parity, no missing/changed cases or source drift, peak2465263616/events0.
Incremental build6904ms has no diagnostics; corrected glTF2PASS2915ms and context
8PASS470ms. The original full97896 remains recorded:10970PASS/118SKIP/2FAIL,
1193421ms, no drift/events, peak3684462592. Both original failures are retained.
First v2 launch typo stopped255 before any build/test; no duplicated heavy job.
The rest of this section records the original investigation before correction.

The full native run97896 has reported two failures in GltfConformanceL6:
SkinnedDrawBindsThePaletteAndFourInfluencesPerVertex and
AFreshlyLoadedSkinnedModelIsAlreadyPosedInItsBindPose. Both fail their palette
cardinality assertion for the stock SkinnedEffect on skin-unlit (actual72 versus
authored1). The preceding034/035 gate ran full graphics and selected aggregate
effects, not these content tests; no prior claim of a post034/035 full aggregate
pass is made. The current full run is still live; collect before changing tests.

Reference evidence was rechecked: genuine Microsoft XNA v5 audit has41PASS,
including GetBoneTransforms(72).Length==72 and identity in untouched bone71 after
a two-matrix write. Existing SkinnedEffect::FillGpuDrawParams explicitly requests
GetBoneTransforms(MaxBones) and uploads every returned matrix; that code was not
changed by034/035. The custom SkinnedPbrEffect still uses variable-sized parameter
storage and must retain its separate current contract.

Planned test correction: stock captured palette must have exactly72 entries,
custom PBR exactly the authored count; retain every original authored matrix
component comparison at the original tolerance, and additionally require each
untouched stock-tail element to equal identity exactly. Preserve the corpus's
non-identity bind-pose witness and four-influence assertion. Do not change CNA
runtime behavior or loosen counts to a lower bound. Correction not yet applied or
validated; both full-run failures remain recorded and unaccepted.

## Genuine reference and native witnesses (2026-10-22)

**Closure:** product committed locally as
`d650554a89b967cebca5af6b40917b9ded8294c8`, only034/035 files and plan hunk;
inherited011/082 work remains uncommitted. Metadata3898 accepted: incremental
build5479ms/no diagnostics, registry719, four renderer33-check PASS, exact only
restriction/knownIssue removal, five enabled CTests PASS/noSKIP7298ms. No frozen
input drift, peak1862242304/events0. Permanent034/035 descriptions and failed
observations retained; resolved marker removed only after product validation.
One knownIssue demo becomes unmarked PASS for two issue IDs. No push/merge and no
claim that the separate082 failures or full final campaign are resolved.

**All product gates complete:** compiled-on SDL_GPU/FNA3D/WEBGPU each127PASS,
zero skips/failures. Initial25845 built168710ms with no diagnostics and SDL_GPU
ran7292ms successfully; its inventory assertion incorrectly expected116 because
the prepared driver omitted11 existing EffectApplyTest cases. Native XML and the
two source files prove100 existing tests (34 EffectTest+11 EffectApplyTest+55
EffectParameterTest), plus27 new. Resumed85003 verifies the exact127 names, reuses
the completed build/SDL run and executes FNA3D8652ms/WebGPU6052ms. No assertion,
product source or skip policy changed. Nine compiled tests skipped by the default
configuration now actually execute and pass on these renderers. No drift/events;
initial peak1776058368/resumed158388224. Reports multi and multi-v2 retain both gates.
No heavy job live; coherent product commit and exact demo-metadata closure next.

**External and sanitizer gates collected:** external13160 ended2 deliberately for
the eight unchanged082 failures, with parityAccepted:true and no deltas:132 prior
cases=123PASS/8FAIL/1SKIP. Eight DDS/C controls PASS; all37 catalogued stock/effect/C
scenarios on four renderers=142PASS/6declaredSKIP, no failures. Skips are the three
EasyGL-only shadow examples on SOFTWARE/SDL_GPU. Six exact visual references PASS
with zero pixel/channel difference. All ten screenshots individually inspected:
four gold two-bone strips bend at the joint on navy background (same nonbackground
bounds376,147..538,385); font digits, cornflower clear and checkerboard references
retain their authored layout. No cross-renderer exact-shading equivalence claimed.
No drift, peak194445312/events0.

Sanitizer-after84611 completed0: actual instrumented SOFTWARE product/Runtime,
all27 cases PASS under ASan/UBSan/float-cast-overflow/leak detection with no findings
or suppressions. Incremental libraries6456ms/strict fixture8508ms/run243ms/audit34ms;
no drift, peak837709824/events0. The before-fix NaN failure remains retained.
Compiled-on SDL_GPU/FNA3D/WebGPU validation is the remaining runtime gate before
product commit and metadata closure; it expects27 new+89 existing cases per renderer.

**Post-fix native gate accepted:**40268 completed0. Incremental build15557ms with
zero diagnostics, focused27PASS1964ms, full graphics3067=2977PASS/90 unchangedSKIP
56076ms, selected aggregate effects1128=1107PASS/21 unchangedSKIP116689ms.
Exact comparison168ms preserves every prior graphics/selected outcome and adds27
passing native cases. No drift, peak4000546816/events0. Reports native-v3.
External13160 is the sole live scope, comparing132 prior outcomes plus8 DDS/C
controls, every stock/effect/C scenario on four renderers and six exact visuals.
Sanitizer-after and compiled multi remain pending. The next086/112 source audit is
separate in DEVICE_CONTEXT_TRIAGE.md; its baseline driver is prepared, not run.

**Nonfinite conversion reproduced and repaired:** sanitizer36243 ended1 with the
exact UBSan diagnostic `nan is outside the range of representable values of type
'int'` at EffectParameter.cpp:450. Actual instrumented product/Runtime build73301ms
and strict fixture4476ms succeeded; execution216ms stopped at the new witness.
No drift/suppressions, peak1160265728/events0. The narrow repair avoids integer
conversion when the destination stores floats; requested integer conversions are
unchanged. Post-fix focus99893:27PASS plus original33 demo checks PASS on all four
renderers, build71950ms/fixture3682ms, no drift/OOM, peak4297433088, memory.high258
(throttling), max/oom/oom_kill0. First focus2325 stopped before build because the
sanitizer had legitimately refreshed an archive in the old manifest; resumed with
the completed sanitizer's frozen inputs, without rerunning an expensive build.
Native-v3 is the next gate; external/visual/ASan-after/multi still pending.

**Resumed checkpoint:** native51126 completed0 after the successful1357 build's
six pre-existing nodiscard diagnostics were explicitly reviewed. All four warning
source files match HEAD; no suppressions or unrelated edits. Full graphics3066
=2976PASS/90 unchangedSKIP; selected aggregate effects1127=1106PASS/21 unchangedSKIP;
focused26PASS, exact prior outcome parity. Reports native-v2 and
native-diagnostic-review.json. Peak635252736, memory events0, no source drift.
This does not replace the last full aggregate native checkpoint.

Genuine XNA v5 completed41PASS (build3103ms/run1016ms), including NaN/+Infinity/
-Infinity scalar broadcasts, peak341856256/events0/no drift. Native witness27
preserves all12 nonfinite matrix components and adjacent-bone identity. A source
review found SetCompiledScalarValue unnecessarily casts floatingValue to int even
when storesFloat is true. Sole live sanitizer-before/session36243 builds the actual
instrumented SOFTWARE closure to reproduce this before a narrow repair. No closure
or product commit yet; knownIssue remains. Prepared focus-after-v4 expects27 cases.

**Latest checkpoint:** session86220 is terminal0: strict build53805ms0, fixture
2928ms0,26PASS and corrected demo33PASS on each SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU.
No source drift/events, peak4037730304 bytes. Sole running job is native1357,
cna-repair-stock-parameters-native.scope: full graphics and selected CnaTests
effects with exact previous-outcome comparison. Collect before sanitizer/external.
The complete final campaign remains due; no034/035 closure/commit yet.

The preceding25-test candidate/session88163 also passed all four renderers and
the corrected demo; build499448ms, peak4301172736 bytes, memory.high1157 throttles,
max/oom/oom_kill0. New genuine v4/session39765 is38PASS: scalar broadcast writes all
12 components of a bone matrix and leaves adjacent bones unchanged; array-parent
scalar writes refuse. New native boundary witness/session17543 then records
25PASS/1FAIL per renderer against that candidate: first-element float/int/bool
broadcast corrupts its next bone, and last-element broadcast misses components.
The final candidate uses the shared matrix register dimensions for broadcast and
explicitly bounds every scalar cell to compiledByteSize_;26 witnesses now pass.
Reports preserve all intermediate failures under repair-stock-parameters-
{bounds-before,focus-after-v3,v4-xna}. No source/test suppression or tolerance change.

Latest live validation: session88163, scope cna-repair-stock-parameters-after-v2,
one-worker consumer library/demo build followed by25 native tests and the corrected
33-check demo on four renderers. No closure. Prepared subsequent drivers:
repair-stock-parameters-native.mjs (full graphics + selected native effect tests,
exact old outcome comparison), -sanitizer.mjs (existing instrumented SOFTWARE
closure, ASan/UBSan/float-cast/leaks), and -external.mjs (132-case baseline plus
all catalogued stock/effect/C scenarios, DDS controls and six exact visuals).
Run them sequentially, never alongside the live build.

Candidate69336 is terminal1, not green: product build159143ms0, strict fixture
2942ms0, SOFTWARE22PASS; unchanged demo then fails exactly four raw-M44 assertions.
No source drift; peak1512153088/events0. Remaining renderers were not attempted
by that fail-fast driver. Extended genuine session30250/v2 is terminal0:36PASS,
transpose observations match the mathematical transpose with the missing column
zero; flat float reading is row-major without the padding column. Native layout
session25641 then proves23PASS/1FAIL on all four renderers: the newly added numeric
array read exposes the column-register mapping error in the candidate. No drift,
peak444792832/events0. It is corrected by bounded logical-to-storage indexing.

Genuine v3/session65073 is terminal0:36PASS, peak312160256/events0. A16-float write
of1..16 yields raw first matrix `1,2,3,0;4,5,6,0;7,8,9,0;10,11,12,0` and raw second
`13,14,15,0;16,1,0,0;0,0,1,0;0,0,0,0`: untouched components survive. The same indexing
is now used for numeric-array reads/writes, with one additional native test.
Microsoft XNA EffectParameter.SetValue numeric-array IL independently preserves
the old matrix and replaces only supplied row-first components. The demo now
expects raw4x3 values/transpose and additionally checks both complete affine
GetBoneTransforms results (31→33 checks, same tolerance). KnownIssue is retained
until broad validation; no expected failure has been hidden.

Session42602 completed0: genuine Microsoft XNA4 Graphics assembly
8c1a51ed-a06c-475f-8fdf-50c74a6c71b0 (MVID), actual HiDef device on AMD780M,
Wine/private Xwayland/DXVK. All36 checks pass:30 scalar-refusal checks over five
stock effects;72 bone views, retained capacity/tail after partial writes,
parent/child aliasing and clone independence. Peak691195904 bytes, all memory
events0, no source drift. This is not native Windows hardware validation.
Reports: repair-stock-parameters-xna-{build,run}.log and -audit.json.

Unlike the stored FNA fixture, genuine XNA reports Bones parent and children as
4 rows/3 columns. Raw default matrix has diagonal(1,1,1,0); after affine
scale(2,3,4)*translation(5,6,7), raw parent/child is
`2,0,0,0;0,3,0,0;0,0,4,0;5,6,7,0`. Public GetBoneTransforms restores M44=1.
This disproves the demo's incidental assumption that raw Bones preserves M44=1;
correct that expectation with explicit reference evidence, not relaxed tolerance.

Native StockParameterContractTests.cpp adds22 cases, mirrored into CNA's graphics
tests. Session8001 completed0 as an observation against unchanged product:
each SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU yields16FAIL/6PASS. The ten stock/clone
shape witnesses and six Bones witnesses fail; five valid-scalar controls and the
standalone-extension control pass. Strict -Wall/-Wextra/-Werror compilation2269ms;
peak570241024 bytes, no memory events/input drift. Reports use
repair-stock-parameters-focus-before. No failure is suppressed or marked resolved.

An uncommitted candidate now marks stock parameter construction explicitly and
shares the compiled parameter's existing bounded storage for matrix arrays, with
XNA row/column metadata and zero-fill distinct from FNA register reflection.
Scalar checks leave independent constructor storage unchanged. Session69336 is
the sole candidate build/focus process; collect its authoritative result before
another heavy job. No broad validation or closure yet. The original demo is
expected to expose the raw-M44 oracle error; keep that failed run as evidence.
An extended genuine oracle is prepared (reference-v2 driver, not yet run) to
inspect transpose and flattened-float matrix conventions before finalizing035.

## Confirmed current baseline

After054 closure, CNA bda0cf902 (product b6ab302cc) / demos bdd0c05:
session80776 completed0 as an observation, not a green suite. Unchanged
graphics.effect.parameter_values runs all31 checks on SOFTWARE/OPENGLES3/
OPENGL33/SDL_GPU:29PASS/2FAIL each. The only failed labels remain scalar getter
refusal034 and72 Bones elements035. Current getters print the first component;
Bones still reports0 elements/72 rows/4 columns. No source drift, peak140460032
bytes/all memory events0. Reports:build/linux-full/reports/
repair-stock-parameters-before-{SOFTWARE,OPENGLES3,OPENGL33,SDL_GPU}.json/.log
and repair-stock-parameters-before-audit.json. No new product code or tests yet.

C API EFFECTS.md distinguishes standalone parameter creation from aliases into
effect collections; do not change standalone storage/array defaults incidentally.
XNA SkinnedEffect.GetBoneTransforms(count) additionally forces each M44 to1 after
reading the parameter array; inspect this distinction before asserting raw Bones
matrix behavior. An affine bone transform test must protect parent/element aliasing,
partial-array writes and clone independence, not just collection cardinality.

## Original read-only audit (before the baseline above)

2026-10-22, while ISSUE-054 external validation ran. At that point no new034/035
runtime result, fix or closure was claimed.

The permanent reproduction is graphics.effect.parameter_values. Current
EffectParameter.cpp applies RequireCompiledScalarGetterShape only inside the
compiledStorage_ branch of GetValueBoolean/GetValueInt32/GetValueSingle. The
stock path still takes the first vector component (or default integer storage).
RequireCompiledScalarGetterShape itself already expresses the relevant rule.

The actual Microsoft XNA4 decompilation in sibling xna4-decomp, under
reference/xna4/decompiled/windows/Microsoft.Xna.Framework.Graphics/
Microsoft.Xna.Framework.Graphics/EffectParameter.cs:1442/1631/1808, applies the
same guard to all three scalar getters: a non-Scalar with no Elements throws
InvalidCastException before calling D3DX. This is source evidence, not a fresh
Windows GPU execution. Preserve array handling rather than assuming all vectors
are rejected, and distinguish CNA's public standalone constructor contract from
stock effect reflection before changing behavior. Review C API coverage/docs.

The existing private CompiledStorage already shares bounded bytes among element
and structure views. Read its matrix accessors and Effect's reflection factory
before inventing another aliasing mechanism: independent vector-backed children
would break parent/element mutation semantics. Preserve the documented standalone
constructor behavior unless separate contract evidence authorizes a change.

Current SkinnedEffect.cpp:118 still creates Bones with MaxBones rows/four columns
and no element collection. Read the existing fna-effect-reflection.json as JSON
(it is a single very long line): its SkinnedEffect Bones entry is the recorded
reference, not an inference from HLSL spelling. Parent/element writes, clone
independence and rendering must stay coherent; merely fabricating72 independent
metadata entries would not be an adequate root-cause repair.

Parsed the stored FNA JSON directly: SkinnedEffect.fxb Bones is Matrix/Single,
parent3 rows/4 columns,72 elements, first child Matrix/Single3x4 with12 value
components. No genuine XNA dimension measurement is claimed from this FNA file.

Next: run the unchanged demo on available renderers after054 closure, inspect
native Effect/SkinnedEffect tests and genuine XNA reference, minimize the public
stock-versus-standalone distinction, then repair a coherent shared root cause.
Do not change the pending066 blend-toggle contract while doing this batch.
