# Containment checkpoint — 2026-10-20

Closes127/129/133 as FIXED_VALIDATED. All155 historical IDs remain:57 final/98
pending (25 runtime findings,one verified duplicate,23 documentation corrections,
seven pre-campaign fixes,one demonstrated demo-expectation correction).

Runtime `5ab1c7729`; genuine-boundary C regression companion `99d790167`.
Demo marker/oracle commit `74622b4`. Color132 (`1ad0a3916`) is a separate native
repair and was NOT included in these frozen external binaries. Owner-reserved131
remains open: its ray method body and outside-entry exception were not changed.

## Contract and native validation

Genuine probes pass509 minimized checks,14328 shape pairs/27216 overload checks,
1944 plane-precision cases,12800 radius cases,96 boundary-ray checks and19200
squared-arithmetic cases. The initial point-overload parity hypothesis failed
1012/2048 cases; that evidence is retained. The repair preserves genuine distinct
value/output storage boundaries instead of forcing false overload parity.

Native math963PASS/two unchanged missing-fixture skips; registered247/247(63.64s);
frozen broad consumers1408PASS/four unchanged skips(1412total,272.475s).
Actual BoundingBox/BoundingSphere/BoundingFrustum/Vector3 production translation
units with13 regression tests pass ASan/UBSan/float-cast-overflow/leak detection
(696ms). Final formatted native build warning-free; math963/two skips again.

Strict C17/Wall/Wextra/Wpedantic/Werror GeometrySmoke and VertexValueSmoke pass
against the actual rebuilt external C ABI. Initial runs retained three old C
expectation failures despite the new stages passing. Genuine evidence requires
surface sphere point Disjoint, externally tangent spheres not intersecting, and
boundary frustum point Contains. Only those exact expected values were corrected;
all old/new check counts and unrelated controls remain.

Focused17 scenarios per renderer produce15PASS and the two unchanged expected131
failures. Four repaired math witnesses retain8/6/10/8 checks. Four knownIssue
markers were removed only after focused success. Boundary ray now naturally
returns zero through corrected point containment, without changing its method.

## Complete external checkpoint

Both95-demo math sweeps:91PASS/four expectedFAIL. Full719-ID sweeps:

| Renderer | PASS | Expected FAIL | SKIP | Manual/not run |
|---|---:|---:|---:|---:|
| SOFTWARE |603|65|48|3|
| OPENGLES3 |642|67|7|3|

Both exact full comparisons accept46 cumulative intended changes relative to the
campaign's current-HEAD baseline, with `differences: []` for status, knownIssue,
check counts and failed-assertion multisets. Relative to Complete1128,48 expected
failures now pass; two improvements already existed before this campaign. These
are demo-status deltas, not issue totals.

Metadata719/0; tool suites7+4+5 pass. All six SOFTWARE/OPENGLES3 visual references
are pixel-exact (zero differing pixels and maximum channel delta0); representative
GLES capture inspected. No other renderer/platform/hardware runtime claim.

After both full smokes finished, isolated full CTest completed with **all732
enabled tests accepted,zero failures,48 unchanged skips/three disabled,1233.07s**.
The previously collision-prone discovery scenario passes. No retries, timeout
changes, failure whitelist or assertion suppression were introduced. Full smoke
and CTest SystemLink sections must continue running sequentially.

Executable/libcna/libcna_c_api SHA256 values were verified unchanged after smoke
and again after CTest. Raw reports: `build/linux-full/reports/repair-containment-*`,
including original failed C expectations, corrected passes, exact comparisons and
`repair-containment-frozen-final.log`. The external freeze is now released for the
separate Color build. No push or merge.
