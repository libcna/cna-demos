# ISSUE-113 distinct-query Begin: preliminary contract audit

## Genuine XNA observation (2026-10-22)

## Current engineering checkpoint

Latest (2026-10-22): external-v4 completed, with all132 earlier outcomes and assertion
labels unchanged, six exact visual references PASS, eight known ISSUE-082 failures
retained. C++18/C24 checks PASS on SOFTWARE/GLES/GL33; SDL_GPU exits77 only for its
declared missing capability. A deliberately broken supported factory fails exactly
one assertion and does not become a skip. See `repair-occlusion-external-v4-audit.json`.
Earlier live-job statements below are history, not current process state.

Before closing the candidate, the new numerical audit found a real incompatibility:
XNA's shipped `OcclusionQuery.IsComplete` stores GetData's DWORD in its signed int
field without saturation. `OcclusionCountXnaOracle.cs` invokes that actual method
against a constructor-free, finalizer-suppressed object and a rooted COM GetData
seam. Five exact DWORD boundaries and two pending controls pass; 0x80000000 returns
-2147483648, 0xffffffff returns -1. The seam does not establish actual GPU overflow.
`OcclusionCountGpuXnaOracle.cs` supplies that separate evidence on genuine XNA under
Wine/DXVK/RADV: a1024x1024 full-target draw counts1048576;2048 draws count-2147483648;
4096 count0;4097 count1048576. No query segmentation/mocked GPU is used in that oracle.
This is Linux/Wine reference evidence, not a native Windows run. Both reference jobs
completed without source drift or memory events (peaks320978944/350072832 bytes).
Reports: `repair-occlusion-count-xna-*` and `repair-occlusion-count-gpu-xna-*`.

The native14-case boundary fixture now returns10PASS/4FAIL against the still-unfixed
candidate, exposing signed-boundary saturation, uint32 wrap, high-bit arithmetic,
and loss of boolean visibility after a precise sum wraps. The sanitizer baseline
compiled both fixture and actual production coordinator, but did NOT link or run:
the driver incorrectly expected `build-asan/libcna.so`; that configuration builds
component archives. Preserve `repair-occlusion-count-native-before-audit.json` as a
failed validation attempt, fix the link, and reproduce UBSan before claiming it.
Intended correction: defined uint32 accumulation with signed bit interpretation,
plus independent any-sample state for intervals that fall back to boolean precision.
No numerical production fix or ISSUE-113 closure has been made yet.

External-v2/67313 completed its incremental build82641ms with no diagnostics and
passed the strengthened18-check C++ demo on SOFTWARE/GLES/GL33. C23 passes on
SOFTWARE/GLES, but OPENGL33 fails its original count assertion: it cached
precise=false before Begin, then read384000 actual samples. No assertions were
suppressed; the broader132/visual loop did not run after this focused failure.
The three reported changed inputs are C executable relinks from the completed
aggregate demo build, not source drift; all three link steps are present in its log.

Minimized55387 runs the unchanged raw EasyGL factory and the logical factory in
separate cold OPENGL33 processes. Both report cached0/current1/count496; both fail
exactly the old cached-precision assertion and pass the current-precision check.
This is observation evidence, not two PASS tests. EasyGLRenderer.cpp is unchanged
from HEAD (last touched by3b6a2eaf for003); its native target is resolved in Begin.
Source retained as OcclusionPrecisionCacheObservation.cpp. Build3824ms/link986ms,
raw374ms/logical312ms; peak465387520/events0/no drift. This disproves a coordinator
regression and demonstrates a pre-existing C demo expectation bug.

C demo now asks precision alongside the completed result; its original sample-count
assertion and all23 controls remain. Public C/C++ comments document the conservative
pre-first-Begin result; no runtime behavior was changed to satisfy this client error.
Revalidation of this correction and the remaining external gates runs in external-v3/
38286, with a900000ms build limit and exact recording of approved comment/client
source changes and rebuilt executable outputs. V2 SOFTWARE/GLES/GL33 screenshots
were visually inspected: wall, moving light and matching precision HUD, no residue
from the extra measurement squares. No cross-renderer pixel-equality claim.

