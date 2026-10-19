# Genuine XNA matrix residual oracle (139/140)

`MatrixResidualOracle.cs` passes 192 checks against Microsoft's actual XNA 4.0
Windows x86 assembly (2026-10-19). Compile with `mcs -sdk:4 -platform:x86`, reference
the installed XNA assembly, and run with CNA's private display wrapper, the existing
Wine/.NET prefix and empty `WAYLAND_DISPLAY`. Microsoft binaries are not distributed.
Reports: `build/linux-full/reports/repair-matrix-residual-xna-extended*`.

The transpose controls verify all sixteen fields, both zero signs, distinct NaN
payloads and infinities, separate and aliased output, unchanged source and double
transpose. The Plane consumer translates an already offset plane: `(Up,-2)` moved
up five units must become `(Up,-7)` in value/output/aliased-output forms.

## 139: transpose aliases

CNA's output overload wrote into the source before reading all its off-diagonal
fields. Plane.Transform itself uses an in-place transpose of the inverse matrix,
so the defect also corrupts that public consumer. Ordinary zero-offset examples
masked this failure. Three new native tests reproduce two failures before repair;
the separate-output control passes. The original C GeometrySmoke passes, while
its new exact translated nonzero-offset stage fails. All failures are preserved
in `repair-139-*-before*` reports.

Runtime/native commit `0b03b7b21` assigns a completed temporary Matrix before any
output write. It preserves raw bits and repairs both surfaces at the common layer.
Native math now passes 925 tests with two unchanged missing-fixture skips; all
141 registered Matrix/Plane tests pass (10.20s). Strict C17/Werror GeometrySmoke
passes against the rebuilt C API. The original transpose demo retains four checks
and passes both labels; the extended C Plane demo retains nine original checks
plus two exact nonzero-offset/alias controls, all passing both labels.
Two initial multi-ID CLI invocations were rejected before any tests ran; their
usage-error logs remain separate from the correctly invoked focused reports.
The broader Matrix/Plane/glTF/avatar consumer run passes 1,082 tests with the same
four unavailable-fixture/backend skips as the prior consumer checkpoint (1,086
total, 155.12s). The [complete shared external regression checkpoint](../integer-values/REGRESSION_CHECKPOINT.md)
now passes and closes 139. This does not close the separate 140 finding below.

## 140: correct-layer normalization work, validation pending

The same genuine oracle checks scaled-axis shadow planes, source preservation and
near-unit normalization through every Plane form plus Shadow/Reflection consumers.
It demonstrates that blindly copying CNA's current private normalization helper
into CreateShadow would retain a near-unit semantic error. Printed extreme finite
Plane cases also motivate checking intermediate precision at the Plane layer.
`PlaneNormalizeOracle.cs` now passes 3,405 exact field/NaN-classification checks
against genuine XNA, covering all four public forms, source preservation, ordinary
mixed components, near-unit boundaries, signed zeros, subnormals, large and tiny
finite values, infinities and NaN. The measured precision boundary is important:
keep the squared magnitude wide, round Math.Sqrt to Single, then keep its reciprocal
wide until storing the output fields. Preserve the strict near-unit shortcut.
The genuine IL contains the explicit Single conversion after Math.Sqrt.

The initial all-double hypothesis fails 324 checks; its log is retained as
`repair-plane-normalize-xna.log`. Seven candidate rounding models distinguish
the precise behavior: only the square-root-rounded model passes all 3,405 checks.
Rounding the reciprocal too fails 492 cases; prematurely rounding the squared
magnitude also loses the finite range. Diagnostic logs are
`repair-plane-normalize-models*`; final oracle logs are `repair-plane-normalize-final*`.
No tolerance was broadened to accept these differences.

Four new native tests fail before repair (925 passes/two unchanged fixture skips/
four failures). Strict C17 GeometrySmoke independently fails its new normalization
and shadow stages against the frozen pre-140 C API; the prior stages pass.
Local production work makes Plane.Normalize the common correct layer, including
its member form, and routes both Matrix factories through it, removing the private
inconsistent duplicate. Native post-fix validation passes 929 tests with the same
two fixture skips; all 294 registered Matrix/Plane/bounding-volume tests pass
(58.46s). Reports: `repair-140-native-*`. External binaries
remain frozen for the preceding integer/transpose checkpoint: no 140 knownIssue
marker removed or final closure claimed. The broader vector/quaternion range
finding 118 remains independently pending.
