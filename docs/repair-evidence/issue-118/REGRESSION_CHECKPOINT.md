# Magnitudes, vector products and planes: 118, 123, 140

Closure on 2026-10-20, Linux x86_64/Radeon 780M/Mesa. The external runtime
remained frozen at `785bf2d2a` throughout the complete checkpoint. Repairs:
`bd46b6ffe` (118), `0e7be08e0` (123), `e4ddea6e0` (140); avatar recipe
compatibility companions `32e470ed4` and `785bf2d2a`. No all-platform claim.

- Genuine XNA: magnitude 1,184/1,184; distance method-entry 1,176/1,176;
  vector products 8,748/8,748; plane normalization 3,405/3,405. Earlier failed
  exploratory models and caller-dependent probes remain documented and retained.
- Native math: 944 passes, two unchanged missing-fixture skips. Registered
  magnitude/math selection: 955 accepted, zero failures, two existing skips.
  Plane selection 294/294; vector-product selection 566/566.
- Frozen Matrix/Plane/glTF/avatar consumer run: 1,086 passes, four unchanged
  fixture/backend skips (1,090 total, 185.284 seconds). All four original avatar
  golden descriptions pass; all 550,524 representative raw fields match baseline.
  Initial avatar regressions and the separate invalidated live-relink run are
  retained in the 123/118 evidence, not suppressed or presented as passes.
- Nine original focused C++ demos pass both renderer labels, retaining exactly
  5/6/8/5/11/18/14/8/14 assertions. Adjacent C demos pass. Strict C17/Werror
  VectorSmoke, QuaternionSmoke and GeometrySmoke pass the actual rebuilt C API DSO.
- Both 95-demo math sweeps: 81 passes, 14 expected failures. Exact comparison
  accepts 36 cumulative intended changes only, with no unrelated differences.
- Metadata: 719 demos, zero errors. Tool self-tests 7+4+5 pass.
- All 732 enabled CTests accepted, zero failures, 48 skips and three disabled
  unchanged (1,114.49 seconds). The first full run failed only CLI list_ids,
  whose exact expected row still pinned the now-removed 118 knownIssue marker.
  Its exact expectation was corrected to the repaired row and the whole suite
  rerun. Original failure report remains; no behavior assertion was weakened.
- Full SOFTWARE: 593 passes, 75 expected failures, 48 skips, three manual.
- Full OPENGLES3: 632 passes, 77 expected failures, seven skips, three manual.
  Both contain all 719 IDs and exactly 36 cumulative intended changes;
  `differences: []` includes counts, markers and failed-assertion multisets.
  No exception was added for the separately investigated 030 network oracle.
- All six visual references match exactly: zero differing pixels/channel error.
  Representative GLES texture capture inspected. No new memory-ownership change
  was made in this numerical batch; no new sanitizer result is claimed here.

Reports: `build/linux-full/reports/repair-magnitude-products-*`, `repair-118-*`,
`repair-140-*`, `repair-123-*`. Nine knownIssue/restriction pairs were removed
only after focused passes. The subsequent geometry batch is not part of this
checkpoint and has no closure claim yet.
