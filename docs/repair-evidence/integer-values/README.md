# Genuine XNA Point/Rectangle oracle (124 and 125)

`IntegerValuesOracle.cs` passes 4,510 checks against Microsoft's actual XNA 4.0
Windows x86 assembly (2026-10-19). Logs: `build/linux-full/reports/repair-integer-values-xna*`.
Compile using `mcs -sdk:4 -platform:x86` referencing the installed XNA assembly;
run under CNA's private display wrapper with the existing Wine/.NET prefix and
empty `WAYLAND_DISPLAY`. Microsoft binaries remain external prerequisites.

The eight integer samples cover both signed limits, negative values, zero and
positive values. Every pair/four-tuple verifies XNA's unchecked 32-bit field-hash
sum. Seven rectangles cover ordinary overlap, disjoint, negative coordinates and
sizes, zero sizes and Empty. All pairs verify Union value/output/input aliases,
source preservation and adjacent Intersect aliases; each triple-alias Union is
also checked. Actual XNA hashes differ from the inherited XOR implementation.

Current CNA before repair reproduces the three original demo assertions under
both renderer labels (Point10/Rectangle identity13/Union10 controls). New native
tests report 919 passes/two unchanged fixture skips/three failures (924 total).
The Intersect alias control passes. Strict C17/Werror MathValuesSmoke fails both
corrected ordinary hash stages and the new integer hash stage; all other stages,
including the C Union alias control, pass. Logs: `repair-integer-values-native-before*`
and `repair-integer-values-c-before*`.

The original native C smoke and two C demos pinned XOR values as their oracle.
Genuine XNA proves Point(3,-2) hashes to 1 and Rectangle(1,2,10,8) to 21. Those
controls are corrected, not removed; new wrap and C alias controls strengthen them.
C ABI Union was already safe because it computes from copied inputs with its own
wrapping arithmetic; only the C++ output overload needs the alias repair.

Local production repair uses unsigned 32-bit addition and bit-preserving signed
conversion for hashes; the Union output overload assigns the completed value-form
result after all reads. Native post-fix validation passes 922 math tests with the
same two missing-fixture skips; all 57 registered Point/Rectangle tests pass
(11.44s). Logs: `repair-integer-values-native-after*` and
`repair-integer-values-native-ctest.log`. Runtime commit: `5a44d2c50`.
Rebuilt external scenarios pass both labels: all original Point10/Rectangle13/
Union10 checks remain, and the strengthened C Point/Rectangle demos pass. Strict
C17/Werror MathValuesSmoke passes against the rebuilt C API. Three markers were
removed only after those passes. The [complete shared regression checkpoint](REGRESSION_CHECKPOINT.md)
now passes, closing 124/125 with the separately committed Matrix transpose repair.
