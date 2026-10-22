# ISSUE-086/112 device-context boundary: next-batch source audit

## Resumed validation (2026-10-22)

**Closed against CNA9e4d0279affcb9f05f9d28807403f798fc948906.** Metadata93054
completed0: build12006ms/no diagnostics, registry719 and exact two-marker catalog
delta; twelve unmarked demo runs PASS with unchanged17/12/27 checks; seven enabled
CTests PASS/noSKIP14326ms. No drift/events, peak2029166592. Native fixture is part
of the product commit; permanent historical IDs retained and marked Resolved.

External49730 completed1:132 exact prior outcomes (123PASS/8 separate082FAIL/1SKIP),
8 DDS/C controls PASS;192 selected device/texture runs183PASS/7SKIP/2FAIL. The two
SDL_GPU failures are independently tracked as DC-R1/DC-R2, with a source-based proof
that this batch's null SDL context lease cannot cause them. They remain failures,
not newly expected outcomes. See DEVICE_CONTEXT_EXTERNAL_RESIDUALS.md. Separate
external-review8714 completed2: scoped regression accepted, fullRegressionGreen=false;
six exact visuals PASS6753ms, all six screenshots inspected, no drift/events,
peak196763648. Final campaign requires dispositions for both supplemental findings.
No push/merge. Next113 genuine reference disproves distinct-object refusal; native
overlap correctness must be investigated, not assumed from Begin returning normally.

**Multi-renderer review completed:** multi-v2/session11834 exited1 only because
its wrapper demanded65 PASS; all three runtime processes exited0 without failures.
SDL_GPU52PASS/13SKIP, FNA3D55PASS/10SKIP, WEBGPU52PASS/13SKIP. Each has8 new context
witnesses PASS. Existing GraphicsDeviceValidationTests gates ten cases to SOFTWARE/
OpenGL33/OpenGLES3. Three RenderTargetSemantics cases catch explicit cube-MRT refusal
on SDL_GPU/WebGPU (documented WebGPU limitation; unchanged renderer source refusal).
Their native source is byte-identical to HEAD. Separate multi-review verifies every
XML case, exact skip messages, source/artifact hashes and the eight required passes.
No tests changed or outcomes relabelled; original rejected audit remains retained.
Remaining fixture compiles2360/1539/1861ms, link551ms; runs5104/6104/4730ms.
Peak497442816/events0/no drift. Prior multi38383 stopped on warning path spelling
after successful library build16432ms and fixture2149ms; prefix-only normalization
proves the same145 reviewed header warnings, no suppression; build/object reused.
External49730 now runs the next gate; no086/112 commit or closure yet.

**Current native/sanitizer checkpoint:** native-v2/session98260 completed0:
full11090=10972PASS/118 unchangedSKIP, exact previous outcome parity/no missing
cases, plus8 new graphics witnesses; no drift/events, peak2465263616. The glTF
test-only follow-up is committed separately as CNA0a94a68aca7b3b482918306ec5426bc0ff74d6e9,
with demos evidence20b55b5e90db5718bf1fb20f966b5f112c1233a0. No086/112 product commit yet.

Sanitizer38435 completed0:8PASS, actual SOFTWARE CNA and Sharp Runtime instrumented
with ASan/UBSan/float-cast-overflow, leak detection enabled, no findings/suppressions.
Libraries13418ms, strict fixture3584ms, execution95ms, inspection20ms; unchanged
145 inherited header warnings explicitly reviewed, no new diagnostics or source
drift, peak863215616/events0. This is common object-lifetime coverage, not GL
handle-deletion instrumentation. Sole live scope cna-repair-device-context-multi/
session38383: existing multi-renderer library plus source-derived inventory of
context, device validation/disposal and render-target semantics tests. Collect
before external gate, product commit and exact metadata closure. No push/merge.