Final-v2 consumer/multi92628:45PASS each SOFTWARE/GLES/GL33/FNA3D-OpenGL/WebGPU;
SDL_GPU/FNA3D-default31PASS/14 declared capabilitySKIP. No drift/events; peaks
493326336/280821760. Instrumented SOFTWARE ASan/UBSan/float-cast/leak coverage:
45PASS, no findings/drift, peak999878656/events0. Libraries111652ms, fixtures17579ms,
run6475ms. The actual coordinator TU and first-party dependencies are instrumented;
no third-party instrumentation claim.

The original demo now adds a separately measured square and7/2 nested intervals,
polling with the existing frame budget. Exact precise multiples or the declared
boolean result are asserted; all14 prior checks remain (18 expected total). Probe
draws are cleared before the visible scene. C documentation clarifies independent
handles and same-handle reentry. External21569 reached its300000ms build-only
timeout (actual SIGTERM/ETIMEDOUT, no tests started, no compiler errors or OOM;
peak1593294848/events0). The completed C API library relink is retained. An
authoritative host check confirmed the scope and compiler descendants had ended
before incremental continuation67313 with a900000ms build limit. External-v2 is
validating the last changes plus132 previous scenarios and six exact visuals;
not yet accepted. The failed attempt remains in the evidence, not overwritten.

Native85382 completed and accepted:11109 full cases =10991PASS/118 unchangedSKIP;
3094 graphics =3004PASS/90 unchangedSKIP. Exact previous outcomes plus19 new PASS,
including40 unstarted/40 nested query runtime witnesses. No source/artifact drift.
Build268108ms, focus1630ms, graphics41568ms, full1263236ms; peak4302938112,
memory.high1990 throttles/max0/OOM0. Slow genuine-XNA content/compiler tests ran fully.
Sequential92628 now validates45 cases on consumer/multi renderers and under SOFTWARE
sanitizers. The main demo still needs exact nested-count assertions, in addition to
its corrected acceptance check, before113 closure. No closure or product commit yet.

Multi86445 completed:39PASS each FNA3D/OpenGL and WebGPU;27PASS/12 capabilitySKIP
each SDL_GPU and FNA3D/default. Build232296ms, peak1799491584/no events or drift.
Two-slot minimized candidate witnesses93372 both failed with bad_alloc:
completed slots were retired after replacement allocation, and unstarted logical
objects each consumed a native query. These are candidate-design regressions,
not new historical issue IDs. Their frozen source and observation are
build-probe/repair-occlusion-capacity-test.cpp and
build/linux-full/reports/repair-occlusion-capacity-before-{run.log,audit.json}.

Refinement: one construction probe per live group, compaction before allocation,
and immediate release of a completed segment's native handle while retaining its
cached count for all logical consumers. The final group probe is destroyed inside
the owner lease. Both reproductions and11 coordinator units PASS in59248 (13 total).
Real rebuilt consumer/multi libraries47489/87477 subsequently pass43 cases each
on SOFTWARE/GLES/GL33/FNA3D-OpenGL/WebGPU; SDL_GPU/FNA3D-default each31PASS/12
declaredSKIP. No source drift, unexpected diagnostics or memory events.
Consumer/multi peaks1149460480/773070848. All previous assertions retained.

Native and evidence fixtures now additionally test40 unstarted and40 simultaneously
nested logical queries. Native85382 is running19 new cases, all graphics, and the
full aggregate against the prior11090-case baseline. These new runtime cases,
sanitizers, C API and external regression gates are not yet claimed complete.
The sections below preserve earlier chronological observations.

**Candidate focused validation:**44216 completed0: build269379ms/zero diagnostics,
fixture2665ms, six witnesses PASS SOFTWARE/GLES/GL33, SDL_GPU6 declaredSKIP;
corrected14-check original demo PASS on first three, SDL_GPU skips unsupported
queries. No drift/OOM; peak4303982592, memory.high1128 throttles/max0. Controls68477
completed0:39PASS each SOFTWARE/GLES/GL33, SDL_GPU27PASS/12 capabilitySKIP. Seven
new coordinator units protect pending shared results, reuse, disposal, boolean OR,
bounded completed-segment retention, saturation and failed allocation preserving
active intervals. Existing lifecycle/precision/buffer controls retained; both new
fixtures now mirrored natively. No drift/events, peak457297920.

