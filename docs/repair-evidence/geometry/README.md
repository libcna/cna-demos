# Geometry value-contract audit (024/126, 128, 130, 135)

This batch is being prepared while the preceding 118/123/140 full regression
checkpoint runs against frozen runtime `785bf2d2a`. Local geometry production
repairs and native regression tests are now being built; no external geometry
marker removal or closure has been performed.

`GeometryValuesOracle.cs` targets the installed genuine XNA 4.0 Windows x86
assembly. It checks negative-radius exception identity, accepted radius bits,
empty/singleton points, fieldwise-mutated negative radii, four ray/box call forms
over stationary interior/boundary/exterior samples, directed and near-parallel
controls, exact box/ray text and all 24 identity-frustum plane bits plus text.
Compile with `mcs -sdk:4 -platform:x86` referencing the external XNA assembly;
run with CNA's private display wrapper and the existing Wine/.NET prefix.
The corrected bit-constructed-zero oracle passes all 1,338 checks against genuine
XNA: `repair-geometry-values-xna-exact{,-build}.log`. Initial positive-zero plane
assumptions fail seven checks; the diagnostic proves seven negative-zero fields.
A subsequent attempt with C# negative-zero literals still compiled to positive
zeros in the expected array. The final probe constructs zeros from their exact
bits instead. All three earlier failed logs remain preserved; no production fix
was made on the basis of the disproved bit assumptions.

Source audit findings:

- 024 and 126 describe the same doubled-literal-brace root cause. Existing native
  and C text oracles also pin the wrong doubled form; correcting them must retain
  exact string and buffer-size assertions with genuine evidence.
- 128 is partially already repaired: `cd211420e3` changed the empty point-set
  exception to `System::ArgumentException` before this campaign. Negative radius
  remains accepted. XNA's public Radius field remains writable after construction.
  The C marshaller currently invokes the validating constructor from a `noexcept`
  conversion; that must become a fieldwise copy when the constructor is repaired,
  or valid fieldwise C representations could terminate the process.
- 130 is an interval initialization/boundary issue, not just a missing zero-vector
  special case. Genuine XNA clips an initially nonnegative interval and uses a
  strict `abs(direction) < 1e-6` parallel threshold. CNA's optional interval and
  machine-epsilon comparison differ. Boundary-start and threshold controls guard
  the local repair rather than introducing a demo-specific special case.
- 135 is a formatting defect: genuine XNA retains the same seven negative-zero
  plane fields as CNA, while .NET Framework displays them as `0`. The decompiled
  `0 - component` expression must not be mistaken for proof of positive-zero data.
  The shared display-operand helper now hides negative zero only for public math
  text, including Vector2/3/4, Quaternion, Matrix, Plane and sphere radius. Stored
  values and derived frustum planes are untouched. Genuine controls cover every
  affected type; ordinary formatting and brace/buffer-size assertions are retained.

Exact pre-fix native regressions: 952 tests, 944 passes, two unchanged missing-fixture
skips, six failures. The initial test build's nodiscard warning was corrected with
an explicit discard; the exact pre-fix build is warning-free. Strict C17/Werror
GeometrySmoke fails the three corrected text stages, negative-radius construction
and stationary-ray stage; the independent fieldwise negative-radius marshal control
and prior plane normalization/shadow stages pass. Reports: `repair-geometry-values-native-before-exact*`
and `repair-geometry-values-c-before.log`. Post-fix native math passes 950 tests
with the same two fixture skips; no plane-data changes were needed.

## Related text consumers (024)

`GeometryTextConsumersOracle.cs` passes six genuine checks: BoundingBox/Ray design
converters and the four standard XNA vertex types all emit one outer brace pair.
The existing design demo and stock vertex native/C/demo tests had copied CNA's
incorrect doubled literals. The four stock types are additional reproductions of
the same 024 porting defect; do not close 024 while they remain uncorrected.
`VertexTextProbe.cpp` minimizes those four outputs without a graphics device.
Reports: `repair-geometry-text-consumers-xna*`, `repair-vertex-text-before*`.
The three CNA-only extended vertex record formats have no genuine XNA counterpart;
their existing text contract is not changed by this finding.

The first local stock-vertex edit changed only the opening literal because the
closing literal occupies a separate source line. The exact native probe correctly
failed all four outputs (`repair-vertex-text-after.log`). Both literals are now
corrected and the exact probe passes four/four (`repair-vertex-text-corrected-after.log`).
The broad native build recompiled untouched tests with 53 existing nodiscard
warnings; the subsequent four-source corrected build has no warnings. Every one
of the 53 warnings matches the original 152-warning baseline. Native consumers
pass 1,393 tests/four unchanged skips (1,397 total,223.580s); registered geometry,
converter and vertex tests pass 341/341 (69.51s). Runtime commits are `e1377c437`
(math/geometry/C marshal) and `a8a60bf03` (stock vertex text companion). The actual
external rebuild is in progress. Its initial one-job invocation was deliberately
interrupted after native suites completed, then resumed at three jobs in the
same tree; the original interruption log is retained, not a compiler failure.
Six demo/C exact text expectations are corrected from genuine evidence; assertion
counts, buffer-size/atomicity controls and CNA-only extension strings are retained.
Actual rebuilt C API strict C17/Werror GeometrySmoke and VertexValueSmoke both
pass. All13 focused scenarios pass both SOFTWARE/OPENGLES3 labels: original six
math scenarios retain13/5/12/16/15/10 checks, design converter9 and stock vertices8;
five adjacent C demos pass. Six geometry knownIssue markers were removed only
after these passes. Marker rebuild and complete math comparison are running;
full external closure remains pending.

