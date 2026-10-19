# Genuine XNA signed-zero hashing oracle

`SignedZeroHashOracle.cs` passes 212 checks against Microsoft's actual XNA 4.0
Windows x86 assembly in the existing private Wine/.NET environment (2026-10-19).
Microsoft binaries are external prerequisites, not redistributed here. Compile with
`mcs -sdk:4 -platform:x86` referencing that assembly, then use CNA's private display
runner with the existing Wine prefix and empty `WAYLAND_DISPLAY`.
Logs: `build/linux-full/reports/repair-120-xna{,-build,.stderr}.log`.
The extended 212-check run is `repair-120-xna-extended*`; the original 206-check
run remains preserved separately.

All sixteen zero-sign masks retain equality and equal hashes for Vector2/3/4,
Quaternion and CurveKey. Every Matrix field is checked individually. Exact unit,
identity and ordinary-key hash controls retain the historical .NET 4 values.
The adjacent nonzero controls demonstrate that .NET 4 retains quiet-NaN payload
and sign bits in hashes. Thus replacing CNA's scalar helper with current Sharp
Runtime `Single::GetHashCode` would change this unrelated contract: modern .NET
canonicalizes NaNs, unlike the tested .NET 4 implementation. Only signed zero needs
canonicalization here; existing nonzero scalar bit hashing must remain unchanged.

The exact ordinary CurveKey control also exposes a second defect: CNA combines
fields with XOR, while genuine XNA and FNA use a wrapped 32-bit sum. With (0,1,2,3),
CNA gives `3FC00000` but XNA gives `BFC00000`. After the initial zero-only repair,
the native suite has 904 passes/two unchanged skips/one failure at this exact
control. This is retained as `repair-120-all-math-after.log`, not treated as a pass.
The implementation must use unsigned 32-bit addition before widening to its public
`size_t` return type. Additional negative-component/continuity sum controls are
included in the oracle and native/C tests. The genuine oracle passes all 212
checks. The complete native math suite now passes 906 tests with the same two
missing-fixture skips (908 total), including all eight hash regressions.
Logs: `repair-120-native-sum-build.log` and `repair-120-all-math-sum.log`.

Eight native tests and a registered C17 smoke protect every zero-sign combination,
each Matrix field, associative-container lookup, adjacent nonzero bits and CurveKey
combination. Pre-fix tests reproduce six failures, and the C smoke fails its new
zero group. Final repair `bf209dcbe` passes the rebuilt C API smoke and all six
original C++ scenarios under both renderer labels. All original assertions remain;
CurveKey additionally checks the exact wrapped sum. See the shared
[complete regression checkpoint](../bounds/REGRESSION_CHECKPOINT.md), including
the explicitly retained and investigated unrelated networking assertion delta.
