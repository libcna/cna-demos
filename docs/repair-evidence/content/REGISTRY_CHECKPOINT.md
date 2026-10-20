# Compiler, registry and XNB environment checkpoint — 2026-10-20

Status: **PENDING final regression**, not issue closure. Historical totals remain
68 final/87 pending of155. This checkpoint covers088,090 and111 only. Subsequent
partial148 source edits are not present in its frozen binaries.

## Repairs and focused evidence

- CNA7864dd5b1: invoke compiler Initialize outside the registry mutex, with
  duplicate/frozen validation before the callback and before publication. Genuine
  XNA8/8; bounded native deadlock reproduction fails before and9/9 passes after;
  native focused10/10. Exceptions do not publish incomplete writers.
- CNA80b8e45b2: atomic intermediate alias registration and separate lexical alias
  versus canonical type identities. Genuine collision11/11 and priority12/12;
  native standalone16/16; actual production-TU ASan/UBSan/leak5/5. An overly strict
  all-name-collision candidate was discarded on genuine XNA evidence, not shipped.
- CNA6823af99a: direct XNB output uses the explicit environment platform/profile;
  the no-environment overload preserves registration defaults and compression.
  Component identity version2 invalidates stale output caches. Four new native
  tests go from onePASS/threeFAIL to fourPASS. This is a CNA extension contract,
  not Windows Phone or Xbox runtime validation.

Final native focus52PASS/one pre-existing opt-in large-file SKIP of53. Both final
external focused runs cover28 demos:24PASS/four unchanged FAIL per renderer,
with exact metadata/count/failure comparisons. XML-to-XNB33/33. Democe04195 changes
only validated markers088->089 and removes090/111; the exact cache version assertion
changes from1 to2, without relaxing its strength.

## Completed regression gates

Native517 cases/119 suites:516PASS/one same XACT SKIP,958.663s. All508 prior outcomes
are unchanged; five registry and four environment tests are new PASS. Four binary
hashes verified after the run. Broader consumers and runtime/XML gates are pending.

SOFTWARE719:615PASS/53 expectedFAIL/48SKIP/three manual. Strict comparison has58
demo-status improvements against campaign HEAD (60 against Complete1128), plus
two149 and one088 assertion improvements; `differences: []`. OPENGLES3 is pending.
Both95-demo math sweeps have the same93PASS/two owner-reserved131FAIL, with exact48
math improvements. Metadata719/zero errors, tool suites7+4+5 and all six pixel-exact
references pass. GLES font capture inspected. Final serial isolated CTest is pending.

Registry native build retains two pre-existing material nodiscard warnings; later
native111 and external/marker builds are warning-free. Native and all319 external
binary hashes stay frozen until their complete respective gates finish. Reports:
`build/linux-full/reports/repair-registry-*`. No additional platforms, hardware,
online services, pushes or merges are claimed.
