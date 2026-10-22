# Blend state and source ranges (ISSUE-027 / ISSUE-066)

## Evidence boundary (2026-10-21)

### Current continuation (2026-10-22)

ISSUE-003 is committed as CNA3b6a2eaf3 with metadata/evidence commits
CNA30c872915 and demos2031ddd. Its native Windows runtime requirement is explicit.
The027 range repair is committed as `766d62fbd560b8356f43b169c3ebfdf2ccad3a7e`.
All available affected validation gates below are accepted.066 has no repair yet.

Actual baseline driver `build-probe/repair-blend-range-before.mjs`, final prefix
`repair-blend-range-before-v7`, session40802 completed0 as an observation:
SOFTWARE12FAIL/2PASS; GLES14PASS; GL3314PASS; SDL_GPU6PASS/8compiled-offSKIP.
The six original source-range witnesses and eight compiled/MRT cases distinguish
Color/Rg32/Rgba1010102/Rgba64 from HDR and exercise both MRT attachment orders.
Only the Rgba64/HdrBlendable pair mixes normalized and float storage; the32-bit
pairs use a Color sibling to respect XNA's supported blend formats and bit depths.
Original C draw_routes fails on SOFTWARE and SDL_GPU at the same accumulated-red
assertion (255 instead of approximately192); GLES/GL33 pass. SDL's failure remains
unexpected/unmarked, not converted to a known failure. No source drift or memory
events; peak515702784 bytes. Build3901ms; range runs80/607/574/1444ms.

Prepared-fixture corrections, with every prior log retained:

- First compile found a nonexistent CanReadRenderTargetsBack helper; removed it,
  retaining the actual half-float capability requirement. No runtime assertion changed.
- Initial HDR expectations assumed Clear(ClearOptions,Vector4) preserved0.25.
  Genuine XNA GraphicsDevice.cs lines1827ff first constructs Color; CNA correctly
  does the same, producing a half-float background0.2509765625. Use the explicit
  raw-float Clear extension to isolate blend math with exact binary inputs instead.
  All six expected values and tolerances remain unchanged. Corrected v3 baseline
  gives SOFTWARE4FAIL/2PASS, all three other renderers6PASS.
- Initial32-bit MRT companions Single, then HalfVector2, were invalid for alpha
  blending. XNA ProfileCapabilities.cs InvalidBlendFormats lists both (and all
  float formats except HdrBlendable). The final test uses valid normalized32-bit
  pairs plus the64-bit mixed pair; it does not bypass the correct profile refusal.
- CompiledEffectFixtures.hpp has pre-existing switch-enum warnings at line6105.
  Its bytes match committed CNA. Native test flags do not use -Wall/-Wextra/-Werror;
  the focused build retains Wall/Wextra and records every diagnostic, permitting
  only that exact unchanged-header warning category. All other diagnostics fail.
  No warning suppression, helper edit or warning-free claim. v4 strict failure is
  retained alongside v5-v7 diagnostic-review JSON.

Candidate: SoftwareRenderer.cpp locally clamps source RGBA to0..1 for the four
normalized target formats before factor/equation evaluation. Floating inputs stay
unclamped. Both generalized and compiled/MRT paths use the same helper, separately
for each attachment. Final store quantization remains unchanged. Native regression
copies live in modules/graphics/tests, with unchanged evidence assertions.

Actual focused after15346 completed0: product build8815ms/no diagnostics, fixture
build3881ms (reviewed existing shared-helper warnings), SOFTWARE/GLES/GL33 each
14PASS; SDL_GPU6PASS/8compiled-offSKIP. All original066 outcomes remain unchanged.
No input drift; peak902090752 bytes, all memory events0. Reports
`repair-blend-range-after-*`. This is not yet a completed repair or final regression.

Native69993 completed0: existing debug targets built30717ms/no diagnostics,
focused14 completed509ms (6PASS/8compiled-offSKIP), graphics3026 completed44988ms
(2936PASS/90SKIP), full11041 completed1157090ms (10923PASS/118SKIP). Comparator100ms
proves exact prior11027 outcome parity plus those14 new cases. No input drift or
memory events; peak4056010752 bytes. Private GPU/network, owner UID/capabilities0,
one worker/shared cache/6GiB/no swap. Scope verified inactive/dead after completion.
Reports `repair-blend-range-native-*`; the compiled cases were separately executed
in the enabled focused consumer configuration, not counted as native skip coverage.

External47085 terminated2 with parityAccepted:true: exactly132 previous outcomes
(123PASS/8existingFAIL/1SKIP), no delta in identities/status/checks/markers/assertion
labels, including already-validated003 rectangle checks9. The eight082 failures
remain unaccepted, hence exit2 rather than a green suite. Original HDR demo passes
four checks on each SW/GLES/GL33/SDL_GPU; six exact reference comparisons pass
with zero differing pixels and zero tolerance. No frozen-input drift/events;
peak193499136 bytes. All ten images individually inspected: HDR shows the intended
distinct accumulated brightness and labels, and font/clear/checkerboard references
are unchanged. Reports `repair-blend-range-external-*`.

