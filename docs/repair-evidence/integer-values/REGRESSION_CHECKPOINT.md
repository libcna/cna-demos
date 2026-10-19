# Integer values and transpose closure: 124, 125, 139

Validated on Linux x86_64, Radeon 780M/Mesa, 2026-10-19. Runtime fixes are
`5a44d2c50` (124/125) and `0b03b7b21` (139). External binaries remained frozen
at the latter revision throughout this checkpoint. No all-platform claim.

- Genuine XNA: integer values 4,510/4,510; combined matrix residual probe 192/192.
- Native math: 925 passed, two unchanged missing-fixture skips. Registered integer
  tests 57/57; Matrix/Plane 141/141. Matrix/Plane/glTF/avatar consumers: 1,082 passed,
  four unchanged fixture/backend skips (1,086 total).
- Original focused C++ scenarios preserve Point 10, Rectangle identity 13,
  Union 10 and transpose 4 checks; all pass under SOFTWARE and OPENGLES3.
  Strengthened C Point/Rectangle/Plane scenarios pass both labels. Strict C17,
  warnings-as-errors MathValuesSmoke and GeometrySmoke pass against the rebuilt
  C API DSO, not a substituted C++ shared runtime.
- Metadata: 719 demos, zero errors; tool self-tests 7+4+5 pass.
- All 732 enabled CTests accepted, zero failures, 48 skips and three disabled
  unchanged (486.96 seconds).
- Both complete 95-demo math sweeps: 72 pass, 23 expected failures. Exact comparison
  with campaign baseline accepts only the 27 cumulative intended changes.
- Full SOFTWARE: 584 pass, 84 expected failures, 48 skips, three manual.
- Full OPENGLES3: 623 pass, 86 expected failures, seven skips, three manual.
  Both full reports contain every one of 719 IDs. Both exact comparison reports
  have `differences: []`, including check counts and failed assertions. The earlier
  unrelated 030 delta in the values checkpoint remains preserved and investigated
  separately; no comparator whitelist was added.
- All six SOFTWARE/OPENGLES3 visual references match with zero differing pixels
  and zero channel difference. Representative GLES texture capture inspected.

Reports are retained under `build/linux-full/reports/repair-integer-transpose-*`,
`repair-integer-values-*` and `repair-139-*`. Pre-fix failures are retained. Four
knownIssue/restriction pairs were removed after focused passes. The 140 normalization
and 123 product repairs are separate, later native commits and are not claimed as
externally validated by this frozen checkpoint.
