# Validated content roots and public boundaries

2026-10-20. Local closure for013,014,105,106,108,110. Historical descriptions and
all155 IDs remain. This checkpoint is **not globally accepted**: the original
C API RootDirectory contract conflict remains exactly recorded, not suppressed.

## Repairs and regression sources

| IDs | CNA repair | Scope |
|---|---|---|
|013,014|`bbe2bda7b`|Exact manifest bookkeeping exclusions and cold relative-root stability|
|105|`1721b4182`|Named-only importer/processor candidates excluded from default routing|
|106|`2ce326907`|Pre-built compiler refusal diagnostics name all seven affected options|
|108|`0d63ac853`|Programmatic inverted font regions refused under the documented contract|
|110|`2386155f2`, `c57ecab8c`|Strict C++ stored-chunk bounds plus preservation of the different documented C ABI|

Demo boundary controls/marker updates are `34e530e`; the manifest's four additive
controls are `1f8ae67`. The original13 manifest controls remain byte-for-byte in
source; runtime counts increase to17. Original37 CNB,32 compiler and23 font child
controls remain. The original C foundations25 ordered labels are unchanged.
Every old failure and historical ID remains in permanent inventory history.

## Validation

- Actual affected TUs and focused probes pass normally and with ASan/UBSan/leak
  detection: routing25 public/23 native; options21/3; font12/8; chunk11/15;
  manifest16/8; cold-root12/one fresh-process native case. C adapter/native strict
  C smoke and unchanged C demo25 also pass normally and instrumented. Dependencies
  were not all instrumented; these are focused sanitizer results, not a fully
  sanitized engine claim.
- Combined canonical CNA build:1141 printed tasks,45 exactly matching prior
  compiler warnings and zero new diagnostics. Module1993=1989PASS/four sameSKIP;
  pipeline562=561PASS/one sameSDK-fixtureSKIP (858.214s). Consumers1412=1408/4,
  runtime241=238/3, default XML4/2 and original-fixture XML6/6; all exact audits pass.
  Six link-footprint checks pass. Four native artifacts remain frozen and later
  CMake policy regeneration produces byte-identical artifacts.
- External combined build:1183 printed tasks, zero warnings; all eight core/C API/
  demo source hashes match. Content146 exact focus:68PASS/five unchangedFAIL per
  renderer. C API354 exact focus:SOFTWARE167/10,GLES168/9; all original failure
  multisets and C foundations25 controls preserved. Independent external manifest
 16/16 and cold-root12/12 pass against the actual rebuilt shared library.
- Full719 SOFTWARE:625PASS,43FAIL,48SKIP,three manual; OPENGLES3:664PASS,45FAIL,
  sevenSKIP,three manual. Compared with completed012, only105 improves and the
  manifest adds four controls. No other result, knownIssue, count or failed
  assertion changes. Both95-case math sweeps pass their exact comparison; all
  six visual references have zero changed pixels at tolerance0.
- Final serial CTest:735 inventory,732 enabled;684PASS,48 unchangedSKIP,three
  disabled,599.13s. All735 outcomes exactly match the established CTest baseline;
  all319 binary hashes match before/after full sweeps and CTest.

Relative to initial campaign HEAD,68 demo outcomes improve; versus Complete1128,
70 improve on each renderer. This numerical delta is not a globally green result:
`c_api.content.content_manager` still fails at its documented live-root-change
assertion instead of reaching its original103 exception-identity witness. The
owner contract decision remains pending; its original assertion/marker is retained.
The strict full driver intentionally exits1 for exactly this unchanged residual;
all independent gates exit0. Last globally accepted checkpoint remains048.

## Preserved failures and limits

The first boundary full run exposed a genuine C ABI regression: stored CNB chunks
historically ignore declared sizes. Its failed SOFTWARE run and stopped pipeline
remain. Adapter `c57ecab8c` restores that documented exception through the common
C++ helper, without relaxing the helper or C oracle; combined full sweeps prove
the original C test is restored. See `CNB_C_API_COMPATIBILITY.md`.

Two content-focus audit setup failures also remain: ordinary DemoContext success
labels are not printed, and the first source-preservation correction included one
extra newline. The corrected audit checks exact original source preservation and
all runtime counts/status/failure multisets. No product assertion was waived.
Prior stronger-than-project fixture compilation failures remain documented too.

Runtime scope is Linux/SDL3, SOFTWARE and OPENGLES3 on the available AMD/Mesa GPU.
No Windows/macOS/iOS, physical audio, unavailable renderer or external-service
validation is claimed. The cold-root native test actually executes on this SDL
desktop configuration; it is not counted from a platform skip.

Reports use `repair-content-roots-*`; focused audit is
`repair-content-roots-focused-roots-source-exact-audit.json`. Earlier actual-TU
proof/report prefixes are listed in `ROOT_REPAIRS.md` and `BOUNDARY_REPAIRS.md`.
During this checkpoint, independent build-only ISSUE-001 repair `b35fbc933` was
committed and native-validated, with every runtime binary byte-identical. Its
external reconfiguration/build remains a separate next gate, not part of this
six-ID content closure.