Final CTest57592 completed0, five enabled tests PASS/noSKIP in24422ms: HDR,
sprite blend states, rectangle SetData, registry validation and catalog parsing.
No input drift/events; peak148176896 bytes. Reports `repair-blend-range-ctest*`.
Product commit766d62fbd contains the localized numerical correction and14 native
regressions, excluding inherited011/082 edits.027 is FIXED_VALIDATED. No new
sanitizer run is claimed: this change introduces no lifetime/ownership operations.
No new full719 smoke is claimed; its final all-issue rerun remains required.
There was no027 knownIssue marker to remove. No push/merge. All listed jobs ended.

For066 the owner was asked asynchronously whether enabling should install
SourceAlpha/InverseSourceAlpha/Add (existing GLES/demo behavior) or preserve
factors as a pure enable switch. No answer is presumed;027 continues independently.

### Historical preparation

Read-only production audit at CNA a1c43a51f with the recorded uncommitted
011/082/003 candidates. Native003 regression68394 is still running: no second
heavy job, production edit, new blend runtime result or issue closure.
`BlendSourceRangeTests.cpp` contains six **UNCOMPILED/UNRUN** witnesses outside
the running job's frozen input manifest. Do not register them natively or run
them until the current validation is terminal. Finish003 validation first.

## ISSUE-027: target-dependent blend input range

Current SOFTWARE source still sends unclamped shader output into the blend
equation in both `SoftwareRenderer.cpp` paths:

- `WriteCompiledPixelResult` uses each `pixel.colors[slot]` directly; this is a
  separate compiled-effect/MRT path, not covered by a stock SpriteBatch alone.
- The generalized shaded path constructs `source{r,g,b,a}` and directly calls
  `BlendComponent`. That helper derives InverseSourceAlpha from the raw alpha.
- `SoftwareFramebuffer::WriteColor` / `QuantizeColor` constrain the eventual
  store, after the blend equation. Final-result clamping cannot repair a negative
  destination factor caused by alpha above one.

The [Microsoft output-merger documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-output-merger-stage)
describes pre-blend range handling per render-target format, with floating-point
exceptions. This is Direct3D10/11 documentation, **not a fresh XNA4/D3D9 runtime
measurement**. The historical inventory supplies GLES comparison evidence;
revalidate that on the current renderer before implementing. The Khronos wiki
fetch returned403, so no fetched wiki page is claimed as additional evidence.

The new witnesses isolate four Color-target cases: alpha above one, negative
alpha, RGB subtraction and reverse subtraction. Two HdrBlendable controls must
retain above-one and negative values, including their blend factors. Fixed-point
checks allow one UNORM8 step for rounding exact binary fractions; HDR inputs,
background and expected arithmetic results are binary16-exact and use float
equality. No existing assertion or tolerance is altered.

Do not use `HasWideColor()` to decide whether clamping is needed: it means
`surfaceFormat != Color`, and also includes normalized `Rgba1010102`, `Rg32`
and `Rgba64`. A correct local repair must classify actual target ranges, preserve
floating-point targets, and cover compiled/MRT and fixed shader paths. Further
compiled/MRT witnesses are required before claiming that scope validated.

## ISSUE-066: explicit enable/disable

`SoftwareRenderer2DState.cpp` still contains an empty `SetBlendEnabled(bool)`.
GraphicsDevice and its C adapter forward to that method. Existing
`standalone/c_api.graphics.draw_routes/main.c` checks the accumulated red pixel
from two half-alpha instances and overwrite behavior after disabling. Preserve
those original assertions and reproduce on current CNA after003's ABI consumer
rebuild; the old executable must not run against changed class layouts.

Do not silently assume a uniform factor contract across renderers:

- EasyGL enables blending and installs SourceAlpha / InverseSourceAlpha.
- FNA3D's legacy toggle deliberately installs One / InverseSourceAlpha and Add.
- SDL_GPU and WebGPU only toggle `blendEnabled_`, retaining the blend factors
  captured by their draw commands; ApplyBlendState sets both factors and flag.
- DirectX9 also has an empty toggle.

The public method is CNAEXT, not an XNA4 toggle, and its current comment merely
says it enables/disables blending. The SOFTWARE no-op is source-confirmed, but
factor/reset semantics across families need explicit investigation before
changing a public contract or expanding the repair. A passing SOFTWARE fix alone
would not establish renderer-wide consistency. Both issue IDs remain pending
current runtime verification, with no fix commit and no knownIssue removal.
