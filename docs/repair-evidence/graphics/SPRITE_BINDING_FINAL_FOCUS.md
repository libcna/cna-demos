# ISSUE-003 final focused validation (2026-10-22)

Product repair is committed as `3b6a2eaf3bc51a7cf44cf645b5cfa66494e0a6bb`. Do not
mistake this checkpoint for completion of the entire repair campaign.

## Restriction-only closure gate

Session92221 completed0 with driver `build-probe/repair-sprite-binding-metadata.mjs`.
The existing consumer target rebuilt in3894ms without diagnostics; registry719
reports0 problems. Original texture demos pass on all four external renderers
(eight runs; unchanged1/9 checks). All five enabled focused/catalog CTests pass,
no skips,17.01s. Exact full catalog comparison allows only removal of the003
restriction; all other fields and5367 frozen inputs remain unchanged except the
expected rebuilt consumer binary and that metadata source. Peak1267585024 bytes,
all memory events zero. Report prefix `repair-sprite-binding-metadata-*`.

The initial preflight stopped before any build because it used the earlier46333
manifest, which predates the separately validated D3D9 Immediate correction and
multi-renderer library rebuild. The successful gate uses the later accepted84921
manifest; no assertion or diagnostic was suppressed. Native Windows/D3D9 remains
explicitly pending;003 is FIXED_PLATFORM_VALIDATION_PENDING, not fully validated
on every platform. The original issue description and regression assertions remain.

## Actual final-focus observations

Driver `build-probe/repair-sprite-binding-final-focus.mjs`, session46333, terminal0:

- Existing cna-demos target build73819ms, no diagnostics; registry70ms,719 demos.
- Strict five-source/22-test fixture build11506ms, warnings as errors.
- SOFTWARE22PASS1741ms; OPENGLES322PASS924ms; OPENGL3322PASS923ms.
- SDL_GPU13PASS/9 compiled-offSKIP3065ms. These nine skips are not runtime coverage.
- Both original texture demos pass on each of the four renderers: eight passes.
  `set_data` retains its one check; `set_data_rectangle` retains all seven original
  checks and adds two direct post-SpriteBatch binding/refusal checks, totaling nine.
- All319 consumer artifacts audited; no frozen source drift. Peak1310486528 bytes,
  all memory events zero. Raw logs/JSON/PNG/XML share the report prefix
  `build/linux-full/reports/repair-sprite-binding-final-focus-*`.

## Screenshot review

All eight screenshots were individually inspected. The gradient/checkerboard
texture is present, correctly placed and unclipped. The rectangle-paint scenes
retain the expected gray background, white canvas and four-color brush strokes.
SOFTWARE contains more strokes, so these snapshots were not called identical.

A read-only pixel analysis reconstructed the original demo's seeded brush sequence:
seed12345, minstd_rand multiplier48271/modulus2147483647, libstdc++ uniform positions
0..15, four original colors (Crimson, DodgerBlue, Gold, SeaGreen), and right-to-left
evaluation of the two Rectangle coordinate arguments in this build. Exact matches:

| Renderer | Compatible Update/stroke counts | Pixel mismatches |
|---|---:|---:|
| SOFTWARE | 685,686,687 (identical final grid) | 0 |
| OPENGLES3 | 89 | 0 |
| OPENGL33 | 88 | 0 |
| SDL_GPU | 86 | 0 |

The analysis checked every pixel: each brush cell is exactly24x24 screen pixels
inside the384x384 canvas at(208,48), with exactly DimGray outside. No tolerance or
image editing was used. These are reconstructed compatible counts, not instrumented
Update counters. They explain the visual difference consistently with Game.cpp's
fixed-step catch-up Update loop and DemoGame.cpp's frame limit counting EndDraw,
not Update. This is diagnostic reconstruction, not a substituted visual oracle.
The earlier six exact-reference comparisons remain preserved in the full audit.

## D3D9 and remaining compiled-on coverage

Final source review found D3D9 also omitted the existing SetImmediateMode hook.
Like WebGPU, its Draw accumulated vertices and deferred custom-effect application
until End. The narrow correction flushes that run before an Immediate Draw returns.
Native fixtures already include DirectX9 and preserve the failure/slot timing tests.

Session17526 compiled all five relevant Windows translation units successfully:
D3D9SpriteBatch7085ms, common SpriteBatch11ms, Texture2D10ms, binding helper8ms,
and allocating DirectX9Renderer5309ms. No diagnostics/drift; peak648994816 bytes,
all memory events zero. This is MinGW compile coverage, not a Windows runtime pass.

The SDL compiled path is available using the already patched MojoShader dependency.
Session84921 completed: configure12627ms0, libraries95586ms0, strict fixture
compile9453ms0. Exactly16 previously reviewed Draco warnings were retained,
with unchanged vendor hashes; no suppression or warning-free build claim.
SDL_GPU22PASS2962ms, FNA3D22PASS3329ms and WebGPU22PASS2186ms; no skips or source
drift, peak2133086208 bytes/all memory events zero. Dependency patch helpers
explicitly skipped already-applied patches; no dependency reset or fetch.
The external tree keeps compiled SDL disabled, preserving its separately
validated C refusal control. All jobs in this evidence chain have terminated.