Actual pre-candidate compiled-renderer baseline67361 reuses the pinned old library
and original six-test executable: WebGPU4FAIL/2PASS; FNA3D default driver SDL_GPU
advertises no queries (6SKIP). Forcing its available OpenGL driver in39309 gives
5FAIL/1PASS. Thus both independent native implementations also need the corrected
logical contract. Sole multi-after86445 now rebuilds the existing multi tree and
runs39-case coverage on SDL_GPU, FNA3D/default, FNA3D/OpenGL and WebGPU.
UNRUN capacity witness in build-probe simulates two native slots to check reclaim
before replacement allocation; inspect after the current frozen run. No closure.

**Candidate in progress:** native source audit also finds a single active pointer
in WebGPU; FNA3D forwards Begin/End directly to its native driver. A common logical
interval adapter avoids four independent renderer rewrites and leaves draw/raster
paths unchanged. Only logical Begin/End/disposal boundaries split native measurements;
the active logical set shares each segment. Precise counts add with saturation,
boolean fallback results OR, pending segments remain nonblocking, completed segments
retire opportunistically. A weak renderer-owned coordinator holds no native resource
after the last logical query disappears. Public queries retain construction-time
factory refusal and lease their owning context for native operations/destruction.

Candidate files: IGraphicsRenderer.hpp, OcclusionQuery.hpp/.cpp, new common
OcclusionQueryIntervals.cpp and native copy of the six overlap tests. The original
demo now requires distinct Begin acceptance, preserving every other control and the
knownIssue marker (restriction text explains the actual overlap defect). This is an
XNA-evidence-backed correction, not a relaxed expected value. No113 commit/closure.
Sole scope/session44216 performs one-worker303-task shared-header consumer rebuild,
then native and demo focus. Additional coordinator-unit evidence fixture7 cases is
prepared UNRUN; native integration and broader renderer/lifecycle/sanitizer gates
remain. Do not claim candidate correctness until these checks complete.

**Current CNA baseline85698 completed0 (observation, not pass):** new six-case
OcclusionOverlapTests.cpp compiles2019ms, zero unexpected diagnostics. SOFTWARE,
OPENGLES3 and OPENGL33 each5FAIL/1PASS; SDL_GPU6 declaredSKIP because it does not
advertise queries. The sole passing case disposes the inner contender. Five others
expose never-completing contender results (bounded3-second completion checks, no
sleeps/retries hiding failure); GL can additionally end the wrong owner's target.
The original unmodified demo fails its distinct-Begin assertion on SOFTWARE/GLES/
GL33 and skips SDL_GPU. No source/artifact drift/events, peak454422528. Exact XML,
logs and manifest: repair-occlusion-overlap-before-* in build/linux-full/reports.

Confirmed root causes: SoftwareRenderer retains only one active query pointer;
TryActivateOcclusionQuery refuses a distinct contender, but Begin returns void and
the wrapper still enters its Begin/End state. EasyGL attempts a second glBeginQuery
on the same target, then End unconditionally ends/erases the first owner. Both must
represent independent intervals, not add per-device refusal. The frozen six-case
fixture also distinguishes empty intervals and both active-disposal orders.
No113 production patch or demo expectation change has been made yet.

Next implementation investigation: keep SOFTWARE's no-query fast path but route
passing-sample counts to every active logical query; unlink only the ending/disposed
query. GL's single active target needs disjoint native segments whose results are
shared by the logical queries active in each segment. Sum precise counts and OR
boolean fallback results, preserving nonblocking completion, abandoned-run reuse,
context ownership, device-loss recovery and disposal. Inspect FNA3D/WebGPU as well:
the latter also has an active-query pointer. Minimize before choosing the shared
abstraction; do not redesign unrelated draw machinery or fake results. Add native
regression coverage when implementing, and preserve the genuine reference evidence.

