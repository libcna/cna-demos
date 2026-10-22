# Frustum-ray repair — 2026-10-22

CNA repair: `9a5e5d896150bf5094de57be22e25ad6f35e2242` on local `next`.
Baseline CNA: `d0cea70363c805cc1e4891ebf7350c441f88a540`; demos: `173bf332469`.
Closes CNA-DEMOS-ISSUE-131 and supplies the museum's matching repair for revalidation.
Nothing was pushed or merged; museum inventory updates remain the museum's responsibility.

The owner explicitly revoked the hands-on implementation reservation. Current HEAD had
already corrected boundary containment, but still rejected every outside-origin ray.
The repair clips the parameter interval against six planes. It preserves XNA's contained
origin result of zero, 1e-5 parallel threshold, exact outside-parallel rejection,
non-normalized directions, empty output on misses, and value/output/delegating routes.

## Evidence

- `FrustumRayOracle.cs`: 24 cases, three original XNA4 routes, 72/72 checks under Wine
  using the original Microsoft assembly. Includes all six entering faces, misses, tangent
  edge/corner contact, stationary rays, threshold boundaries and transformed/camera frusta.
- Two permanent native tests fail before repair; full math baseline is 967 PASS,
  two new FAIL, two existing missing-fixture SKIP. Final math: 969 PASS, the same two SKIP.
- Permanent C GeometrySmoke adds seven cases through both C routes. Strict C17
  `-Wall -Wextra -Wpedantic -Werror` compile and complete smoke pass.
- All 95 math demos on SOFTWARE and OPENGLES3 move from 93 PASS/two expected failures
  to 95 PASS. Before marker removal, the runner correctly exits 1 for unexpected passes.
  After marker removal, both sweeps exit 0. Exact comparison of status, check count,
  knownIssue and message has no unintended deltas. The original two demos still assert
  six and three controls; no assertion or tolerance changed.
- All 19 C math demos pass on each renderer. All 12 selected CLI/catalog/ray/frustum
  CTests pass without skips. Consumer configure and SDL audit pass unchanged.

Raw artifacts are under `build/linux-full/reports/repair-frustum-ray-*`: `reference-v1`,
`red-v1`, `fixed-v1`, `closed-v1`, and `gates-v1`, with logs, JSON outcomes, comparisons
and memory audits. Existing CNA debug and demos linux-full build trees were reused,
with shared ccache and one compiler. Peak consumer rebuild memory was 4,298,473,472
bytes; MemoryHigh throttled, but MemoryMax/OOM/OOM-kill counters stayed zero.
No full-CNA, new visual-reference, sanitizer, or native-Windows validation is claimed
for this pure geometry repair. The authentic XNA reference is Windows code under Wine.

The separate DirectX9 failed-Reset museum fix is CNA `d0cea7036`; it passed five Wine
DirectX9 tests and unchanged 103-test presentation/reset/viewport selections on each
of SOFTWARE, OPENGLES3, OPENGL33, WEBGPU and SDL_GPU.

Next campaign work: reproduce the shared C/C++ runtime duplication in the C API,
then repair linkage with its install/static-archive contracts preserved. The rest
of the campaign is not complete. Preserve inherited ownership/ISSUE113 changes and
the uncommitted storage path tests and input-string reference preparation.
