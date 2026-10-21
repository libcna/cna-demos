# Blend state and source ranges (ISSUE-027 / ISSUE-066)

## Evidence boundary (2026-10-21)

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
