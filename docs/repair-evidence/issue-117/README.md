# Genuine XNA perspective boundary/exception oracle

`PerspectiveOracle.cs` tests Microsoft's actual XNA 4.0 assembly. The 2026-10-19
private Wine run passed 96 checks, reporting Pi bits `40490FDB` and assembly
`Microsoft.Xna.Framework, Version=4.0.0.0, Culture=neutral,
PublicKeyToken=842cf8be1de50553`. Microsoft assemblies are external prerequisites,
not redistributed here.

Compile with `mcs -sdk:4 -platform:x86` referencing the installed genuine Windows
x86 XNA assembly, then run through CNA's private display runner with the existing
XNA/.NET Wine prefix and empty `WAYLAND_DISPLAY`. No graphics/audio object is
created by this oracle. Logs are `build/linux-full/reports/repair-117-xna.log`,
`repair-117-xna-build.log`, and `repair-117-xna.stderr.log`; private-display
startup/shutdown diagnostics in stderr are not a claimed empty stream.

The value and output forms of all three perspective factories use exact
`ArgumentOutOfRangeException` and `ParamName`. Field-of-view validation precedes
depth validation; near <= 0 precedes far <= 0, followed by near >= far. The last
case names `nearPlaneDistance`. Refused output matrices remain unchanged.
Exactly public Pi is refused, its preceding float is accepted. NaN field-of-view
propagates but does not bypass depth checks. Zero/negative aspect and collapsed
width/height/off-centre spans retain their raw IEEE-754 behavior.

Before repair, all three newly added native compatibility tests fail; the whole
math module has 886 passes, two unchanged missing ShipGame XML fixture skips,
and exactly those three failures (891 total). The native C17/Werror MatrixSmoke
also fails its newly added Pi/refusal-output control against the unchanged
demo library. Evidence: `repair-117-native-before.log`,
`repair-117-all-math-before.log`, `repair-117-c-api-before{,-build}.log`.
This confirms both the historical Pi defect and the related wrong exception
type/parameter contract.

CNA repair `2a245b91f` passes 889 native math tests with the same two fixture skips,
all 98 registered Matrix CTests, and native C17/Werror MatrixSmoke. Both renderer
labels pass all 12 original perspective checks; the external C factory demo passes
14 checks including exact Pi refusal, output preservation, and the adjacent float.
Both complete 95-demo math sweeps have only seven cumulative expected changes
(five Curve findings, oracle 116, and repair 117), with no unrelated failed assertion
or check-count changes. All 732 enabled demo CTests pass in 557.06s; 48 skips and
three disabled tests are unchanged. All six visual references match exactly with
zero differing pixels/channels. Evidence: `repair-117-{all-math-after,native-matrix-ctest,
demo-ctest,c-api-after,visual}.log` and `repair-117-math-*-comparison.json` in the
same report directory. No tolerance or behavior assertion was weakened.
