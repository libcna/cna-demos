# Geometry values/text checkpoint — 2026-10-20

Closes024/128/130/135 as FIXED_VALIDATED and126 as DUPLICATE_ROOT_CAUSE of024.
All155 historical IDs remain. Campaign totals:54 final/101 pending, including22
runtime findings,one verified duplicate,23 documentation corrections,seven
pre-campaign fixes and one demonstrated demo-expectation correction.

Runtime: `e1377c437` (geometry/math/C marshalling), `a8a60bf03` (four standard
vertex text consumers). Focused demo/evidence commit: `c3f3bbd`. The separate
containment repair `5ab1c7729` is NOT included in this frozen external checkpoint
and its127/129/133 findings remain pending external validation.

## Contract and native validation

- Genuine XNA:1338 value/bit/exception checks plus six exact converter/vertex text
  checks. Earlier wrong positive-zero hypotheses and incomplete vertex-literal
  edit failures remain retained and explained in README.md.
- Native math950 passes/two unchanged missing-fixture skips; registered geometry,
  converter and vertex tests341/341 (69.51s); frozen broad consumers1393 passes/
  four unchanged skips (1397 total,223.580s). All53 broad-build warnings matched
  the original warning baseline; subsequent corrected build warning-free.
- Actual rebuilt C ABI strict C17/Werror GeometrySmoke and VertexValueSmoke pass.
  Mutable negative-radius field marshalling remains distinct from construction.
- 13 focused demos pass under SOFTWARE and OPENGLES3 labels. Original six math
  scenarios retain13/5/12/16/15/10 checks, converter9,stock vertices8 and the C
  vertex program's seven internal checks. Six markers removed only after passes.
  Six adjacent text expectations were corrected to exact genuine output without
  relaxing buffer-size/atomicity/extension controls.

## Complete external checkpoint

Both95-demo math sweeps:87PASS/8expectedFAIL; exactly42 cumulative intended changes
against campaign baseline, no unrelated status/check-count/failed-assertion delta.

| Run | PASS | Expected FAIL | SKIP | Manual/not run |
|---|---:|---:|---:|---:|
| SOFTWARE,719 IDs |599|69|48|3|
| OPENGLES3,719 IDs |638|71|7|3|

Both exact full comparison reports have `differences: []`. Complete1128 was
SOFTWARE555/113/48/3 and OPENGLES3594/115/7/3:44 expected failures become passes
cumulatively. The starting current-HEAD baseline already improved two of these;
the repair-campaign delta is42. This is a demo-status delta, not an issue count.

Metadata719/0, tool suites7+4+5 pass. All six SOFTWARE/OPENGLES3 visual references
are pixel-exact (zero differing pixels, maximum channel difference0); representative
OPENGLES3 texture capture was inspected. No other renderer/platform runtime claim.

First full CTest retained one failure (731accepted/one failed,1200.53s): the
find_and_join client selected a same-gamertag listing with two gamers/two public
slots instead of its own one-gamer/one-public-slot host. Source selects by shared
gamertag alone; this run overlapped another smoke suite on the same discovery
network. Cross-suite discovery is the evidence-backed explanation, not a CNA
geometry regression. No code, assertion, timeout or retry policy was changed.

The entire unchanged suite was then run with no simultaneous SystemLink/discovery
suite: **732 enabled tests accepted,zero failures,48 unchanged skips/three disabled,
1248.07s**. The original failed test passes. Do not overlap full smoke and CTest
network sections in subsequent checkpoints; separate XDG profiles do not isolate UDP.

Executable, libcna.so and actual libcna_c_api.so SHA256 values stayed unchanged
through the checkpoint and were verified again at completion. Raw reports remain
under `build/linux-full/reports/repair-geometry-*`, including the failed first
CTest and the accepted `repair-geometry-demo-ctest-isolated.log`.

The13-test four-TU sanitizer results recorded later in README.md belong to the
next containment batch, not this value/text closure. No evidence is borrowed
across frozen runtime revisions. No push or merge.