Session33342 completed0: shipped Microsoft.Xna.Framework.Graphics4.0 assembly
SHA-25639aed7fd8e308ccace5ce9390256f1c829ca72c2eccd97d4a0f629f24015d3a5,
MVID8c1a51ed-a06c-475f-8fdf-50c74a6c71b0, under Wine/DXVK2.6/RADV AMD780M/private
GPU. Build2188ms/run726ms; peak354033664/events0/no input drift. This is not native
Windows validation. Source: OcclusionOverlapXnaOracle.cs; complete logs and pinned
assembly/source manifest: repair-occlusion-overlap-xna-{run.log,audit.json}.

One triangle returns496 samples. Distinct queries accept Begin in all four cases:
nested intervals return owner3472(7 triangles)/contender992(2); crossed intervals
return owner1488(3)/contender2976(6); disposing the inner query preserves owner3472;
disposing the outer preserves contender2976. Fresh and reused queries subsequently
return496 and992 in every case. Same-object reentry, End-before-Begin and reuse
before observing IsComplete all throw InvalidOperationException as expected.

Thus the historical **distinct-object refusal expectation is incorrect**. Do not
add a per-device rejection guard to satisfy it. Correct the ambiguous header/demo
with retained history, but first reproduce actual CNA overlap counts and lifetime:
Software's single active pointer and EasyGL's single native target are potential
independent compatibility defects. A return from Begin alone does not prove correct
overlap semantics. No CNA or demo expectation changed yet;113 remains unresolved.

## Earlier source-only audit

2026-10-22, source-only preparation while the086/112 full aggregate runs. No new
runtime reproduction, reference execution, product change or closure is claimed.

The permanent finding says a second OcclusionQuery on the same GraphicsDevice
must throw InvalidOperationException while another query is active. The current
public header says "if a query is already active". Current OcclusionQuery::Begin
only checks its own isInBeginEndPair_/hasIsCompleteBeenQueried_, calls the renderer,
then records a successful begin. EasyGL calls native begin before try_emplace in
its per-context owner map; End unconditionally ends/erases that context's owner.
That can differ from the public wording, but wording alone does not prove XNA's
cross-object contract.

The local genuine XNA decompilation is
../xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework.Graphics/
Microsoft.Xna.Framework.Graphics/OcclusionQuery.cs. Begin lines117-136 check the
same two per-object flags, issue IDirect3DQuery9::Issue(BEGIN), and translate a
negative HRESULT before changing flags. There is no managed per-device guard.
Therefore actual D3D/XNA behavior for overlapping *distinct* objects must be
measured, not inferred from the same-object error or GL's target restriction.

Primary documentation cross-check (2026-10-22): Microsoft's archived
[XNA Game Studio 2.0 reference](https://download.microsoft.com/download/D/F/6/DF6BE85E-8523-46DF-9DA6-730EABDDFDE4/XNA_Game_Studio_2.0_en-us.pdf),
OcclusionQuery.Begin, lists reentry before End and reuse before checking IsComplete;
the latter explicitly concerns the same query object. This older reference is
supporting evidence, not a substitute for XNA4 execution. Microsoft's
[D3D9 query documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d9/queries)
describes per-query building/issued/completed states; it does not establish a
cross-object prohibition. Pending results from multiple ended queries must not be
confused with overlapping active Begin/End intervals. No documented cross-object
exception has been established by this search.

Next, after the active batch's gates are collected: run the original
graphics.occlusion_query.visible_pixels on current CNA, then a genuine Microsoft
XNA reference with two distinct queries, recording second-Begin exception type,
owner/contender End and disposal ordering, and subsequent reuse/completion.
Do not run Wine concurrently with the full native XNA differential test or any
other job using .wine-cna-xna40. Keep same-object reentry separate from overlap,
and verify two-device ownership if current source evidence calls for it. Only
then choose a compatibility repair versus a documented demo/header correction.
