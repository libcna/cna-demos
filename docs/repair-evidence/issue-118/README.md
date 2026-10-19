# Genuine XNA magnitude and distance evidence

The current direct-entry probes invoke Microsoft's actual XNA 4.0 Windows x86
methods through reflection, preventing caller inlining. `DistanceEntryOracle.cs`
passes 1,176 exact result/NaN-classification checks across all six Vector2/3/4 value
and output methods. `MagnitudeEntryOracle.cs` passes 1,184 checks for Vector2/3/4
and Quaternion Length and all normalization forms, including source preservation
and separately returned squared-magnitude controls.

Compile with `mcs -sdk:4 -platform:x86` referencing the installed XNA assembly;
run through CNA's private display wrapper with the existing Wine/.NET prefix and
empty `WAYLAND_DISPLAY`. Microsoft binaries are not redistributed. Logs:
`build/linux-full/reports/repair-distance-entry-xna*` and `repair-magnitude-entry-xna*`.

The measured model retains wide squared sums, and wide coordinate differences
for Distance, until the returned float magnitude. Normalize explicitly rounds
the square root to float, then retains a wide reciprocal until each stored
component, as the genuine IL's conversion and the Plane audit also establish.
Do not broaden normalization tolerances or replace its zero/nonfinite behavior
with a convenient special case. Separately returned LengthSquared/DistanceSquared
still overflow/underflow for the original extreme inputs; their contracts differ
from retaining the intermediate inside a square root.

## Retained caller-dependent evidence

`RangeCallerProbe.cs` is the original exploratory probe, not a passing conformance
oracle. Its two failed universal-distance assumptions are preserved in
`repair-range-xna.log`. A smaller `DistanceCallerProbe.cs`, including NoInlining
wrappers, returns finite values for the same extreme inputs through all six forms
(`repair-distance-xna.log`). Neither result is discarded or described as universal.

The direct-entry reflection probes remove caller inlining as a variable and give
the reproducible finite-range results above. The earlier differences are consistent
with JIT intermediate spills/inlining, but generated JIT assembly was not captured;
that causal detail remains an inference. C# explicitly permits wider intermediate
precision and an explicit cast forces a rounding boundary. See the official
[floating-point specification](https://learn.microsoft.com/en-us/dotnet/csharp/language-reference/language-specification/types#837-floating-point-types).
Matching the measured method-entry arithmetic does not require emulating every
caller-dependent JIT spill. No demo assertion has been weakened or removed.

Eleven new typed native tests fail before repair: 946 tests, 933 passes, two
unchanged missing-fixture skips, eleven failures. The local repair retains wide
intermediates for Length/Distance and the measured normalization rounding, while
leaving the separately returned Squared operations unchanged. Post-fix native math:
944 passes, the same two skips. All 955 selected registered tests are accepted with
zero failures and two existing fixture/backend skips (67.83 s). Strict C17/Werror
VectorSmoke and QuaternionSmoke reproduce the new magnitude stage failure against
the still-frozen pre-fix C API DSO; VectorSmoke also retains its pending 123 failure.
Reports: `repair-118-native-*` and `repair-118-c-before-*`.

The unchanged four-description avatar catalog golden test detects a numerical
consumer regression. The representative field probe finds 32,556 raw differences
among 550,524 records, of which 56 cross the existing quantization thresholds;
positions and imported normals are affected. Original failure and diagnostic:
`repair-118-avatar-golden.log`, `repair-118-avatar-fields-before-comparison.json`.
The catalog's versioned arithmetic must preserve its original binary32 rounding
independently of public XNA operations; no golden digest or tolerance is changed.
CNA repair `bd46b6ffe` contains the public math and native/C regressions. Companion
`785bf2d2a` shares private catalog arithmetic between deformation and imported
normal processing, explicitly retaining binary32 products, sums, distances and
reciprocals. It introduces no descriptor-specific case. All four original golden
descriptions pass again, and all 550,524 captured records match baseline exactly,
with zero raw differences (`repair-118-avatar-fields-stable-comparison.json`).
The broad consumer run passes 1,086 tests with four unchanged fixture/backend skips
(1,090 total, 185.284 s). Rebuilt external focused demos retain all 11/18/14/8/14
original checks under both labels; strict C17 VectorSmoke/QuaternionSmoke pass the
new magnitude stages against the rebuilt C API. The combined 118/123/140 math sweeps
each have 81 passes/14 expected failures, exactly 36 cumulative intended changes
and no unrelated differences. Complete CTest, visual and full renderer regression
now pass; see [the shared closure checkpoint](REGRESSION_CHECKPOINT.md), including
the preserved initial CLI metadata expectation failure and corrected full rerun.
