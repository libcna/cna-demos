# ISSUE-113 distinct-query Begin: preliminary contract audit

## Genuine XNA observation (2026-10-22)

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
