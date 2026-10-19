# Genuine XNA Curve repair oracle

`CurveOracle.cs` preserves the campaign's independent XNA 4.0 checks for historical
ISSUE-002, 036, 097, 137 and 138. Run it against Microsoft's actual framework, not
CNA, MonoGame or FNA. The 2026-10-19 private Wine run passed all 28 checks against
`Microsoft.Xna.Framework, Version=4.0.0.0, Culture=neutral,
PublicKeyToken=842cf8be1de50553`. Microsoft assemblies are external prerequisites
and are not redistributed here.

Compile with `mcs -sdk:4 -platform:x86` and a reference to the installed genuine
`Microsoft.Xna.Framework.dll`, writing the executable into the existing ignored
`build-probe` directory. Use CNA's private display runner with `WAYLAND_DISPLAY`
empty and the existing XNA/.NET Wine prefix. This probe opens no graphics/audio
objects and compares numeric values/classifications directly, avoiding the
console-format ambiguity discovered for ISSUE-116.

Confirmed contracts:

- Step uses normalized segment position, including shifted/negative origins and
  non-unit spans (002).
- Linear post-loop uses the last key's outgoing tangent (036).
- Any changed key position is reinserted; approximate equality must not defeat
  sorting (097). The exact demo witness uses CNA's `MachineEpsilonFloat`, which
  is half the spacing after 1: `5.9604644775390625e-8`, not C++'s epsilon constant.
- CurveKey comparisons involving NaN return positive one in all three operand
  orders, while signed-zero positions compare equal (137).
- Exact duplicate and <=1e-10 spans use interpolation fraction zero. Smooth
  evaluation at NaN propagates NaN; Step at NaN selects the last key's value.
  Empty and one-key curves retain their constant behavior (138).

The reference also verifies value ordering when replacing a key with another
element of the same collection. CNA's value-based C++ collection must copy the
replacement before erasing/shifting its source element; this does not assert
that CNA's documented value ownership is identical to XNA's reference identity.

Raw evidence: `build/linux-full/reports/repair-curve-xna-exact{,-build}.log` and
`repair-curve-xna-exact.stderr.log`. Final output is `checks=28 failures=0`, exit 0.
Stderr contains private display startup/shutdown diagnostics. The earlier run
used the larger C++ epsilon for the nearby-key example and is preserved separately;
the exact run and this source supersede that example without deleting evidence.

Before any Curve production change, the corrected native regressions fail seven
of 73 CTests (`repair-curve-native-before-exact.log`, exit 8). The original native
C API CurveSmoke passes, but its corrected NaN oracle fails stage 2 against the
same old library (`repair-curve-c-api-tests-before.log`, exit 2). These witnesses
must become passes after the production repair; they are not expected failures
to suppress permanently.

A focused sanitizer probe also instruments the actual production
`CurveKeyCollection.cpp` (not only the caller) with ASan/UBSan, while linking the
remaining runtime normally. Replacing index zero from the last collection element
fails before repair with an invalid-vptr member call in `Add`, called by
`setItemProperty`: `erase` destroyed the referenced last object. Evidence:
`repair-curve-sanitizer-before{,-build}.log`, exit 1. This is targeted collection
coverage, not a claim that the whole CNA runtime was sanitizer-instrumented.
The native `ReplacementMayAliasTheLastElement` regression retains this case for
normal and sanitizer configurations alongside the shifting-element value test.

After the local production repair, the unchanged focused sanitizer probe exits
zero with leak detection enabled and no ASan/UBSan diagnostic; both aliased source
positions preserve their values. The native math module passes 886 of 888 tests,
with only the same two missing ShipGame XML fixture skips and no failures. The
aggregate `CnaTests` Curve selection passes all 92 cases (including content-reader
cases). Logs: `repair-curve-sanitizer-after{,-build}.log`,
`repair-curve-all-math-after.log`, `repair-curve-aggregate-after.log`.

Final repair: CNA commit `d753d0997`. All five original C++ reproductions pass
under both renderer labels with all 6/17/20/4/7 controls. The restored collection
exception catches preserve the framework contract and expose all 20 controls,
instead of the old std catches aborting after six. The C API workflow retains
its 15 controls and adds two real Step/post-loop regressions (17 total); the
native C17/Werror CurveSmoke also passes against the rebuilt demo CNA library.
Only the five proven Curve knownIssue markers are removed; assertions remain.

Both complete 95-demo math sweeps report 51 passes and 44 unrelated expected
failures. Comparison with the campaign baseline permits exactly these five
Curve repairs and earlier Matrix oracle correction 116; every other status,
check count and failed-assertion multiset is unchanged. OPENGLES3 ran on CNA's
private display because math.color.named_colors is a Game demo. The preceding
no-display diagnostic is preserved as `repair-curve-math-OPENGLES3-no-display.*`
and is not counted as a regression pass. All 732 enabled demo CTests pass
(412.58 seconds; 48 skipped and three disabled, unchanged from baseline).
Registered native Curve CTests pass 74/74. Evidence is retained under
`build/linux-full/reports/repair-curve-*`, including both comparison JSON files.