Current external checkpoint: both95 math sweeps87PASS/8expectedFAIL, exactly42
cumulative intended changes and no other differences. Full SOFTWARE719:
599PASS/69expectedFAIL/48SKIP/3manual, exact comparison differences[]. Six exact
visuals pass. OPENGLES3 full smoke is still running against frozen `a8a60bf03`.
First full CTest:731accepted/one failure/48skips/three disabled,1200.53s.
The failed find_and_join client selected a different Demo Player session with
two gamers/two public slots rather than this host's one gamer/one public slot.
It selects by gamertag alone and ran concurrently with the full smoke's network
scenarios. Keep this original failure; repeat the full CTest with no concurrent
network suite before accepting closure. Separate XDG profiles do not isolate UDP
discovery. No assertion/timeout/retry change is authorized by this observation.

## Next containment batch: measured distinctions, not current closure

`ContainmentOracle.cs` passes 509/509 genuine XNA checks. It covers sphere point
surface/NaN behavior, sphere containment/intersection boundaries, exact box/frustum
corners, frustum point tolerance and all planes, equal frusta and true separation.
An identity frustum reports Contains(sphere)=Intersects for center(1.8,1.8,0.5),
radius1, but Intersects(sphere)=false: the containment plane classifier is
conservative, while the intersection API performs actual convex separation.
Do not implement the latter by forwarding the former when fixing 127/129/133.

The sphere created from demo points(-2,0,0),(2,0,0),(0,1,0) geometrically encloses
the first point, but genuine Contains(point) calls that exact surface Disjoint.
The creation demo's current use of Contains to assert geometric enclosure must
be corrected with this evidence when the point-containment repair lands; that
must not be misreported as a production regression or hidden by relaxed bounds.
Reports: `repair-containment-xna{,-build}.log`. No containment production code
has changed in this value/text batch.

Seven new native containment tests reproduce six failures against the unchanged
geometry runtime:959 total,951 passes,two unchanged skips,six failures. The
inclusive volume-corner control already passes. No containment production repair
has been applied. `FrustumSeparationOracle.cs` additionally captures12 compact
result rows for four frusta,7,776 sphere cases,5,832 boxes and720 frustum pairs;
all27,216 genuine value/output parity checks pass. This is next-batch evidence,
not a closure claim; report `repair-frustum-separation-xna.log`.

The full native separation fixture adds three failing tests: pre-fix962 tests,
951 passes,two unchanged skips,nine failures. A local stack-only GJK simplex
helper repairs true convex separation without a public API/cache or changes to
the owner-reserved ray/frustum method. First post-fix run:957 passes,two skips,
three failures. All six minimized regressions and complete box/frustum-pair grids
pass; two old native oracles still expected FNA surface/equal-frustum behavior.
The sphere grid's only36 assertion differences are18 Contains(sphere) cases tested
through both forms; all GJK hit and reverse-containment fields already agree.

`FrustumSpherePlaneOracle.cs` minimizes that residual precision issue:1,944 genuine
cases,zero mismatches with a wide inline plane expression,18 with a rounded public
Dot intermediary. XNA Frustum.Contains(sphere) inlines its expression, whereas
calling Sphere.Intersects(plane) inserts a narrowing point. The local fix now
preserves the inline expression without altering the separate public plane API.
Two exact native expectations are corrected from genuine evidence, not relaxed.
Corrected full native math passes960 tests with two unchanged missing-fixture
skips (962 total). Actual modified BoundingBox/BoundingSphere/BoundingFrustum
translation units plus both new test files pass all10 regression tests under
ASan/UBSan/float-cast-overflow with leak detection, including every genuine grid
case; sanitizer build is warning-free. Reports:
`repair-containment-native-after*`, `repair-frustum-sphere-plane-xna*`,
`repair-containment-native-corrected*`. No external containment edits during the
preceding frozen geometry checkpoint.

Additional radius arithmetic audit: the first exploratory caller model repeated
the documented118 Distance spill-dependent range discrepancy (492 mismatched
pairs). It is retained as `SphereBoundaryCallerProbe.cs` and
`repair-sphere-boundary-arithmetic-xna.log`, not treated as a product failure.
`SphereBoundaryArithmeticOracle.cs` now consistently invokes genuine method-entry
Contains/Intersects and Distance/DistanceSquared. All12,800 checks pass its wide
radius-comparison model; narrowing radius sums/differences first differs in18
cases. A new native regression first failed (963tests,960passes/two skips/one
failure). Keeping the radius comparisons wide fixes it:961passes/two unchanged
skips. Final registered160/160pass31.08s; actual production-TU sanitizer11/11pass
with leak detection and warning-free builds. This includes nonfinite centers and public-field negative/nonfinite radii;
constructor validation is not bypassed or changed.

`FrustumBoundaryRayOracle.cs` passes96/96 genuine checks over boundary/interior
origins and zero/forward/backward/diagonal directions. Correct point containment
changes the existing CNA ray consumer branch to return zero on the boundary,
without any edit to the owner-reserved method. The old C oracle expecting
NOT_SUPPORTED there is disproved, as is its public header comment. The next C
smoke preserves the boundary assertion with SUCCESS/hit/zero and adds independent
sphere/convex separation stages. Against frozen pre-containment C API it fails
exactly these three stages (`repair-containment-c-before.log`). Post-fix actual
C ABI validation is still pending; do not infer it from the native C++ build.

The separate containment audit (127/129/133) additionally found that genuine
frustum point containment uses a `1e-5` plane threshold and that several local FNA
implementations retain the historical defects. Genuine XNA takes precedence where
they differ. That work is not folded silently into this value batch.
131 remains explicitly reserved for the human author in CNA `handoff.md` section6;
no ray/frustum implementation or removal of its exception is authorized here.
