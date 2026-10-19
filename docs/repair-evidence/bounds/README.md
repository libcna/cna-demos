# Genuine XNA scalar/vector bounds oracle (122 and 136)

`BoundsOracle.cs` passes 6,401 bit-level checks against Microsoft's actual XNA 4.0
Windows x86 assembly in the private Wine/.NET environment (2026-10-19). Microsoft
binaries remain external prerequisites. Compile with `mcs -sdk:4 -platform:x86`
referencing that assembly, then use CNA's private display runner, the existing Wine
prefix, and empty `WAYLAND_DISPLAY`. Logs: `build/linux-full/reports/repair-bounds-xna*`.

The eight-value table contains finite values, both zero signs, infinities, and two
distinct quiet-NaN payloads. Every pair exercises Vector2/3/4 Min/Max value/out/input
alias forms and MathHelper Min/Max. Every triple exercises Clamp value/out/minimum
alias forms plus the scalar counterpart. Ordered, reversed, equal and unordered
bounds retain the observed operand selection and exact zero/NaN bits.

Vector Min/Max select the second operand on ties or unordered comparisons. In
contrast, MathHelper Min/Max propagate a NaN first operand, including its payload.
Clamp caps against maximum first and then floors against minimum; a NaN value
survives, NaN bounds are ignored at their comparison positions, and zero signs
are retained when no comparison selects a bound.

The probe also identifies an incorrect control inside the Vector4 demo: its
purported ordinary case has a reversed Z interval, minimum 8 and maximum 6, for
input 7. Genuine XNA returns `(1,5,8,-6)`, not the demo's `(1,5,6,-6)`. The bounds
repair must correct that control with this evidence, retain its assertion and all
other behavior checks, and describe it as a mixed ordered/reversed case.

CNA runtime/native repair is committed as `eef23e6f8`. Before the repair, the
917-test math suite has 907 passes/two unchanged missing-fixture skips/eight
failures. Strict C17/Werror VectorSmoke and MathValuesSmoke each fail only their
new bounds stage. After the repair, native math passes 915 tests with the same two
skips; all 316 registered vector/scalar/bounds/hash tests pass (64.99s).
Logs: `repair-bounds-native-*`, `repair-bounds-VectorSmoke-before*`, and
`repair-bounds-MathValuesSmoke-before*`.

The Vector4 demo control has been corrected using the actual reference result,
retaining all eight checks. All four original C++ scenarios and the rebuilt C ABI
smokes now pass under both renderer labels. Markers were removed only after those
passes. See the [complete regression checkpoint](REGRESSION_CHECKPOINT.md), including
the explicitly retained and investigated unrelated networking assertion delta.
