# Genuine XNA Matrix decomposition oracle

`DecomposeOracle.cs` tests Microsoft's actual XNA 4.0 Windows x86 assembly. The
2026-10-19 private Wine run passes 92 checks; the first exploratory version passed
48 checks and printed the same 28 case classifications. Microsoft assemblies are
external prerequisites, not redistributed here. Compile using `mcs -sdk:4
-platform:x86` and the installed XNA assembly reference, then run with CNA's private
display wrapper, the existing XNA/.NET Wine prefix, and empty `WAYLAND_DISPLAY`.
Evidence: `build/linux-full/reports/repair-115-xna-zero{,-build,.stderr}.log`;
the earlier 90-check version is retained as `repair-115-xna-extended*`;
the first exploratory logs are retained as `repair-115-xna*`.

Observed contracts include:

- Odd reflections put the negative scale on the largest row length, with X/Y/Z
  priority for ties; the returned rotation/scale recompose the original matrix.
- All seven zero-axis combinations succeed with a four-NaN rotation, preserving
  scale and translation. A nonzero 1e-5 axis is refused with Identity rotation;
  exactly 1e-4 and the tested next larger value succeed.
- Shears 0.05/0.1 are accepted; 0.2/0.5/1 are refused with Identity rotation.
- The fourth column is ignored; NaN/infinite basis input succeeds with NaN rotation.
- When scale and translation outputs alias, scale wins (translation is stored first).

CNA's former positive-scale reflection regression documented the FNA row-product
heuristic as though it were XNA behavior. The actual XNA probe disproves that oracle;
the corrected test retains the determinant control and adds recomposition checks.
The pure C zero-matrix expectation similarly changes from false/Identity to true/NaN.
These are evidence-backed native oracle corrections, not weakened tests.

Before production repair, the whole native math suite reports 893 passes, two
unchanged missing ShipGame XML fixture skips, and five decomposition failures
(900 total). Native C17/Werror MatrixSmoke fails the corrected zero-matrix stage.
CNA runtime/native regression repair is committed as `705ea2324`. Afterward the
native math suite passes 898 tests with the same two fixture skips; 106 registered
Matrix CTests pass. The Matrix/glTF/avatar consumer run passes 1,022 tests with four
skips, all matching baseline (three optional glTF fixtures and a failed-upload test
requiring a non-3D renderer). Native C17/Werror MatrixSmoke passes; the original demo
retains all nine checks and passes under both labels. The extended C factory demo
passes 17 checks. Both full math sweeps have only ten cumulative expected changes
and zero unrelated outcome/assertion deltas. Six visual references are exact.
The first full CTest exposed one additional incorrect demo oracle:
`c_api.math.matrix_values` expected false/Identity for a zero matrix. The new explicit
all-zero genuine-XNA case confirms true/NaN, with zero scale and translation.
That assertion is corrected and strengthened to include scale; all 18 checks remain.
The original failed CTest log is retained (731 accepted outcomes/one failed test,
48 skips and three disabled unchanged, 610.08s), not relabeled as successful.
The corrected full checkpoint passes all 732 enabled CTests (1,174.76s), with
48 skips and three disabled unchanged. Both complete 719-demo sweeps are verified:
SOFTWARE 567 passes/101 expected failures/48 skips/three manual; OPENGLES3
606 passes/103 expected failures/seven skips/three manual. Each exact comparison
has ten cumulative expected changes against the campaign baseline and zero
unrelated status/check/failed-assertion deltas. Reports use the distinct
`repair-115-full-corrected-*` prefix; CTest is `repair-115-demo-ctest-corrected.log`.
The original failed evidence remains intact. Issue 115 is now FIXED_VALIDATED;
the original nine-check demo assertion is preserved and its marker removed.
Evidence logs: `repair-115-*`.
