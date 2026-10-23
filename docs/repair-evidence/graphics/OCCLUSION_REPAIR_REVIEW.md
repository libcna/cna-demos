# ISSUE-113 inherited repair review — 2026-10-23

The owner requested review and completion of the inherited candidate before the
final handoff. The original refusal expectation is incorrect: genuine XNA permits
distinct overlapping queries and measures each interval independently. Same-query
reentry and reuse without an observation remain refused. The historical finding
and all earlier failed attempts remain in `OCCLUSION_BEGIN_TRIAGE.md`.

The common coordinator partitions draws into native intervals shared by active
logical queries. It never replays draws. Completed native handles are retired
while results remain available to all consumers; one construction probe is shared
by a live group. Disposal runs under the renderer's owning context lease.

## Defects found during this review

The inherited summation saturated at INT_MAX and could overflow signed arithmetic
on native counts with their high bit set. It also lost boolean visibility when a
previous precise sum wrapped to zero. The unchanged 14-case fixture exposed four
failures; UBSan reproduced signed overflow in the actual coordinator translation
unit. These were candidate defects, not accepted expected failures.

The corrected common counter adds modulo 2^32 and interprets the bits as Int32.
Logical queries retain an independent any-sample flag for boolean fallback.
Final source review found the same saturation in the raw SOFTWARE accumulator and
WebGPU's uint64 readback conversion. Both now use the common counter too. Three
additional tests protect genuine XNA's DWORD boundaries, its observed GPU wrap,
and accumulation across both boundaries. The large raw SOFTWARE/WebGPU count
defects were established by source inspection against that reference, not by
rasterizing billions of fragments on each CNA renderer.

## Validation

- Native configured OPENGLES3 graphics: 3,163 cases = 3,048 PASS / 115 unchanged
  SKIP. Exact comparison against the 3,160-case inherited baseline changes only
  the four count failures to passes and adds three counter passes.
- All tests matching `*Occlusion*`: SOFTWARE, OPENGLES3, OPENGL33 and FNA3D OpenGL
  each 35 PASS / 4 WebGPU-only SKIP; WebGPU 39 PASS; SDL_GPU and FNA3D SDL_GPU each
  21 PASS / 18 declared-capability or WebGPU-only SKIP. No skipped query runtime
  is represented as a pass.
- ASan/UBSan/float-cast/leak check: all 17 counter/coordinator tests PASS, with no
  suppression. This latest sanitizer probe instruments the actual coordinator
  and fixture; linked dependency archives are uninstrumented. The earlier full
  lifetime sanitizer checkpoint remains separately documented.
- After the raw-renderer refinement, the final external v3 gate preserved all 132 prior
  outcomes and assertion labels, including eight known ISSUE-082 failing rows.
  C++ query demo: 18 checks; C query demo: 24 checks; both pass on SOFTWARE,
  OPENGLES3 and OPENGL33. SDL_GPU verifies its declared refusal and skips.
  A deliberately defective factory fails the C assertion rather than becoming
  a skip. Six exact visual references pass with zero differences.
- SOFTWARE/OPENGLES3/OPENGL33 screenshots were inspected: wall, moving light and
  matching count/boolean HUD, with no probe geometry left in the visible scene.
  Animation positions differ; no cross-renderer pixel equality is claimed.

Reports: `repair-inherited-before-*`, `repair-inherited-count-{before,after}-*`,
`repair-inherited-count-v2-after-*`, `repair-inherited-counter-v2-*`, and
`repair-inherited-occlusion-external-v3-*` in the ignored report directory.
The first external driver stopped before tests because two historical paths had
been retired by ISSUE-150; the v2 manifest explicitly records those absent paths.
That failed setup log is retained.

The external v3 gate is accepted with no source/artifact drift. CNA repair commit:
`c303d3ad9821ca7ad5cf04490d6f1a0555b2af8c`. The obsolete knownIssue and restriction
are removed; focused unmarked and catalog validation is recorded below.

Linux private-display validation only. No native Windows/D3D9, macOS, browser or
physical-device validation is inferred from these runs.

## Closure gate

The unmarked C++18/C24 controls pass again on SOFTWARE/OPENGLES3/OPENGL33;
SDL_GPU remains a declared skip. The deliberately broken unsupported factory
still fails exactly its assertion. All five selected query/catalog CTests pass.
Report `repair-inherited-113-closure-v2-audit.json`: accepted, no input drift,
210,419,712-byte peak, no memory events. The first closure driver refused an
existing build-log filename before running tests; that setup failure is retained.