**Latest:** native97896 terminal1: graphics3075=2985PASS/90 unchangedSKIP;
full11090=10970PASS/118SKIP/2FAIL in1193421ms. Only two stock-palette cardinality
expectations in GltfConformanceL6 fail, as documented in STOCK_PARAMETER_TRIAGE.md.
The genuine XNA reference proves fixed72 capacity; test-only correction preserves
all matrix assertions and additionally checks untouched identity tails. No product
change was made for these two failures. Corrected2PASS2915ms/context8PASS470ms after
incremental build6904ms/no diagnostics. Sole live scope
cna-repair-device-context-native-v2-run/session98260 repeats full aggregate/parity.
Collect before sequential sanitizer/multi/external drivers. Initial v2 ip-command
typo stopped255 before any build; corrected launch only. No push/merge/closure.

Continuation check: v2 process2929278 remains live under session98260 and has
passed both corrected glTF cases within the full run (1250/1239ms). No failures
have been reported so far; this is not a completed pass. Prepared metadata driver
repair-device-context-metadata.mjs is UNRUN and requires the recorded CNA product
commit plus accepted external gate. It verifies exact two-marker removal, keeps
the cube readback limitation, reruns three demos on four renderers and seven enabled
CTests. Read-only CTest inventory confirms all seven exist and none is disabled:
target2D, cube transfers, volume, backbuffer readback, and the three CLI gates.
Catalog shape was checked (knownIssueRenderers is always an array). Product/demo
markers have not been removed. External gate follows sanitizer/multi and carries
their latest frozen artifact manifest, avoiding stale pre-build library hashes.

Readback-before41922 is terminal0 observation: SOFTWARE/SDL_GPU8PASS; GLES and
OPENGL33 each5PASS/3FAIL (unbind readback plus independent Color/packed overloads).
Peak451362816/events0/no drift. Added the missing lease immediately around the
shared native read, after every argument/profile/format/binding validation.

Focused after-v2/session8590 completed0: all8 native cases and17/12/27 original
demo checks PASS on all four renderers. Product build53767ms/strict compile2226ms,
peak3899883520/events0/no drift. All12 screenshots inspected: correct target-arrow
scales/tint, six cube faces plus authored mips, four volume gradients with white
patches only in slices1/2. In particular both previously black GLES atlases render.
Exact original source preservation and knownIssue metadata checked; only two exact
volume pixel assertions added. Both IDs remain open for broad regression/commit.

Sole live native scope cna-repair-device-context-native/session97896: one-worker
build20183ms, focused8PASS1834ms, full graphics3075 cases completed0 in39752ms;
the full aggregate CnaTests is running (not the earlier filtered effect subset).
Collect before launching prepared external/sanitizer/multi drivers. The sanitizer
tree is SOFTWARE-only: it tests common object lifetime, not GL native handle
deletion. Real GL context behavior is covered by the runtime witnesses above.

The initial after gate is terminal, not live: product build77425ms and strict
fixture compile2994ms succeeded, SOFTWARE6 native and17/12/27 demo checks PASS.
It stopped at a driver assertion: successful checks are silent in ordinary logs,
so comparing log labels cannot establish preserved assertions. No runtime failure
was hidden. The resume driver verifies baseline source hashes instead: cube and
target sources unchanged, volume byte-identical after removing only the new include
and two framebuffer assertions. The completed build/SOFTWARE results were reused.
Initial scope peak4285431808/events0; no input drift.

Resume42952 then genuinely failed: GLES5PASS/1FAIL, no drift/events,
peak216211456. Binding2D/cube/MRT and both lifetime witnesses now pass. Unbind
retains the correct target content, but secondary GetBackBufferData reads the
primary's CornflowerBlue instead of Orange. Source confirms the shared readback
core also lacked an owning-context lease. No assertions were relaxed. Two extra
native/evidence witnesses independently exercise all Color and packed overloads
without target binding. Readback-before scope/session41922 is the sole live job;
collect its four-renderer observations before patching or rebuilding CNA.

Neither issue is closed. Broad native, other-renderer, visual and sanitizer gates
remain due. Product candidates are only GraphicsDevice.cpp plus the new native
fixture; demos adds volume pixel checks and permanent evidence. Inherited011/082
changes remain untouched. No push or merge.

## Current reproduction and minimized witnesses

