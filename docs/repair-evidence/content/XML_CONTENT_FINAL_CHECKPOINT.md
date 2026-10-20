# Validated XML/content checkpoint — not campaign completion

Completed 2026-10-20. Historical findings **039, 043, 045, 103 and 148** now
have `FIXED_VALIDATED` dispositions. The campaign retains all **155** IDs:
**90 final / 65 pending**. No actionable remaining defect is waived.

## Repairs and focused evidence

| Issue | Repair | Focused evidence |
| --- | --- | --- |
| 039 | CNA `583af2a5e`; owner-approved C contract/docs correction `124281bff`, demos `dab9d4f` | Both setters, same-root refusal and unchanged cache; genuine reference and native checks; C demo67/C++ cache5 on both renderers |
| 043 | CNA `252e9bca1` | Distinct reader/target registration, genuine5/native11/actual-TU sanitizer11; documented fresh C++ ownership preserved |
| 045 | CNA `583af2a5e` | Header/body I/O wrapping and erased LZX prefix bounds; genuine91/native33/actual-TU sanitizer23; caller exceptions unchanged |
| 103 | CNA `68a3c7b1f` | Eight C loader exception identities,44 exact labeled controls on each renderer; canonical C demo67 and existing resource lifetime checks |
| 148 | CNA `e0318ab52`, `de1b6e259`, corpus `dbfe6b4c1`; Sharp Runtime `16993ac6`, `97713eb2` | Self-closing envelope sections, valid PI placement, character BOM refusal; original corpus9 and unchanged demo36 on each renderer |

The owner explicitly selected XNA/C++ RootDirectory behavior for the C API.
The C demo preserves its 54 original controls under two documented semantic
mappings and adds 13 checks. This deliberate contract correction is detailed in
`C_ROOT_CONTRACT.md`; it is not an unexplained relaxed expectation. Marker
follow-ups are demos `aab4bf3` (103) and `eafc39e` (148).

The source/readers history and earlier focused/module/sanitizer evidence remain
in `RUNTIME_OWNERSHIP_TRIAGE.md`. XML dependency details, retained failed runs,
61-case .NET oracle and instrumentation scope are in `XML_SUBSTRATE.md`.
The separate Sharp Process test synchronization commit `d326d37b` changes only
test scheduling, preserving output assertions and strengthening snapshot counts.

## Complete regression gates

- CNA native full suite: **10,690 PASS / 101 unchanged SKIP**, 10,791 cases,
  **1,647.295 seconds**. Every case matches the published frustum baseline;
  three frozen native binary hashes match. Strict original XML corpus **9/9**,
  with PI/BOM exemptions removed and no changed golden/message expectations.
- Sharp Runtime: **18,158 PASS across 41 executables**, zero failures/skips.
  XML574/XML.Linq352 pass; actual parser4 and Reader/Document5 ASan/UBSan/leak
  probes pass. Other linked dependencies were not fully instrumented.
- SOFTWARE: **719 scenarios = 627 PASS / 41 existing FAIL / 48 SKIP / 3 manual**.
- OPENGLES3: **719 = 666 PASS / 43 existing FAIL / 7 SKIP / 3 manual**.
- Both original-baseline audits permit exactly **70 repaired demo outcomes**,
  with zero unexplained differences and no remaining C-root contract residual.
  Local audits permit only XML/C-root FAIL-to-PASS since the ISSUE-016 checkpoint;
  every other status, marker, check count and failing-assertion multiset is exact.
- Relative to **Complete1128**, expected-failure demo counts decrease by **72**
  on each renderer. Existing skips/manual requirements remain unchanged.
- Both95 math sweeps, metadata/catalog719 and all three Python validation suites
  pass. **Six visual references have zero changed pixels, tolerance zero**.
- CTest: **684 PASS / 48 unchanged SKIP / 3 disabled**, 735 registered / 732
  enabled, **887.44 seconds**, zero failures; all **319 frozen artifacts**
  match before and after. Existing expected-failure tests remain visible as such.

Primary machine-readable evidence under `build/linux-full/reports/`:
`repair-xml-substrate-native-gates.json`, `repair-xml-substrate-final-gates-summary.json`,
both `full-*-comparison.json` / `full-*-local-audit.json`,
`repair-xml-substrate-ctest-final-audit.json`, and
`repair-xml-substrate-closure-audit.json`. The closure audit verifies 150 unrelated
historical bodies unchanged and all five original resolved bodies preserved.

Tested runtime sources are represented by CNA `dbfe6b4c1`, Sharp `97713eb2`,
and demos `4160dc3`; subsequent recordkeeping/probe-only commits do not change
those tested binaries. The later uncommitted ISSUE-109 encoder candidate has its
own build and regression and is **not** validated by this checkpoint.

Runtime coverage is Linux/private SDL display with SOFTWARE and OPENGLES3
(Mesa25.0.7); no other renderer, physical input device, platform or online-service
validation is claimed. No additional commit was pushed or merged. The campaign
continues with model-encoder validation and remaining ownership/storage/content
defects; publication authorization remains limited to the earlier frustum push.
