# Compiler, registry and XNB environment checkpoint — 2026-10-20

Status: **strict GLES comparison failed; superseding corrected checkpoint pending**,
not issue closure. Historical totals remain
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
hashes verified after the run and subsequent consumers. Broader1412 cases give
1408PASS/four unchanged SKIP; runtime241 gives238PASS/three unchanged SKIP.
Default XML fixture selection gives fourPASS/two missing-fixture SKIP; with the
original sample fixtures all six pass. All prior consumer outcomes are unchanged.

SOFTWARE719:615PASS/53 expectedFAIL/48SKIP/three manual. Strict comparison has58
demo-status improvements against campaign HEAD (60 against Complete1128), plus
two149 and one088 assertion improvements; `differences: []`. OPENGLES3 has
654PASS/55FAIL/7SKIP/three manual, but the strict comparison exits1: the existing
030 outage child assertion passes instead of failing, while its latency assertion
still fails. No whitelist or retry was applied. Reference-backed investigation
and subsequent corrected focused execution are recorded in
[the network evidence](../NETWORK_SIMULATOR.md); this old sweep stays a failed gate.
Both95-demo math sweeps have the same93PASS/two owner-reserved131FAIL, with exact48
math improvements. Metadata719/zero errors, tool suites7+4+5 and all six pixel-exact
references pass. GLES font capture inspected. Final serial isolated CTest completes
with all732 enabled cases accepted,48 unchanged skips and three disabled,1045.68s.
All319 artifact hashes match both before and after that CTest. A passing expected-
failure CTest does not supersede the failed strict child-assertion comparison.

Registry native build retains two pre-existing material nodiscard warnings; later
native111 and external/marker builds are warning-free. Native and all319 external
binary hashes remained frozen through their respective gates and the separately
staged030 probe. They were then released for the next section/external-identity
checkpoint. Reports:
`build/linux-full/reports/repair-registry-*`. No additional platforms, hardware,
online services, pushes or merges are claimed.