CNA1edc8542c (034/035 productd650554a8), demos c352e47. Original42457 completed
12 observations: SOFTWARE/SDL_GPU all three demos PASS; GLES112 target binding
fails one of17 assertions and086 cube atlas fails two of12; OPENGL33 also fails
the112 binding assertion. Texture3D's25 controls PASS on all four, but inspected
GLES screenshot is entirely black, as is the GLES cube atlas; OPENGL33 cube draws
correctly. No drift/events, peak196317184. No broad cross-platform claim.

New DeviceContextBoundaryTests.cpp (mirrored natively) has six cases: secondary
2D/cube/MRT binding, secondary unbind retaining target content and restoring its
backbuffer, secondary Dispose preserving the primary framebuffer, and destructor
preserving primary texture drawing. An existing public renderer lease models the
primary frame; no private access or renderer-specific fallback. On unchanged CNA,
24800 reproduces SOFTWARE6PASS, GLES6FAIL, OPENGL33 binding4FAIL/lifetime2PASS,
SDL_GPU6PASS. MRT fails with IncompleteMissingAttachment/InvalidOperation; unbind
clears the still-bound target instead of its backbuffer. Initial primary clear
controls pass before secondary destruction, then GLES reads black. No drift/events,
peak442527744; compile2935ms, runs82/530/485/1684ms.

First strict probe30189 stopped at compile:145 unused-parameter diagnostics in the
unchanged IGraphicsRenderer.hpp's default virtual methods. Native project test
flags do not enable Wextra/Werror. The resumed probe retains every warning,
checks the exact145 diagnostics and header HEAD hash, and keeps all other warnings
as errors. No product/header warning cleanup or test suppression was performed.

Candidate (uncommitted) changes only GraphicsDevice.cpp: lease the owning context
through actual SetRenderTargets/unbind plus viewport/scissor/discard work, after
public validation/no-op checks; lease renderer destruction in a bounded scope
before the retiring window is destroyed. Existing shared lease control keeps the
native context alive while renderer members release, then restores the other
device's binding. No EasyGL/dependency code change. Texture3D adds two exact pixel
assertions for all four gradients and the two white sub-boxes (25 to27 checks).
The original25 controls and all cube/target assertions remain unchanged.

Sole live scope cna-repair-device-context-focus-after builds the consumer once,
then six witnesses and all three demos/screenshots on four renderers. Collect
before any next heavy job; broader native/renderer/lifetime sanitizer validation
and commit/closure are still required. Neither086 nor112 is resolved yet.

## Initial source audit

2026-10-22, read-only product review while034/035 regression gates run. Neither
issue has a fresh reproduction or repair in this batch yet. Do not infer current
failure from the historical reports; run the original demos first.

`GraphicsDevice::SetRenderTargets` calls the renderer for binding/unbinding and
resets viewport/scissor without acquiring the device's thread-context lease.
`Clear`, `Present`, and `Reset` already use `AcquireRendererThreadContextLease`,
which requests `RestorePreviousBinding`. The historical112 failure occurs before
destruction: a secondary32x32 device binds a20x10 target, clears LimeGreen, unbinds,
and reads transparent black. Public cross-device move controls are independent.
Minimize both primary/secondary alternation and binding/unbinding outside a frame;
cover2D/cube/MRT paths where available, preserving no-op and refusal semantics.

086 is a separate lifetime finding: disposing a secondary device leaves subsequent
primary frames black. `destroyNativeResources` resets renderer/presenter/window
under NativeDeviceLifecycleMutex without an explicit context lease. This alone is
not proof of its root cause: inspect EasyGL destructor, per-context registry and
lease restoration before changing ownership/teardown. Protect resource identity
and ensure a lease cannot outlive a destroyed context.

Original demos: graphics.render_target.render_target2d (112),
graphics.texture_cube.face_transfers and graphics.texture3d.volume_data (086).
Prepared UNRUN driver: build-probe/repair-device-context-before.mjs, requiring the
completed034/035 metadata checkpoint, then12 observations/screenshots across
SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU. It does not change knownIssue or swallow failures.
No context/lifetime production patch or new native test has been applied yet.
