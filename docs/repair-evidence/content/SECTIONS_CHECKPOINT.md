# Section ordering, external identities and corrected network checkpoint

Date:2026-10-20. Status: **COMPLETE validated checkpoint**.
Historical inventory remains155 IDs,73 final/82 pending. This checkpoint closes
088/090/111 and046 and verifies pre-campaign-fixed030;148 receives
only a partial repair and remains open. It supersedes the strict-failed registry
GLES gate without erasing that result; see [registry evidence](REGISTRY_CHECKPOINT.md).

## Source and reference evidence

- CNAf421948fe resolves external XML target identities in namespace/generic scope,
  including declared fixups, without requiring an instantiable target serializer.
  Genuine15/15;17 native cases include two actual writer round trips. Assembly-
  qualified whitespace is accepted, unqualified surrounding whitespace refused.
  Earlier incorrect whitespace expectations and failing candidates are retained.
- CNAe0318ab52/de1b6e259 repairs self-closing sections and per-section fixup order.
  Genuine18/18. An initial whole-corpus sanitizer run exposed changed diagnostics
  and cyclic test-fixture leaks; the follow-up repairs ordering and adds post-
  assertion RAII cleanup. It does not weaken goldens or production ownership.
- Final actual Reader/Serializer translation units, whole intermediate corpus,
  18 section tests and17 external cases pass ASan/UBSan/leak44/44 in565ms.
  Native focused58/58 passes706ms. Full native552/123 suites completes with
  551PASS/one unchanged XACT SKIP,973.298s: all517 previous outcomes unchanged,
  all35 added section/order/external identity cases PASS. Four native hashes match
  afterward. Original-fixture XML consumers6/6PASS4ms. Broader1412 consumers give
  1408PASS/four unchanged SKIP187.433s; runtime241 gives238PASS/three unchanged
  SKIP; default XML gives fourPASS/two unchanged fixture SKIP. Exact comparisons
  preserve every prior outcome. Final four hashes match, and only then is the
  native freeze released for the separate048 repair. The external freeze remains.
- Network030 was fixed in CNA01534c2e7 before the initial campaign revision.
  Demo0b4d2ce corrects sender-side configuration and the mistaken minimum-latency
  expectation using actual XNA4 simulator27/27 evidence. Exact packet counts and
  six checks remain. See [network evidence](../NETWORK_SIMULATOR.md).

## Completed external focused gate

Existing build/linux-full rebuild succeeded without warnings. Strict56 custom-
content executions (28 per renderer) give25PASS/three remaining FAIL each, with
`errors: []` for every row: metadata, parent/child assertion counts and failure
multisets all match the expected changes. ISSUE-046 gives20/20 on each renderer.
ISSUE-148 retains exactly the string-BOM and inter-element processing-instruction
failures. ISSUE-089 and047/048 remain visible and unchanged.

The rebuilt aggregate runner also passes network030 once per renderer with all
six host checks: SOFTWARE13.319s, OPENGLES3 9.033s. Its previous staged build passed
both renderer selections, and the native simulator/ENet integration gate11/11.
These are local SystemLink tests, not online-service or Xbox hardware validation.

Only after these passes are046/030 markers removed (demos3d1f833). The final marker
build is warning-free and319 artifacts frozen. Both95-case math sweeps preserve
the exact48 expected improvements; metadata/tools and all six pixel-exact visual
references pass. The GLES font capture was also inspected. Both complete719-demo
sweeps now both pass strict comparison: SOFTWARE617PASS/51FAIL/48SKIP/three manual;
OPENGLES3 656PASS/53FAIL/7SKIP/three manual. Both have exactly60 demo improvements
versus initial HEAD (62 versus Complete1128), with `differences: []`; unrelated
assertion failure multisets are unchanged. Final isolated serial CTest completes
with732 accepted results, zero unexpected failures,48 unchanged skips and three
disabled tests,980.07s. Its inventory has735 cases;046/030 now have WILL_FAIL false.
All319 hashes match immediately before and after it. The external freeze is now
released for the separate048 repair; it was not rebuilt during these checks.

## Final dispositions

| Issue | Disposition | CNA repair |
|---|---|---|
| 046 | FIXED_VALIDATED | f421948fe |
| 088 | FIXED_VALIDATED | 80b8e45b2 |
| 090 | FIXED_VALIDATED | 7864dd5b1 |
| 111 | FIXED_VALIDATED | 6823af99a |
| 030 | ALREADY_FIXED, verified with corrected demo | 01534c2e7, before campaign |

Runtime snapshotf421948fe and marker snapshot3d1f833 are the tested checkpoint;
later048 work is not included in these results. Marker changesce04195/3d1f833
retain all behavior assertions. Totals now:40 fixed runtime findings,23
documentation corrections,eight already-fixed findings,one duplicate root cause
and one standalone demo-expectation correction. The030 disposition also records
its evidence-backed demo correction; these categories are not counted twice.
Remaining048,089 and partial148 are not closed by this checkpoint.

Reports: `build/linux-full/reports/repair-sections-*`,
`repair-external-alias-final-sanitizer.log`, and `repair-network-*`.
No vendor changes, extra platforms, service passes, pushes or merges are claimed.
