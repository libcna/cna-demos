# ISSUE-132 native repair; full external regression pending

Source audit of genuine XNA Color.Lerp/Multiply and PackUtils finds that the
nonfinite differences are part of a broader fixed-point factor contract.
Lerp uses PackUNorm(65536, amount), including ties-to-even factor rounding and
NaN-to-zero. Multiply first scales by65536, clamps the factor to[0,16777215],
then multiplies unsigned channel values and shifts16 bits before saturation.
Merely special-casing NaN Lerp and positive-infinite Multiply would miss finite
factor quantization. Zero channels remain zero even for infinite scale.

`ColorFixedPointOracle.cs` passes all1,782 checks against genuine XNA, comparing
the fixed-point model with genuine methods/operators across nine colors and
eighteen finite/nonfinite factors. Logs: `repair-color-fixed-point-xna{,-build}.log`.
`ColorFixedPointProbe.cpp` reproduces162 exact native failures among the same1782
checks against the unchanged Color implementation; strict warning-as-error build
passes. Reports: `repair-color-fixed-point-native-before{,-build}.log`.
Three new CNA-native tests fail before the repair (968total,963PASS,two unchanged
skips,three failures; `repair-color-native-before*`). Two new strict C17/Werror
stages fail against the actual pre-Color external C ABI, while all four original
stages pass (`repair-color-c-before*`). These failures are retained.

The local repair uses the existing packed rounding helper for Lerp's factor and
bounded unsigned products for Multiply. It does not change constructors, packing,
API surface or public layout. The old native NaN Lerp alpha expectation is exactly
corrected from0 to255 based on genuine XNA; no assertion is removed. The matching
native1782-check probe now passes every check. Full native math passes966 tests
with two unchanged missing-fixture skips (`repair-color-native-corrected.log`).
An initial post-fix invocation from the demos cwd could not load eight native
fixtures; `repair-color-native-after.log` preserves that harness mistake. The
unchanged suite passes from the required CNA root.

The actual Color.cpp translation unit plus both Color test files pass64/64 under
ASan/UBSan/float-cast-overflow with leak detection; warning-as-error instrumented
build passes (`repair-color-sanitizer*`). External demos remain frozen on the
preceding containment checkpoint, so no external Color pass or closure is claimed.

Registered Color/original XNA packing tests pass70/70 (11.07s). An additional
non-desktop consumer selection covering packed values, converters, vertex colors
and avatar catalogs/descriptions passes156 tests with one unavailable compiled
effect fixture skipped (157total,7.739s; `repair-color-native-consumers-cpu.log`).
This selection also matches a few named packed-value graphics controls; it is
not evidence of a full graphics renderer regression run. Native build is warning-free.

The subsequent exact1,412-test native consumer inventory also passes:1,408PASS,
the same four fixture/capability skips as containment,282.277s
(`repair-color-native-consumers.log`). It includes content/model/glTF, converters,
avatar catalog/renderer/service controls, vertex/effect consumers, existing math
and audio consumers. It ran on a private display with a separate XDG profile;
the executable/DSO stayed frozen through self-spawning catalog tests. No live
desktop, external online-service or unavailable hardware validation is claimed.

After the preceding containment checkpoint closed, the external demos and actual
C ABI were rebuilt against Color repair `1ad0a3916`, without lifecycle production
changes. Build warning-free; strict C17/Werror ColorSmoke passes all six stages.
All six focused color scenarios pass under both renderer labels, preserving
7/9 checks in the two repaired math demos and all construction/packing/named-color
and actual C controls. Reports `repair-color-c-after*`, `repair-color-focused*`.
Only then were the two132 knownIssue/restriction markers removed; assertions stay
unchanged. Marker rebuild and complete frozen external regression follow. No final
Color closure yet.
