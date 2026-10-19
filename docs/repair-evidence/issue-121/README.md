# Genuine XNA vector array-range oracle

`RangeOracle.cs` passes 200 cases against Microsoft's actual XNA 4.0 Windows x86
assembly (2026-10-19). Twenty-five windows cover all eight Vector2/3/4 Matrix,
Quaternion and normal range overloads, verifying exact exception types and every
destination element. Logs: `build/linux-full/reports/repair-121-xna*`.
Compile with `mcs -sdk:4 -platform:x86` referencing the installed XNA assembly;
run with CNA's private display wrapper and existing Wine/.NET prefix, with empty
`WAYLAND_DISPLAY`. Microsoft binaries are not redistributed.

The contract is more precise than simply accepting every negative length:

- Source and destination endpoints are evaluated with signed 64-bit addition.
  An endpoint greater than the array length throws `ArgumentException`, even
  when the requested length is zero or negative.
- Otherwise nonpositive lengths are no-ops, including negative indices and
  indices beyond the end whose negative length brings the endpoint back in range.
- For a positive length, a negative source/destination index throws
  `IndexOutOfRangeException` after the endpoint checks. No destination is changed.
- Valid positive windows transform exactly their requested elements; guard values
  outside the window remain untouched.

CNA's former helpers reject negative lengths and indices up front with
`ArgumentOutOfRangeException`; their signed-int endpoint additions also risk
overflow. Native pre-fix regressions report 915 passes/two unchanged fixture skips/
three failures. UBSan on the actual Vector2 production translation unit confirms
signed overflow at `INT_MAX + 1`, retained in `repair-121-ubsan-before.log`.

The guard repair (`9786d4c93`) uses signed-64-bit endpoints and preserves the measured
ordering, empty-range and exact exception contracts. Whole native math now passes
918 tests with the same two fixture skips. Registered vector tests pass 247/247
(63.76s); post-fix UBSan instrumentation of all three production translation units
passes without diagnostics. Logs: `repair-121-native-*`, `repair-121-ubsan-after*`.
All three original external scenarios now pass under both renderer labels, with
16/10/12 checks retained. Markers were removed only after those passes. See the
[complete regression checkpoint](../bounds/REGRESSION_CHECKPOINT.md), including
the explicitly retained and investigated unrelated networking assertion delta.

C ABI array routes use unsigned counts/indices and a separately validated loop
over individual vector operations. They cannot express a negative length and do
not require a corresponding contract change; their existing guard/array smoke
remains part of regression validation.
