# Investigated avatar regression after the vector-product repair

The combined 140/123 native consumer run has 1,085 passes, four unchanged skips
and one failure among 1,090 tests. `AvatarCatalogTest.V3DescriptionsAssembleExactlyAsRevised`
changes the catalog-v3/revision-4 digest for the 1950-mm, body-type-1, build-40
description. Original failure: `repair-140-123-native-consumers.log`.

The unchanged test passes when the same test binary uses the frozen pre-product
shared runtime (`0b03b7b21`), proving a numerical dependency rather than an asset
change. This diagnostic works for CnaTests' dynamically imported vector operations;
it does not imply that LD_LIBRARY_PATH can override the separately embedded C API.
Log: `repair-avatar-golden-frozen-runtime.log`.

A field-by-field probe uses the same descriptor and native avatar assembly code
with both shared runtimes. Among 550,524 captured scalar/count records, 952 raw
float fields differ, with maximum absolute difference 2.384185791015625e-7.
Only two cross the existing digest quantization boundary: part6/vertices3357 and
3417/position.X change from -0.2777249813079834 to -0.2777250111103058, causing
quantized integers -27772 to become -27773. The recipe's build projection called
public Vector3.Dot, whose corrected wider accumulation introduced this one-ULP
change. There is no justification for silently regenerating the catalog digest.

CNA `32e470ed4` makes catalog deformation dot products explicitly binary32 at each
product/addition, independently of the public XNA compatibility operations. The
three recipe call sites use that local helper; no public vector fix is reverted,
no descriptor-specific branch is introduced and no golden value is changed.
The existing four-description golden test passes. The same probe now matches all
550,524 baseline records exactly: zero raw or quantized differences.

Evidence: `build/linux-full/reports/repair-avatar-fields-{frozen,current,stable}.log`,
`repair-avatar-fields-comparison.json`, and `repair-avatar-rounding-golden.log`.
The intermediate `repair-140-123-native-consumers-corrected.log` is invalidated:
we relinked its live self-spawning executable while it was running. The avatar
golden test passed, but the child-process installation test failed. Sharp Runtime
uses `/proc/self/exe`, whose replaced executable path gains `(deleted)`; failed
child launch is the supported explanation, not a captured child stderr diagnosis.
This operational failure is retained, not counted as a passing run.

With both executable and runtime frozen, the isolated self-spawning test passes
(4,815 ms), then the complete consumer run passes 1,086 tests with the same four
baseline skips (1,090 total; 182.763 s). Reports:
`repair-avatar-process-frozen-native.log`, `repair-140-123-native-consumers-frozen.log`.
External 123 validation remains pending; the separate integer/transpose checkpoint
has now completed. Never relink a running consumer or its runtime.
