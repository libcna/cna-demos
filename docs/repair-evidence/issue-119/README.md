# Genuine XNA Quaternion inverse/division range oracle

`QuaternionInverseOracle.cs` runs against Microsoft's actual XNA 4.0 Windows x86
assembly, not a rewritten implementation. The private Wine run on 2026-10-19
passes 41 checks and reports assembly `Microsoft.Xna.Framework, Version=4.0.0.0,
Culture=neutral, PublicKeyToken=842cf8be1de50553`. Microsoft binaries remain external
prerequisites and are not redistributed.

Compile with `mcs -sdk:4 -platform:x86` referencing the installed XNA assembly and
run through CNA's private display runner with the existing XNA/.NET Wine prefix
and empty `WAYLAND_DISPLAY`. Evidence logs are
`build/linux-full/reports/repair-119-xna{,-build,.stderr}.log`.

Five finite extremes (positive/negative huge/tiny and float maximum) exercise
Inverse and Divide value/out/input-alias forms and self division. The output bits
also preserve signed zero. A mixed numerator/divisor demonstrates why keeping only
the squared sum wide is insufficient: inverse Y near -1e-60 must remain available
until multiplication by numerator Y near 1e20. The final quotient has Z=1 and
W bits `000116C2`, not zero.

The native regressions retain ordinary Hamilton-order and undefined IEEE controls.
This finding is separate from ISSUE-118's caller-dependent genuine-XNA Distance
results; it does not authorize changing Distance or its test expectations.
CNA runtime/native regression repair is committed as `df9234202`. Before repair,
three new range tests fail while the IEEE control passes; the native C17/Werror
smoke fails its new range stage. Afterward, native math has 893 passes and the same
two missing ShipGame XML fixture skips; aggregate and registered Quaternion tests
both pass 109/109. Original demos retain all 11/24 checks and pass under both labels;
native C smoke and the extended 17-check C demo also pass. Both complete math sweeps
have only nine cumulative expected status changes, zero unrelated assertion/check
deltas, and 54 passes/41 remaining expected failures. All six visual references
match exactly. Final full CTest checkpoint passes all 732 enabled tests in 408.88s,
with 48 skips and three disabled unchanged. ISSUE-119 is Resolved; all original
behavior assertions remain. Logs: `build/linux-full/reports/repair-119-*`.
